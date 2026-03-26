#include "parkinson_nn.h"
#include "weights.h"
#include <stdint.h>

void parkinson_predict(
    uint8_t input_img[16384],
    float output_logits[2]
) {
    // AXI Interfaces
    #pragma HLS INTERFACE m_axi port=input_img bundle=gmem0 depth=16384
    #pragma HLS INTERFACE m_axi port=output_logits bundle=gmem0 depth=2
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    // --- RESOURCE CONSTRAINTS: THE "HARD" LIMITS ---
    // This forces HLS to use a very small number of floating-point units.
    #pragma HLS allocation operation instances=fmul limit=15
    #pragma HLS allocation operation instances=fadd limit=15
   
    // Explicitly mapping to BRAM to save LUTs
    static float normalized_img[16384];
    #pragma HLS BIND_STORAGE variable=normalized_img type=RAM_1P impl=BRAM
    static float layer1_out[4][31][31];
    #pragma HLS BIND_STORAGE variable=layer1_out type=RAM_1P impl=BRAM
    static float layer2_out[8][7][7];
    #pragma HLS BIND_STORAGE variable=layer2_out type=RAM_1P impl=BRAM

    // --- STEP 0: NORMALIZATION ---
    for(int i = 0; i < 16384; i++) {
        #pragma HLS PIPELINE II=1
        // Multiply by (1/255) to avoid the heavy floating-point divider
        normalized_img[i] = (float)input_img[i] * 0.0039215686f;
    }

    // --- STEP 1: CONV1 ---
    for (int f = 0; f < 4; f++) {
        for (int r = 0; r < 31; r++) {
            for (int c = 0; c < 31; c++) {
                // We move the PIPELINE here to only optimize the math, not the loops
                float max_val = -1e10f;
                for (int pr = 0; pr < 4; pr++) {
                    for (int pc = 0; pc < 4; pc++) {
                        #pragma HLS PIPELINE II=4
                        // Increasing II (Initiation Interval) allows the tool to reuse 1 DSP
                        // for multiple multiplications instead of creating 9 separate ones.
                        float sum = conv1_b_init[f];
                        for (int wr = 0; wr < 3; wr++) {
                            for (int wc = 0; wc < 3; wc++) {
                                int img_r = (r * 4) + pr + wr;
                                int img_c = (c * 4) + pc + wc;
                                if(img_r < 128 && img_c < 128)
                                    sum += normalized_img[img_r * 128 + img_c] * conv1_w_init[f*9 + wr*3 + wc];
                            }
                        }
                        if (sum > max_val) max_val = sum;
                    }
                }
                layer1_out[f][r][c] = (max_val > 0) ? max_val : 0.0f;
            }
        }
    }

    // --- STEP 2: CONV2 ---
    for (int f = 0; f < 8; f++) {
        for (int r = 0; r < 7; r++) {
            for (int c = 0; c < 7; c++) {
                float max_val = -1e10f;
                for (int pr = 0; pr < 4; pr++) {
                    for (int pc = 0; pc < 4; pc++) {
                        #pragma HLS PIPELINE II=4
                        float sum = conv2_b_init[f];
                        for (int ch = 0; ch < 4; ch++) {
                            for (int wr = 0; wr < 3; wr++) {
                                for (int wc = 0; wc < 3; wc++) {
                                    int in_r = (r * 4) + pr + wr;
                                    int in_c = (c * 4) + pc + wc;
                                    if(in_r < 31 && in_c < 31)
                                        sum += layer1_out[ch][in_r][in_c] * conv2_w_init[f*36 + ch*9 + wr*3 + wc];
                                }
                            }
                        }
                        if (sum > max_val) max_val = sum;
                    }
                }
                layer2_out[f][r][c] = (max_val > 0) ? max_val : 0.0f;
            }
        }
    }

    // --- STEP 3: DENSE ---
    for (int i = 0; i < 2; i++) {
        float sum = dense_b_init[i];
        int flatten_idx = 0;
        for (int r = 0; r < 7; r++) {
            for (int c = 0; c < 7; c++) {
                for (int f = 0; f < 8; f++) {
                    #pragma HLS PIPELINE II=2
                    sum += layer2_out[f][r][c] * dense_w_init[i * 392 + flatten_idx];
                    flatten_idx++;
                }
            }
        }
        output_logits[i] = sum;
    }
}

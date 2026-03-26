#include <stdio.h>
#include <stdlib.h>
#include "parkinson_nn.h"

int main() {
    // Buffers for simulation
    float input_raw_float[IMG_SIZE * IMG_SIZE];
    uint8_t input_camera_ready[IMG_SIZE * IMG_SIZE];
    float output_hls[OUTPUT_SIZE];

    // 1. Load the single test image (assumed to be float 0.0-1.0 from Python)
    FILE *f_in = fopen("input_img.bin", "rb");
    if (!f_in) {
        printf("ERROR: Cannot find input_img.bin\n");
        return 1;
    }
    fread(input_raw_float, sizeof(float), IMG_SIZE * IMG_SIZE, f_in);
    fclose(f_in);

    // 2. Convert to uint8_t (0-255) to simulate real OV7670 camera data
    for(int i = 0; i < (IMG_SIZE * IMG_SIZE); i++) {
        input_camera_ready[i] = (uint8_t)(input_raw_float[i] * 255.0f);
    }

    // 3. Run the Hardware IP Simulation
    printf("--- Running HLS Prediction on Single Camera Frame ---\n");
    parkinson_predict(input_camera_ready, output_hls);

    // 4. Determine Result
    const char* result_label = (output_hls[0] > output_hls[1]) ? "Healthy" : "Parkinson's Detected";

    // 5. Final Output Format
    printf("\n==============================\n");
    printf("       DIAGNOSIS REPORT       \n");
    printf("==============================\n");
    printf("Healthy Score:    %10.4f\n", output_hls[0]);
    printf("Parkinson Score:  %10.4f\n", output_hls[1]);
    printf("------------------------------\n");
    printf("Result:           %s\n", result_label);
    printf("==============================\n");

    return 0;
}

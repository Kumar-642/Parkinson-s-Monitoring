#include <stdio.h>
#include "xil_io.h"
#include "xgpio.h"
#include "sleep.h"
#include "xparameters.h"
#include "xparkinson_predict.h"
#include "xil_cache.h"

#define SNAPSHOT_BRAM_BASE 0x40100000

#define FRAME_W 320
#define FRAME_H 240

#define CROP_SIZE 128
#define START_X 96
#define START_Y 56

/* CNN buffers */
uint8_t input_img[16384] __attribute__((aligned(64)));
float output_logits[2] __attribute__((aligned(64)));

XParkinson_predict model;
XGpio Gpio;

int main()
{
    printf("System Started\n");

    XGpio_Initialize(&Gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    XGpio_SetDataDirection(&Gpio, 1, 0x0);

    XParkinson_predict_Initialize(&model, XPAR_PARKINSON_PREDICT_0_DEVICE_ID);

    while(1)
    {
        printf("\nTaking snapshot...\n");

        /* trigger snapshot */
        XGpio_DiscreteWrite(&Gpio,1,1);
        usleep(40000);
        XGpio_DiscreteWrite(&Gpio,1,0);

        int idx = 0;

        for(int y=0;y<128;y++)
        {
            for(int x=0;x<128;x++)
            {
                int real_x = START_X + x;
                int real_y = START_Y + y;

                int addr = real_y*FRAME_W + real_x;

                u32 pixel32 = Xil_In32(SNAPSHOT_BRAM_BASE + addr*4);
                u16 pixel = pixel32 & 0x0FFF;

                int r = (pixel>>8) & 0xF;
                int g = (pixel>>4) & 0xF;
                int b = pixel & 0xF;

                int gray4 = (r+g+b)/3;   // 0..15
                int gray8 = gray4 * 17;  // 0..255

                input_img[idx++] = (uint8_t)gray8;
            }
        }

        printf("Input sample: %d %d %d\n",
               input_img[0],
               input_img[50],
               input_img[500]);

        printf("Running inference...\n");

        Xil_DCacheFlushRange((UINTPTR)input_img,16384);
        Xil_DCacheFlushRange((UINTPTR)output_logits,8);

        XParkinson_predict_Set_input_img(&model,(u64)input_img);
        XParkinson_predict_Set_output_logits(&model,(u64)output_logits);

        XParkinson_predict_Start(&model);

        while(!XParkinson_predict_IsDone(&model));

        Xil_DCacheInvalidateRange((UINTPTR)output_logits,8);

        float healthy = output_logits[0];
        float parkinson = output_logits[1];

        printf("Healthy score: %f\n",healthy);
        printf("Parkinson score: %f\n",parkinson);

        if(parkinson>healthy)
            printf("Prediction: Parkinson\n");
        else
            printf("Prediction: Healthy\n");

        sleep(2);
    }

    return 0;
}


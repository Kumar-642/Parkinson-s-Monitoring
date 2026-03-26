#ifndef PARKINSON_NN_H
#define PARKINSON_NN_H

#include <hls_math.h>
#include <stdint.h>

#define IMG_SIZE 128
#define OUTPUT_SIZE 2

// Function signature for 8-bit camera data input
void parkinson_predict(
    uint8_t input_img[IMG_SIZE * IMG_SIZE],
    float output_logits[OUTPUT_SIZE]
);

#endif

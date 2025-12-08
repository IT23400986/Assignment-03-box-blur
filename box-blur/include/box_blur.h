#ifndef BOX_BLUR_H
#define BOX_BLUR_H

#include <stdint.h>

#define KERNEL_SIZE 3

// Function prototype for Box blur (RGB output)
// Implementations should blur per channel and produce 3-channel RGB output
void apply_box_blur(const unsigned char *input_image, unsigned char *output_rgb,
                   int width, int height, int channels, int kernel_size);

#endif // BOX_BLUR_H
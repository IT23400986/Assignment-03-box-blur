#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <stdint.h>

// Read BMP grayscale image file and return pixel data
unsigned char* read_image(const char* filename, int* width, int* height);

// Write BMP grayscale image file
int write_image(const char* filename, unsigned char* image, int width, int height);

#endif // IMAGE_IO_H
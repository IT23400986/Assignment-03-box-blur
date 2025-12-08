#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_io.h"

// BMP file header structures
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t important_colors;
} BMPInfoHeader;
#pragma pack(pop)

// Read BMP grayscale image (8-bit)
unsigned char* read_image(const char* filename, int* width, int* height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    BMPHeader header;
    BMPInfoHeader info;

    // Read headers
    if (fread(&header, sizeof(BMPHeader), 1, file) != 1) {
        fprintf(stderr, "Error reading BMP header\n");
        fclose(file);
        return NULL;
    }

    if (header.type != 0x4D42) { // "BM"
        fprintf(stderr, "Not a valid BMP file\n");
        fclose(file);
        return NULL;
    }

    if (fread(&info, sizeof(BMPInfoHeader), 1, file) != 1) {
        fprintf(stderr, "Error reading BMP info header\n");
        fclose(file);
        return NULL;
    }

    *width = info.width;
    *height = info.height;

    if (info.bits_per_pixel != 8) {
        fprintf(stderr, "Only 8-bit grayscale BMP supported (got %d-bit)\n", info.bits_per_pixel);
        fclose(file);
        return NULL;
    }

    // Skip color palette (256 colors * 4 bytes)
    fseek(file, 54 + 256 * 4, SEEK_SET);

    // Calculate row size (must be multiple of 4)
    int row_size = ((*width) + 3) & (~3);
    unsigned char *image = (unsigned char*)malloc((*width) * (*height));
    
    if (!image) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // BMP stores bottom-to-top, read and flip
    for (int i = *height - 1; i >= 0; i--) {
        if (fread(image + i * (*width), 1, *width, file) != *width) {
            fprintf(stderr, "Error reading image data\n");
            free(image);
            fclose(file);
            return NULL;
        }
        // Skip padding
        fseek(file, row_size - *width, SEEK_CUR);
    }

    fclose(file);
    return image;
}

// Write BMP grayscale image (8-bit)
int write_image(const char* filename, unsigned char* image, int width, int height) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return -1;
    }

    int row_size = (width + 3) & (~3);
    int image_size = row_size * height;
    int file_size = 54 + 256 * 4 + image_size;

    // BMP Header
    BMPHeader header = {
        .type = 0x4D42,
        .size = file_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = 54 + 256 * 4
    };

    // BMP Info Header
    BMPInfoHeader info = {
        .size = 40,
        .width = width,
        .height = height,
        .planes = 1,
        .bits_per_pixel = 8,
        .compression = 0,
        .image_size = image_size,
        .x_pixels_per_meter = 2835,
        .y_pixels_per_meter = 2835,
        .colors_used = 256,
        .important_colors = 256
    };

    // Write headers
    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info, sizeof(BMPInfoHeader), 1, file);

    // Write grayscale palette
    for (int i = 0; i < 256; i++) {
        unsigned char color[4] = {i, i, i, 0};
        fwrite(color, 4, 1, file);
    }

    // Write image data (bottom-to-top)
    unsigned char padding[3] = {0, 0, 0};
    int pad_size = row_size - width;
    
    for (int i = height - 1; i >= 0; i--) {
        fwrite(image + i * width, 1, width, file);
        if (pad_size > 0) {
            fwrite(padding, 1, pad_size, file);
        }
    }

    fclose(file);
    return 0;
}

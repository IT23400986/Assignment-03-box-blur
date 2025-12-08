#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Convert any image format to grayscale BMP
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Image to BMP Converter\n");
        printf("=====================\n");
        printf("Usage: %s <input_image> <output.bmp>\n\n", argv[0]);
        printf("Supported input formats: JPG, PNG, TGA, BMP, GIF\n");
        printf("Output: Grayscale BMP file\n\n");
        printf("Example:\n");
        printf("  %s photo.jpg data/sample_images/photo.bmp\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    int width, height, channels;
    unsigned char *img = stbi_load(input_file, &width, &height, &channels, 0);
    
    if (img == NULL) {
        fprintf(stderr, "Error: Could not load image '%s'\n", input_file);
        fprintf(stderr, "Reason: %s\n", stbi_failure_reason());
        return EXIT_FAILURE;
    }

    printf("Loaded image: %s\n", input_file);
    printf("  Dimensions: %dx%d\n", width, height);
    printf("  Channels: %d\n", channels);

    // Convert to grayscale
    unsigned char *grayscale = (unsigned char*)malloc(width * height);
    if (grayscale == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        stbi_image_free(img);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < width * height; i++) {
        if (channels == 1) {
            grayscale[i] = img[i];
        } else if (channels >= 3) {
            // Convert RGB to grayscale using standard formula
            int r = img[i * channels];
            int g = img[i * channels + 1];
            int b = img[i * channels + 2];
            grayscale[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        } else {
            grayscale[i] = img[i * channels];
        }
    }

    // Convert grayscale to RGB for BMP (BMP needs 3 channels)
    unsigned char *rgb = (unsigned char*)malloc(width * height * 3);
    if (rgb == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(grayscale);
        stbi_image_free(img);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < width * height; i++) {
        rgb[i * 3] = grayscale[i];     // R
        rgb[i * 3 + 1] = grayscale[i]; // G
        rgb[i * 3 + 2] = grayscale[i]; // B
    }

    // Write BMP file
    if (!stbi_write_bmp(output_file, width, height, 3, rgb)) {
        fprintf(stderr, "Error: Could not write BMP file '%s'\n", output_file);
        free(rgb);
        free(grayscale);
        stbi_image_free(img);
        return EXIT_FAILURE;
    }

    printf("\nSuccessfully converted to: %s\n", output_file);
    printf("  Format: Grayscale BMP (24-bit)\n");

    free(rgb);
    free(grayscale);
    stbi_image_free(img);

    return EXIT_SUCCESS;
}

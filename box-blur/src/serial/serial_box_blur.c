#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Blur each RGB channel independently; if input is grayscale (1 channel) we reuse that channel for all outputs.
void apply_box_blur_color(const unsigned char *input, unsigned char *output_rgb, int width, int height, int channels, int kernel_size) {
    int k_offset = kernel_size / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                int count = 0;

                for (int m = -k_offset; m <= k_offset; m++) {
                    for (int n = -k_offset; n <= k_offset; n++) {
                        int nx = x + n;
                        int ny = y + m;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            int src_idx = (ny * width + nx) * channels + (channels == 1 ? 0 : c);
                            sum += input[src_idx];
                            count++;
                        }
                    }
                }

                int dst_idx = (y * width + x) * 3 + c;
                output_rgb[dst_idx] = (unsigned char)(sum / count);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Box Blur - Serial Implementation\n");
        printf("=================================\n");
        printf("Usage: %s <input_image> <output_image>\n", argv[0]);
        printf("Supported formats: JPG, PNG, BMP, TGA, GIF\n");
        return EXIT_FAILURE;
    }

    printf("=== Serial Box Blur ===\n");
    printf("Input: %s\n", argv[1]);
    printf("Output: %s\n", argv[2]);

    int width, height, channels;
    unsigned char *input_rgb = stbi_load(argv[1], &width, &height, &channels, 0);
    
    if (input_rgb == NULL) {
        fprintf(stderr, "Error: Could not read image '%s'\n", argv[1]);
        fprintf(stderr, "Reason: %s\n", stbi_failure_reason());
        return EXIT_FAILURE;
    }

    printf("Image loaded: %dx%d, %d channel(s)\n", width, height, channels);

    int kernel_size = 5;  // 5x5 box blur kernel

    printf("Kernel size: %dx%d (box blur - uniform averaging)\n", kernel_size, kernel_size);
    printf("\nApplying box blur...\n");

    unsigned char *output_rgb = (unsigned char*)malloc(width * height * 3);
    if (output_rgb == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        stbi_image_free(input_rgb);
        return EXIT_FAILURE;
    }

    // Start timing
    clock_t start_time = clock();

    apply_box_blur_color(input_rgb, output_rgb, width, height, channels, kernel_size);
    
    // End timing
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Auto-detect output format
    int success = 0;
    if (strstr(argv[2], ".png") || strstr(argv[2], ".PNG")) {
        success = stbi_write_png(argv[2], width, height, 3, output_rgb, width * 3);
    } else if (strstr(argv[2], ".jpg") || strstr(argv[2], ".JPG") || 
               strstr(argv[2], ".jpeg") || strstr(argv[2], ".JPEG")) {
        success = stbi_write_jpg(argv[2], width, height, 3, output_rgb, 90);
    } else {
        success = stbi_write_bmp(argv[2], width, height, 3, output_rgb);
    }

    if (!success) {
        fprintf(stderr, "Error writing output image.\n");
        free(output_rgb);
        stbi_image_free(input_rgb);
        return EXIT_FAILURE;
    }

    printf("\n=== Results ===\n");
    printf("Execution time: %.6f seconds\n", elapsed_time);
    printf("Pixels processed: %d\n", width * height);
    printf("Throughput: %.2f Mpixels/sec\n\n", (width * height) / (elapsed_time * 1000000));

    free(output_rgb);
    stbi_image_free(input_rgb);
    return EXIT_SUCCESS;
}
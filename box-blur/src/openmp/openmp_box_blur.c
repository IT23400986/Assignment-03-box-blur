#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Blur three channels independently; if input is grayscale reuse the single channel for all outputs.
void apply_box_blur_openmp(const unsigned char *input, unsigned char *output_rgb, int width, int height, int channels, int kernel_size) {
    int k_offset = kernel_size / 2;

    #pragma omp parallel for collapse(2)
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
        printf("Box Blur - OpenMP Multi-threaded\n");
        printf("Usage: %s photo.jpg output.jpg\n", argv[0]);
        printf("Supports: JPG, PNG, BMP\n");
        return EXIT_FAILURE;
    }

    // Get number of threads
    int num_threads = omp_get_max_threads();
    printf("=== OpenMP Box Blur ===\n");
    printf("Input: %s\n", argv[1]);
    printf("Output: %s\n", argv[2]);

    int width, height, channels;
    unsigned char *input_rgb = stbi_load(argv[1], &width, &height, &channels, 0);
    
    if (input_rgb == NULL) {
        fprintf(stderr, "Error: Cannot read %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Loaded: %dx%d, %d channel(s)\n", width, height, channels);

    int kernel_size = 5;

    printf("Kernel: %dx%d box blur\n", kernel_size, kernel_size);
    printf("Threads: %d\n", num_threads);
    printf("\nProcessing...\n");

    unsigned char *output_rgb = (unsigned char*)malloc(width * height * 3);
    if (!output_rgb) {
        fprintf(stderr, "Memory allocation failed.\n");
        stbi_image_free(input_rgb);
        return EXIT_FAILURE;
    }

    double start_time = omp_get_wtime();
    apply_box_blur_openmp(input_rgb, output_rgb, width, height, channels, kernel_size);
    double time_sec = omp_get_wtime() - start_time;

    // Auto-detect output format
    int ok = 0;
    if (strstr(argv[2], ".png")) ok = stbi_write_png(argv[2], width, height, 3, output_rgb, width*3);
    else if (strstr(argv[2], ".jpg")) ok = stbi_write_jpg(argv[2], width, height, 3, output_rgb, 90);
    else ok = stbi_write_bmp(argv[2], width, height, 3, output_rgb);

    if (!ok) {
        fprintf(stderr, "Error writing output\n");
        return EXIT_FAILURE;
    }

    printf("\n=== Results ===\n");
    printf("Time: %.6f seconds\n", time_sec);
    printf("Pixels: %d\n", width * height);
    printf("Speed: %.2f Mpixels/sec\n\n", (width * height) / (time_sec * 1000000));

    free(output_rgb);
    stbi_image_free(input_rgb);
    return EXIT_SUCCESS;
}
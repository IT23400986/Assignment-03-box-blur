#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda_runtime.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// CUDA kernel for Box blur
__global__ void box_blur_kernel(const unsigned char *input, unsigned char *output, int width, int height, int channels, int kernelSize) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int halfKernelSize = kernelSize / 2;

        for (int c = 0; c < 3; c++) {
            int sum = 0;
            int count = 0;
            for (int ky = -halfKernelSize; ky <= halfKernelSize; ky++) {
                for (int kx = -halfKernelSize; kx <= halfKernelSize; kx++) {
                    int ix = x + kx;
                    int iy = y + ky;
                    
                    if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
                        int src_idx = (iy * width + ix) * channels + (channels == 1 ? 0 : c);
                        sum += input[src_idx];
                        count++;
                    }
                }
            }
            int dst_idx = (y * width + x) * 3 + c;
            output[dst_idx] = (unsigned char)(sum / count);
        }
    }
}

// Function to allocate memory and launch the kernel
void cuda_box_blur(const unsigned char *h_input, unsigned char *h_output, int width, int height, int channels, int kernelSize, int blockDim) {
    unsigned char *d_input, *d_output;

    size_t inputSize = width * height * channels * sizeof(unsigned char);
    size_t outputSize = width * height * 3 * sizeof(unsigned char);

    cudaMalloc((void **)&d_input, inputSize);
    cudaMalloc((void **)&d_output, outputSize);

    cudaMemcpy(d_input, h_input, inputSize, cudaMemcpyHostToDevice);

    dim3 blockSize(blockDim, blockDim);
    dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);

    box_blur_kernel<<<gridSize, blockSize>>>(d_input, d_output, width, height, channels, kernelSize);

    cudaMemcpy(h_output, d_output, outputSize, cudaMemcpyDeviceToHost);

    cudaFree(d_input);
    cudaFree(d_output);
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        printf("Box Blur - CUDA GPU Implementation\n");
        printf("Usage: %s photo.jpg output.jpg [block_size]\n", argv[0]);
        printf("Supports: JPG, PNG, BMP\n");
        printf("block_size: threads per block dimension (default: 16, max: 32)\n");
        printf("  - Block 8x8   = 64 threads/block\n");
        printf("  - Block 16x16 = 256 threads/block (default)\n");
        printf("  - Block 32x32 = 1024 threads/block (max)\n");
        return EXIT_FAILURE;
    }

    // Parse block size parameter
    int block_dim = 16;  // Default
    if (argc == 4) {
        block_dim = atoi(argv[3]);
        if (block_dim < 1 || block_dim > 32) {
            fprintf(stderr, "Error: block_size must be 1-32 (1-1024 threads/block)\n");
            return EXIT_FAILURE;
        }
    }

    printf("=== CUDA Box Blur ===\n");
    printf("Input: %s\n", argv[1]);
    printf("Output: %s\n", argv[2]);
    printf("Block size: %dx%d (%d threads/block)\n", block_dim, block_dim, block_dim * block_dim);

    int width, height, channels;
    unsigned char *input_rgb = stbi_load(argv[1], &width, &height, &channels, 0);
    
    if (input_rgb == NULL) {
        fprintf(stderr, "Error: Cannot read %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Loaded: %dx%d, %d channel(s)\n", width, height, channels);

    int kernel_size = 5;
    printf("Kernel: %dx%d box blur\n", kernel_size, kernel_size);
    printf("\nProcessing on GPU...\n");

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    
    unsigned char *output_rgb = (unsigned char*)malloc(width * height * 3);
    if (!output_rgb) {
        fprintf(stderr, "Memory allocation failed.\n");
        stbi_image_free(input_rgb);
        return EXIT_FAILURE;
    }

    cuda_box_blur(input_rgb, output_rgb, width, height, channels, kernel_size, block_dim);
    
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    float ms = 0;
    cudaEventElapsedTime(&ms, start, stop);
    double time_sec = ms / 1000.0;

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

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    free(output_rgb);
    stbi_image_free(input_rgb);
    
    return EXIT_SUCCESS;
}
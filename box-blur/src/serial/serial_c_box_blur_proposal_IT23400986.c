/*
 * Serial 2D Convolution (Box Blur)
 *
 * This code generates a sample 2D 'image' and applies a 3x3 box blur
 * kernel to it. It handles boundaries by skipping the 1-pixel border.
 * A more robust implementation would use padding or edge clamping.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Image dimensions (large for performance testing)
#define WIDTH 1024
#define HEIGHT 1024

// Kernel dimensions
#define K_SIZE 3 

// 2D arrays for images. For larger sizes, malloc would be better.
static int input_image[HEIGHT][WIDTH];
static int output_image[HEIGHT][WIDTH];

// A 3x3 Box Blur kernel (all weights are 1)
static int kernel[K_SIZE][K_SIZE] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1}
};
static int kernel_factor = 9; // Sum of kernel weights

// Initialize the image with random pixel values (0-255)
void initialize_image() {
    srand(time(NULL));
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            input_image[i][j] = rand() % 256;
        }
    }
}

// Perform serial 2D convolution
void serial_convolution() {
    int k_center = K_SIZE / 2;

    // Iterate over each pixel in the image
    // We skip the border pixels (from 1 to HEIGHT-1) for simplicity
    for (int i = k_center; i < HEIGHT - k_center; i++) {
        for (int j = k_center; j < WIDTH - k_center; j++) {
            
            int sum = 0; // Accumulator for the weighted sum

            // Apply the kernel
            for (int ki = 0; ki < K_SIZE; ki++) {
                for (int kj = 0; kj < K_SIZE; kj++) {
                    
                    // Get the corresponding input pixel
                    int r = i + ki - k_center;
                    int c = j + kj - k_center;
                    
                    sum += input_image[r][c] * kernel[ki][kj];
                }
            }
            
            // Write the blurred pixel to the output image
            output_image[i][j] = sum / kernel_factor;
        }
    }
}

int main() {
    initialize_image();
    printf("Performing serial 2D convolution on %d x %d image...\n", HEIGHT, WIDTH);

    clock_t start = clock();
    serial_convolution();
    clock_t end = clock();

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Convolution complete.\n");
    printf("Time taken: %f seconds\n", time_spent);

    // Print a sample for verification
    printf("\nSample Output (Top-Left 5x5 of blurred area):\n");
    for(int i = 1; i <= 5; i++) {
        for (int j = 1; j <= 5; j++) {
            printf("%d\t", output_image[i][j]);
        }
        printf("\n");
    }

    return 0;
}

/* * Citation: This code is a standard textbook implementation of a 2D convolution,
 * adapted for this proposal. The core logic is based on common 
 * digital image processing stencil operations.
 */
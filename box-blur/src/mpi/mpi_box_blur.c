#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Blur three channels; input may be 1-channel or 3+/4-channel. Only RGB channels are processed; alpha is ignored.
void apply_box_blur_mpi(const unsigned char *input, unsigned char *output_rgb, int width, int height, int channels, int kernel_size, int start_row, int end_row) {
    int k_offset = kernel_size / 2;

    for (int y = start_row; y < end_row; y++) {
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

                int local_y = y - start_row;
                int dst_idx = (local_y * width + x) * 3 + c;
                output_rgb[dst_idx] = (unsigned char)(sum / count);
            }
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            printf("Box Blur - MPI Distributed\n");
            printf("Usage: mpirun -np 4 %s photo.jpg output.jpg\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    unsigned char *input_rgb = NULL;
    unsigned char *output_rgb_root = NULL;
    int width = 0, height = 0, channels = 0;
    int rows_per_process, start_row, end_row;
    int kernel_size = 5;
    double start_time, end_time;

    // Root process loads the image
    if (rank == 0) {
        printf("=== MPI Box Blur ===\n");
        printf("Input: %s\n", argv[1]);
        printf("Output: %s\n", argv[2]);
        printf("Processes: %d\n", size);

        input_rgb = stbi_load(argv[1], &width, &height, &channels, 0);
        if (input_rgb == NULL) {
            fprintf(stderr, "Error: Cannot read %s\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        
        printf("Loaded: %dx%d, %d channel(s)\n", width, height, channels);
        
        printf("Kernel: %dx%d box blur\n", kernel_size, kernel_size);
        printf("\nProcessing...\n");
    }

    // Broadcast image dimensions and channels
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate rows per process
    rows_per_process = height / size;
    start_row = rank * rows_per_process;
    end_row = (rank == size - 1) ? height : start_row + rows_per_process;
    int my_rows = end_row - start_row;

    // Allocate buffers for non-root processes
    if (rank != 0) {
        input_rgb = (unsigned char*)malloc(width * height * channels);
        if (!input_rgb) {
            fprintf(stderr, "Memory allocation failed on rank %d.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    if (rank == 0) {
        output_rgb_root = (unsigned char*)malloc(width * height * 3);
        if (!output_rgb_root) {
            fprintf(stderr, "Memory allocation failed on root.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    unsigned char *my_output = (unsigned char*)malloc(width * my_rows * 3);
    if (!my_output) {
        fprintf(stderr, "Memory allocation failed on rank %d.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Start timing after setup
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // Broadcast entire image to all processes
    MPI_Bcast(input_rgb, width * height * channels, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Each process works on its assigned rows
    apply_box_blur_mpi(input_rgb, my_output, width, height, channels, kernel_size, start_row, end_row);

    // Gather results back to root
    int *recv_counts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        recv_counts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            int i_start = i * rows_per_process;
            int i_end = (i == size - 1) ? height : i_start + rows_per_process;
            recv_counts[i] = (i_end - i_start) * width * 3;
            displs[i] = i_start * width * 3;
        }
    }

    MPI_Gatherv(my_output, my_rows * width * 3, MPI_UNSIGNED_CHAR,
                output_rgb_root, recv_counts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    // Root process saves the output
    if (rank == 0) {
        double elapsed_time = end_time - start_time;
        
        // Auto-detect output format
        int ok = 0;
        if (strstr(argv[2], ".png")) ok = stbi_write_png(argv[2], width, height, 3, output_rgb_root, width*3);
        else if (strstr(argv[2], ".jpg")) ok = stbi_write_jpg(argv[2], width, height, 3, output_rgb_root, 90);
        else ok = stbi_write_bmp(argv[2], width, height, 3, output_rgb_root);
        
        if (!ok) {
            fprintf(stderr, "Error writing output\n");
        } else {
            printf("\n=== Results ===\n");
            printf("Time: %.6f seconds\n", elapsed_time);
            printf("Pixels: %d\n", width * height);
            printf("Speed: %.2f Mpixels/sec\n\n", (width * height) / (elapsed_time * 1000000));
        }

        free(output_rgb_root);
        free(recv_counts);
        free(displs);
    }
    
    free(input_rgb);
    free(my_output);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}
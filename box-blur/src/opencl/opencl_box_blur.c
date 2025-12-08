/*
 * OpenCL Box Blur Implementation
 * 
 * This implementation uses OpenCL for GPU acceleration and works with
 * AMD Radeon, NVIDIA, and Intel GPUs. It's cross-platform and vendor-neutral.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// OpenCL kernel source code (embedded as string)
const char *kernel_source = 
"__kernel void box_blur_kernel(__global unsigned char *input,\n"
"                               __global unsigned char *output,\n"
"                               int width,\n"
"                               int height,\n"
"                               int channels,\n"
"                               int kernel_size) {\n"
"    int x = get_global_id(0);\n"
"    int y = get_global_id(1);\n"
"    \n"
"    if (x >= width || y >= height) return;\n"
"    int k_offset = kernel_size / 2;\n"
"    \n"
"    for (int c = 0; c < 3; c++) {\n"
"        int sum = 0;\n"
"        int count = 0;\n"
"        for (int m = -k_offset; m <= k_offset; m++) {\n"
"            for (int n = -k_offset; n <= k_offset; n++) {\n"
"                int ix = x + n;\n"
"                int iy = y + m;\n"
"                if (ix >= 0 && ix < width && iy >= 0 && iy < height) {\n"
"                    int src_idx = (iy * width + ix) * channels + (channels == 1 ? 0 : c);\n"
"                    sum += input[src_idx];\n"
"                    count++;\n"
"                }\n"
"            }\n"
"        }\n"
"        int dst_idx = (y * width + x) * 3 + c;\n"
"        output[dst_idx] = (unsigned char)(sum / count);\n"
"    }\n"
"}\n";

/**
 * Check OpenCL error and print message if error occurs
 */
void check_error(cl_int err, const char *operation) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error during operation '%s': %d\n", operation, err);
        exit(EXIT_FAILURE);
    }
}

/**
 * Apply Box blur using OpenCL
 */
double apply_box_blur_opencl(unsigned char *input_image, unsigned char *output_image, 
                           int width, int height, int channels, int kernel_size) {
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel opencl_kernel;
    cl_mem d_input, d_output;
    cl_event event;
    cl_ulong start_time, end_time;
    
    // Step 1: Get platform
    err = clGetPlatformIDs(1, &platform, NULL);
    check_error(err, "Getting platform");
    
    // Step 2: Get device (prefer GPU, fallback to CPU)
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        printf("No GPU found, using CPU instead.\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        check_error(err, "Getting device");
    }
    
    // Print device info
    char device_name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    printf("Using OpenCL device: %s\n", device_name);
    
    // Step 3: Create context
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    check_error(err, "Creating context");
    
    // Step 4: Create command queue with profiling enabled
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    #pragma GCC diagnostic pop
    check_error(err, "Creating command queue");
    
    // Step 5: Create and build program
    program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    check_error(err, "Creating program");
    
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        // Print build log if compilation fails
        char build_log[4096];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, NULL);
        fprintf(stderr, "Build log:\n%s\n", build_log);
        check_error(err, "Building program");
    }
    
    // Step 6: Create kernel
    opencl_kernel = clCreateKernel(program, "box_blur_kernel", &err);
    check_error(err, "Creating kernel");
    
    // Step 7: Allocate device memory
    size_t image_size = width * height * channels * sizeof(unsigned char);
    size_t output_size = width * height * 3 * sizeof(unsigned char);
    
    d_input = clCreateBuffer(context, CL_MEM_READ_ONLY, image_size, NULL, &err);
    check_error(err, "Creating input buffer");
    
    d_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, output_size, NULL, &err);
    check_error(err, "Creating output buffer");
    
    // Step 8: Copy data to device
    err = clEnqueueWriteBuffer(queue, d_input, CL_TRUE, 0, image_size, input_image, 0, NULL, NULL);
    check_error(err, "Copying input to device");
    
    // Step 9: Set kernel arguments
    err = clSetKernelArg(opencl_kernel, 0, sizeof(cl_mem), &d_input);
    err |= clSetKernelArg(opencl_kernel, 1, sizeof(cl_mem), &d_output);
    err |= clSetKernelArg(opencl_kernel, 2, sizeof(int), &width);
    err |= clSetKernelArg(opencl_kernel, 3, sizeof(int), &height);
    err |= clSetKernelArg(opencl_kernel, 4, sizeof(int), &channels);
    err |= clSetKernelArg(opencl_kernel, 5, sizeof(int), &kernel_size);
    check_error(err, "Setting kernel arguments");
    
    // Step 10: Execute kernel with timing
    size_t global_work_size[2] = {width, height};
    size_t local_work_size[2] = {16, 16};
    
    err = clEnqueueNDRangeKernel(queue, opencl_kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
    check_error(err, "Executing kernel");
    
    clWaitForEvents(1, &event);
    
    // Get timing info
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start_time), &start_time, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end_time), &end_time, NULL);
    double elapsed_time = (end_time - start_time) / 1e9; // Convert nanoseconds to seconds
    
    // Step 11: Copy result back to host
    err = clEnqueueReadBuffer(queue, d_output, CL_TRUE, 0, output_size, output_image, 0, NULL, NULL);
    check_error(err, "Copying result to host");
    
    // Cleanup
    clReleaseEvent(event);
    // Step 12: Cleanup
    clReleaseMemObject(d_input);
    clReleaseMemObject(d_output);
    clReleaseKernel(opencl_kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    return elapsed_time;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Box Blur - OpenCL GPU (AMD/NVIDIA/Intel)\n");
        printf("Usage: %s photo.jpg output.jpg\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("=== OpenCL Box Blur ===\n");
    printf("Input: %s\n", argv[1]);
    printf("Output: %s\n", argv[2]);

    int width, height, channels;
    unsigned char *input_rgb = stbi_load(argv[1], &width, &height, &channels, 0);
    
    if (input_rgb == NULL) {
        fprintf(stderr, "Error: Cannot read %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Loaded: %dx%d, %d channel(s)\n", width, height, channels);

    // Convert to grayscale
    unsigned char *output_rgb = (unsigned char*)malloc(width * height * 3);
    if (!output_rgb) {
        fprintf(stderr, "Memory allocation failed.\n");
        stbi_image_free(input_rgb);
        return EXIT_FAILURE;
    }

    int kernel_size = 5;
    printf("Kernel: %dx%d box blur\n", kernel_size, kernel_size);
    printf("\nProcessing on GPU...\n");

    // Apply box blur using OpenCL
    double elapsed_time = apply_box_blur_opencl(input_rgb, output_rgb, width, height, channels, kernel_size);

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
    printf("Time: %.6f seconds\n", elapsed_time);
    printf("Pixels: %d\n", width * height);
    printf("Speed: %.2f Mpixels/sec\n\n", (width * height) / (elapsed_time * 1000000));

    free(output_rgb);
    stbi_image_free(input_rgb);
    
    return EXIT_SUCCESS;
}

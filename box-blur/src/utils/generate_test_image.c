/*
 * Test Image Generator
 * 
 * Generates test images in PGM format with various patterns
 * for benchmarking Box blur implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "image_io.h"

void generate_gradient(unsigned char *image, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i * width + j] = (unsigned char)((i + j) % 256);
        }
    }
}

void generate_checkerboard(unsigned char *image, int width, int height, int square_size) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if ((i / square_size + j / square_size) % 2 == 0) {
                image[i * width + j] = 255;
            } else {
                image[i * width + j] = 0;
            }
        }
    }
}

void generate_circle(unsigned char *image, int width, int height) {
    int cx = width / 2;
    int cy = height / 2;
    int radius = (width < height ? width : height) / 3;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int dx = j - cx;
            int dy = i - cy;
            int dist_sq = dx * dx + dy * dy;
            
            if (dist_sq < radius * radius) {
                image[i * width + j] = 255;
            } else {
                image[i * width + j] = 50;
            }
        }
    }
}

void generate_noise(unsigned char *image, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i * width + j] = (unsigned char)(rand() % 256);
        }
    }
}

void generate_concentric_circles(unsigned char *image, int width, int height) {
    int cx = width / 2;
    int cy = height / 2;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int dx = j - cx;
            int dy = i - cy;
            double dist = sqrt(dx * dx + dy * dy);
            image[i * width + j] = (unsigned char)((int)(dist * 2) % 256);
        }
    }
}

void generate_photo_realistic(unsigned char *image, int width, int height) {
    // Create a landscape-like scene with sky, mountains, and ground
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int value;
            
            // Sky gradient (top third)
            if (i < height / 3) {
                value = 200 - (i * 50 / height); // Lighter at top
            }
            // Mountain range (middle third)
            else if (i < 2 * height / 3) {
                int mountain_height = (int)(50 * sin(j * 0.02) + 30 * cos(j * 0.05));
                int horizon = height / 3;
                if (i < horizon + mountain_height) {
                    value = 100 - ((i - horizon) / 2); // Mountain shadows
                } else {
                    value = 150; // Sky behind mountains
                }
            }
            // Ground with texture (bottom third)
            else {
                int base = 80;
                int texture = (j % 7 + i % 5) * 3; // Ground texture
                value = base + texture;
            }
            
            // Add some noise for realism
            value += (rand() % 20) - 10;
            
            // Clamp to valid range
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            
            image[i * width + j] = (unsigned char)value;
        }
    }
}

void generate_portrait(unsigned char *image, int width, int height) {
    // Create a simple face-like pattern
    int cx = width / 2;
    int cy = height / 2;
    int face_radius = (width < height ? width : height) / 3;
    
    // Background
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i * width + j] = 200; // Light background
        }
    }
    
    // Face (circle)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int dx = j - cx;
            int dy = i - cy;
            int dist_sq = dx * dx + dy * dy;
            
            if (dist_sq < face_radius * face_radius) {
                image[i * width + j] = 180; // Skin tone
            }
        }
    }
    
    // Eyes
    int eye_y = cy - face_radius / 4;
    int eye1_x = cx - face_radius / 3;
    int eye2_x = cx + face_radius / 3;
    int eye_radius = face_radius / 8;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Left eye
            int dx1 = j - eye1_x;
            int dy1 = i - eye_y;
            if (dx1*dx1 + dy1*dy1 < eye_radius*eye_radius) {
                image[i * width + j] = 50; // Dark eye
            }
            
            // Right eye
            int dx2 = j - eye2_x;
            int dy2 = i - eye_y;
            if (dx2*dx2 + dy2*dy2 < eye_radius*eye_radius) {
                image[i * width + j] = 50; // Dark eye
            }
        }
    }
    
    // Mouth (arc)
    int mouth_y = cy + face_radius / 3;
    for (int j = cx - face_radius/2; j < cx + face_radius/2; j++) {
        int dx = j - cx;
        int arc_y = mouth_y + (int)(sqrt(abs(face_radius*face_radius/4 - dx*dx)) / 3);
        if (arc_y >= 0 && arc_y < height && j >= 0 && j < width) {
            for (int thick = -2; thick <= 2; thick++) {
                if (arc_y + thick >= 0 && arc_y + thick < height) {
                    image[(arc_y + thick) * width + j] = 40; // Mouth
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Test Image Generator\n");
        printf("====================\n");
        printf("Usage: %s <output_directory>\n\n", argv[0]);
        printf("This will generate test images of various sizes:\n");
        printf("  - 256x256 (small)\n");
        printf("  - 512x512 (medium)\n");
        printf("  - 1024x1024 (large)\n");
        printf("  - 2048x2048 (very large)\n\n");
        return EXIT_FAILURE;
    }

    const char *output_dir = argv[1];
    
    int sizes[] = {256, 512, 1024, 2048};
    int num_sizes = 4;
    
    printf("Generating test images in directory: %s\n", output_dir);
    printf("=========================================\n\n");
    
    for (int s = 0; s < num_sizes; s++) {
        int size = sizes[s];
        unsigned char *image = (unsigned char *)malloc(size * size * sizeof(unsigned char));
        
        if (image == NULL) {
            fprintf(stderr, "Memory allocation failed for %dx%d image\n", size, size);
            continue;
        }
        
        char filename[256];
        
        // Generate gradient
        generate_gradient(image, size, size);
        snprintf(filename, sizeof(filename), "%s/gradient_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        // Generate checkerboard
        generate_checkerboard(image, size, size, 32);
        snprintf(filename, sizeof(filename), "%s/checkerboard_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        // Generate circle
        generate_circle(image, size, size);
        snprintf(filename, sizeof(filename), "%s/circle_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        // Generate concentric circles
        generate_concentric_circles(image, size, size);
        snprintf(filename, sizeof(filename), "%s/rings_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        // Generate noise (only for smaller sizes to avoid huge random data)
        if (size <= 1024) {
            generate_noise(image, size, size);
            snprintf(filename, sizeof(filename), "%s/noise_%dx%d.bmp", output_dir, size, size);
            write_image(filename, image, size, size);
            printf("Created: %s\n", filename);
        }
        
        // Generate landscape
        generate_photo_realistic(image, size, size);
        snprintf(filename, sizeof(filename), "%s/landscape_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        // Generate portrait
        generate_portrait(image, size, size);
        snprintf(filename, sizeof(filename), "%s/portrait_%dx%d.bmp", output_dir, size, size);
        write_image(filename, image, size, size);
        printf("Created: %s\n", filename);
        
        printf("\n");
        free(image);
    }
    
    printf("All test images generated successfully!\n");
    return EXIT_SUCCESS;
}

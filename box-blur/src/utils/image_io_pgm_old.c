#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_io.h"

// Read PGM image file (P5 binary or P2 ASCII format)
unsigned char* read_image(const char* filename, int* width, int* height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    char format[3];
    if (fscanf(file, "%2s", format) != 1 || (strcmp(format, "P5") != 0 && strcmp(format, "P2") != 0)) {
        fprintf(stderr, "Invalid PGM format. Expected P5 or P2.\\n");
        fclose(file);
        return NULL;
    }

    // Skip whitespace and comments
    int ch;
    do {
        ch = fgetc(file);
        if (ch == '#') {
            // Skip comment line
            while (ch != '\\n' && ch != EOF) {
                ch = fgetc(file);
            }
        }
    } while (ch == ' ' || ch == '\\t' || ch == '\\n' || ch == '\\r' || ch == '#');
    
    // Put back the last character we read
    if (ch != EOF) {
        ungetc(ch, file);
    }

    // Read width, height, and max gray value
    int max_val;
    int scan_result = fscanf(file, "%d %d %d", width, height, &max_val);
    
    if (scan_result != 3) {
        fprintf(stderr, "Invalid PGM header (scanned %d values, expected 3).\n", scan_result);
        fprintf(stderr, "Width: %d, Height: %d\n", *width, *height);
        fclose(file);
        return NULL;
    }

    if (max_val != 255) {
        fprintf(stderr, "Only 8-bit PGM images supported (max value = %d, expected 255).\n", max_val);
        fclose(file);
        return NULL;
    }

    // Skip any remaining whitespace/newline after header
    while (fgetc(file) != '\n' && !feof(file));

    unsigned char *image = (unsigned char*)malloc((*width) * (*height) * sizeof(unsigned char));
    if (!image) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    if (strcmp(format, "P5") == 0) {
        // Binary format
        if (fread(image, sizeof(unsigned char), (*width) * (*height), file) != (*width) * (*height)) {
            fprintf(stderr, "Error reading image data.\n");
            free(image);
            fclose(file);
            return NULL;
        }
    } else {
        // ASCII format
        for (int i = 0; i < (*width) * (*height); i++) {
            int pixel;
            if (fscanf(file, "%d", &pixel) != 1) {
                fprintf(stderr, "Error reading pixel data.\n");
                free(image);
                fclose(file);
                return NULL;
            }
            image[i] = (unsigned char)pixel;
        }
    }

    fclose(file);
    return image;
}

// Write PGM image file (P5 binary format)
int write_image(const char* filename, unsigned char* image, int width, int height) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return -1;
    }

    fprintf(file, "P5\n");
    fprintf(file, "# Created by Box Blur Program\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n");

    if (fwrite(image, sizeof(unsigned char), width * height, file) != width * height) {
        fprintf(stderr, "Error writing image data.\n");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}
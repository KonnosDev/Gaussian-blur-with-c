#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PI 3.14159265358979323846

double gaussian_1d(int x, double sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

double* create_kernel(int size, double sigma) {
    int half = size >> 1;
    double sum = 0.0;
    double* kernel = malloc(size * sizeof(double));

    for (int x = -half; x <= half; x++) {
        double value = gaussian_1d(x, sigma);
        kernel[x + half] = value;
        sum += value;
    }

    for (int i = 0; i < size; i++)
        kernel[i] /= sum;

    return kernel;
}

void gaussian_blur(unsigned char* input, unsigned char* output,
                   int width, int height, int channels,
                   double* kernel, int ksize)
{
    int half = ksize >> 1;
    int width_minus_1 = width - 1;
    int height_minus_1 = height - 1;
    double* temp = malloc(width * height * channels * sizeof(double));

    //X
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                double sum = 0.0;

                for (int kx = -half; kx <= half; kx++) {
                    int px = x + kx;
                    
                    
                    int neg_mask = px >> 31;  // All 1s if negative
                    int overflow_mask = (px - width_minus_1) >> 31;  // All 1s if px <= width-1
                    overflow_mask = ~overflow_mask;  // All 1s if px > width-1
                    
                    // if negative -> 0, if overflow -> width-1, else keep px
                    px = ((px & ~neg_mask & ~overflow_mask) | 
                         (0 & neg_mask) | 
                         (width_minus_1 & overflow_mask));

                    int img_idx = (y * width + px) * channels + c;
                    sum += input[img_idx] * kernel[kx + half];
                }

                temp[(y * width + x) * channels + c] = sum;
            }
        }
    }

    //Y 
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                double sum = 0.0;

                for (int ky = -half; ky <= half; ky++) {
                    int py = y + ky;
                    
                    
                    int neg_mask = py >> 31;
                    int overflow_mask = (py - height_minus_1) >> 31;
                    overflow_mask = ~overflow_mask;
                    
                    py = ((py & ~neg_mask & ~overflow_mask) | 
                         (0 & neg_mask) | 
                         (height_minus_1 & overflow_mask));

                    int tmp_idx = (py * width + x) * channels + c;
                    sum += temp[tmp_idx] * kernel[ky + half];
                }

                output[(y * width + x) * channels + c] = (unsigned char)(sum + 0.5);
            }
        }
    }

    free(temp);
}

int main(int argc, char** argv) {
    // Argument check using bitwise AND short-circuit
    (argc < 5) && (printf("Usage: %s input.png output.png kernel_size sigma\n", argv[0]), exit(1), 0);
    
    int width, height, channels;
    unsigned char* image = stbi_load(argv[1], &width, &height, &channels, 0);
    
    // Image load check
    (image == NULL) && (printf("Failed to load image\n"), exit(1), 0);
    
    int kernel_size = atoi(argv[3]);
    double sigma = atof(argv[4]);
    
    // Kernel validation: must be positive AND odd
    int kernel_invalid = (kernel_size <= 0) | (((kernel_size & 1) ^ 1));
    kernel_invalid && (printf("Error: kernel size must be a positive odd number.\n"), exit(1), 0);
    
    // Sigma validation
    (sigma <= 0.0) && (printf("Error: sigma must be > 0.\n"), exit(1), 0);
    
    unsigned char* output = malloc(width * height * channels);
    
    double* kernel = create_kernel(kernel_size, sigma);
    gaussian_blur(image, output, width, height, channels, kernel, kernel_size);
    
    stbi_write_png(argv[2], width, height, channels, output, width * channels);
    
    stbi_image_free(image);
    free(output);
    free(kernel);
    
    printf("Gaussian blur applied successfully!!\n");
    return 0;
}

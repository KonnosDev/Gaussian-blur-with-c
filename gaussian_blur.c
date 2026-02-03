#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PI 3.14159265358979323846

// Gaussian function 
double gaussian(int x, int y, double sigma) {
    return (1.0 / (2.0 * PI * sigma * sigma)) *
           exp(-(x*x + y*y) / (2.0 * sigma * sigma));
}

// Create Gaussian kernel 
double* create_kernel(int size, double sigma) {
    int half = size / 2;
    double sum = 0.0;
    double* kernel = malloc(size * size * sizeof(double));

    for (int y = -half; y <= half; y++)
    {
        for (int x = -half; x <= half; x++)
        {
            double value = gaussian(x, y, sigma);
            kernel[(y + half) * size + (x + half)] = value;
            sum += value;
        }
    }

    // Normalize kernel 
    for (int i = 0; i < size * size; i++)
        kernel[i] /= sum;

    return kernel;
}

// Apply Gaussian blur 
void gaussian_blur(unsigned char* input, unsigned char* output,
                   int width, int height, int channels,
                   double* kernel, int ksize)
{
    int half = ksize / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                double sum = 0.0;

                for (int ky = -half; ky <= half; ky++) {
                    for (int kx = -half; kx <= half; kx++) {
                        int px = x + kx;
                        int py = y + ky;

                        if (px >= 0 && px < width && py >= 0 && py < height)
                        {
                            int img_idx = (py * width + px) * channels + c;
                            int ker_idx = (ky + half) * ksize + (kx + half);
                            sum += input[img_idx] * kernel[ker_idx];
                        }
                    }
                }

                output[(y * width + x) * channels + c] = (unsigned char)sum;
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 5) {
        printf("Usage: %s input.png output.png kernel_size sigma\n", argv[0]);
        return 1;
    }

    int width, height, channels;
    unsigned char* image = stbi_load(argv[1], &width, &height, &channels, 0);

    if (!image) {
        printf("Failed to load image\n");
        return 1;
    }

    int kernel_size = atoi(argv[3]);   // IMPORTANT: Must be odd
    double sigma = atof(argv[4]);       // weight option

    //Kernel and Sigma checks
    if (kernel_size <= 0 || kernel_size % 2 == 0) {
    printf("Error: kernel size must be a positive odd number.\n");
    return 1;
    }

    if (sigma <= 0.0) {
    printf("Error: sigma must be > 0.\n");
    return 1;
    }

    unsigned char* output = malloc(width * height * channels);

    double* kernel = create_kernel(kernel_size, sigma);
    gaussian_blur(image, output, width, height, channels, kernel, kernel_size);

    stbi_write_png(argv[2], width, height, channels, output, width * channels);

    stbi_image_free(image);
    free(output);
    free(kernel);

    printf("Gaussian blur applied successfully.\n");
    return 0;
}


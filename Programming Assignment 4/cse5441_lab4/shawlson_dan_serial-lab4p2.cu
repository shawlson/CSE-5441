#include <stdio.h>
#include <time.h>
#include <math.h>

extern "C" {
#include "read_bmp.h"
}

#define BLACK 0
#define WHITE 255

int main(int argc, char *argv[]) {

    FILE *in_file, *out_file;
    uint8_t *input_bmp, *output_bmp;
    uint32_t width, height;
    struct timespec time_before, time_after;
    unsigned long time_dif; // in milliseconds
    uint32_t threshold, black_cell_count;
    int i, j;
    uint32_t gradient_x, gradient_y, magnitude;

    if (argc != 3) {
        fprintf(stderr, "usage: ./lab4p2_serial in_file.bmp out_file.bmp\n");
        exit(-1);
    }

    in_file = fopen(argv[1], "rb");
    out_file = fopen(argv[2], "wb");

    // Read input bmp into buffer
    input_bmp = (uint8_t *) read_bmp_file(in_file);

    // Allocate space for output image
    output_bmp = (uint8_t *) malloc(get_num_pixel());
    width = get_image_width();
    height = get_image_height();

    // Start the timer
    clock_gettime(CLOCK_REALTIME, &time_before);

    // Sobel loop
    threshold = 0;
    black_cell_count = 0;

    while (black_cell_count < (width * height * 75 / 100)) {

        black_cell_count = 0;
        threshold += 1;

        for (i = 1; i < height - 1; ++i) {
            for (j = 1; j < width -1; ++j) {
                gradient_x = input_bmp[(i - 1) * width + (j + 1)] - input_bmp[(i - 1) * width + (j - 1)] \
                            + 2 * input_bmp[i * width + (j + 1)] - 2 * input_bmp[i * width + (j - 1)] \
                            + input_bmp[(i + 1) * width + (j + 1)] - input_bmp[(i + 1) * width + (j - 1)];

                gradient_y = input_bmp[(i - 1) * width + (j - 1)] + 2 * input_bmp[(i - 1) * width + j] \
                            + input_bmp[(i - 1) * width + (j + 1)] - input_bmp[(i + 1) * width + (j - 1)] \
                            - 2 * input_bmp[(i + 1) * width + j] - input_bmp[(i + 1) * width + (j + 1)];

                magnitude = sqrt(gradient_x * gradient_x + gradient_y * gradient_y);
                if (magnitude > threshold) {
                    output_bmp[i * width + j] = WHITE;
                }
                else {
                    output_bmp[i * width + j] = BLACK;
                    ++black_cell_count;
                }
            }
        }
    }

    // End timer and print results
    clock_gettime(CLOCK_REALTIME, &time_after);
    time_dif = ((time_after.tv_sec - time_before.tv_sec) * 1000) \
               + ((time_after.tv_nsec - time_before.tv_nsec) / 1000000);

    printf("Serial time: %d (ms)\n", time_dif);
    printf("Serial threshold: %d\n", threshold);

    // Write output bmp data to file
    write_bmp_file(out_file, output_bmp);    
}

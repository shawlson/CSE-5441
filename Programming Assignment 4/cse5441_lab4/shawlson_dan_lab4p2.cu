#include <stdio.h>
#include <time.h>
#include <math.h>

extern "C" {
#include "read_bmp.h"
}

#define BLACK 0
#define WHITE 255

__global__ void horizontal_threads(uint8_t *input_bmp, uint8_t *output_bmp, uint32_t height, uint32_t width, uint32_t *final_thresh);
__global__ void vertical_threads(uint8_t *input_bmp, uint8_t *output_bmp, uint32_t height, uint32_t width, uint32_t *final_thresh);

int main(int argc, char *argv[]) {

    FILE *in_file, *out_file;
    uint8_t *input_bmp, *output_bmp;
    uint8_t *device_input, *device_output;
    uint32_t host_threshold, *device_threshold;
    uint32_t width, height;
    struct timespec time_before, time_after;
    unsigned long time_dif; // in milliseconds
    int num_threads;

    if (argc != 3) {
        fprintf(stderr, "usage: ./lab4p2 in_file.bmp out_file.bmp\n");
        exit(-1);
    }

    in_file = fopen(argv[1], "rb");
    out_file = fopen(argv[2], "wb");

    // Read input bmp into buffer
    input_bmp = (uint8_t *) read_bmp_file(in_file);

    // Allocate space for output image on host
    output_bmp = (uint8_t *) malloc(get_num_pixel());

    // Allocate space for input image and output image on device
    cudaMalloc((void **) &device_input, get_num_pixel());
    cudaMalloc((void **) &device_output, get_num_pixel());

    // Allocate space for threshold count on device, so it can be copied back
    // to the host at the end
    cudaMalloc((void **) &device_threshold, sizeof(uint32_t));

    // Copy input bmp to device
    cudaMemcpy(device_input, input_bmp, get_num_pixel(), cudaMemcpyHostToDevice);

    width = get_image_width();
    height = get_image_height();

    // Start the timer
    clock_gettime(CLOCK_REALTIME, &time_before);

    /*
     * Start the kernel. 
     * 
     * If the picture's width is greater than or equal to its
     * height, the threads will simultaneously calculate the gradient
     * operators on the pixels of a single row.
     * 
     * If the picture's height is greater than its width,
     * the threads will simultaneously calculate the gradient
     * operators on the pixels of a single column.
     * 
     * In either case, there will be one block and as many threads as the
     * number of pixels in the dimension that's being calculated, up
     * to a maximum of 1024 threads.
     */
    if (width >= height) {
        num_threads = (width - 2 > 1024)? 1024 : width - 2;
        dim3 blocks(1);
        dim3 threads_per_block(num_threads);
        horizontal_threads<<<blocks, threads_per_block>>>(device_input, device_output, height, width, device_threshold);
    }
    else {
        num_threads = (height - 2 > 1024)? 1024 : height - 2;
        dim3 blocks(1);
        dim3 threads_per_block(num_threads);
        vertical_threads<<<blocks, threads_per_block>>>(device_input, device_output, height, width, device_threshold);
    }

    // Copy output bmp and threshold back to host and free CUDA memory
    cudaMemcpy(output_bmp, device_output, get_num_pixel(), cudaMemcpyDeviceToHost);
    cudaMemcpy(&host_threshold, device_threshold, sizeof(uint32_t), cudaMemcpyDeviceToHost);
    cudaFree(device_input);
    cudaFree(device_output);
    cudaFree(device_threshold);

    // End timer and print results
    clock_gettime(CLOCK_REALTIME, &time_after);
    time_dif = ((time_after.tv_sec - time_before.tv_sec) * 1000) \
               + ((time_after.tv_nsec - time_before.tv_nsec) / 1000000);

    printf("CUDA time: %d (ms)\n", time_dif);
    printf("CUDA threshold: %d\n", host_threshold);

    // Write output bmp data to file
    write_bmp_file(out_file, output_bmp);    
}

__global__ void horizontal_threads(uint8_t *input_bmp, uint8_t *output_bmp, uint32_t height, uint32_t width, uint32_t *final_thresh) {

    __shared__ uint32_t black_cell_count;
    uint32_t threshold;
    int i, id;
    uint32_t gradient_x, gradient_y, magnitude;

    // We don't want any weird race conditions
    black_cell_count = 0;
    __syncthreads();

    threshold = 0;

    while (black_cell_count < (width * height * 75 / 100)) {

        // Again, no race conditions please. Let's all agree that black_cell_count
        // is 0 going forward
        black_cell_count = 0;
        __syncthreads();
        threshold += 1;

        for (i = 1; i < height - 1; ++i) {
            for (id = threadIdx.x + 1; id < width - 1; id += blockDim.x) {
                gradient_x = input_bmp[(i - 1) * width + (id + 1)] - input_bmp[(i - 1) * width + (id - 1)] \
                            + 2 * input_bmp[i * width + (id + 1)] - 2 * input_bmp[i * width + (id - 1)] \
                            + input_bmp[(i + 1) * width + (id + 1)] - input_bmp[(i + 1) * width + (id - 1)];

                gradient_y = input_bmp[(i - 1) * width + (id - 1)] + 2 * input_bmp[(i - 1) * width + id] \
                            + input_bmp[(i - 1) * width + (id + 1)] - input_bmp[(i + 1) * width + (id - 1)] \
                            - 2 * input_bmp[(i + 1) * width + id] - input_bmp[(i + 1) * width + (id + 1)];

                magnitude = sqrt((double) (gradient_x * gradient_x + gradient_y * gradient_y));

                if (magnitude > threshold) {
                    output_bmp[i * width + id] = WHITE;
                }
                else {
                    output_bmp[i * width + id] = BLACK;
                    atomicAdd(&black_cell_count, 1);
                }
            }
        }

        // Sync threads before next iteration of while loop
        __syncthreads();
    }

    // Clobber the hell out of this, it doesn't matter since all threads
    // have the same final value for threshold
    *final_thresh = threshold;
}

__global__ void vertical_threads(uint8_t *input_bmp, uint8_t *output_bmp, uint32_t height, uint32_t width, uint32_t *final_thresh) {
    
    __shared__ uint32_t black_cell_count;
    uint32_t threshold;
    int i, id;
    uint32_t gradient_x, gradient_y, magnitude;

    // We don't want any weird race conditions
    black_cell_count = 0;
    __syncthreads();

    threshold = 0;

    while (black_cell_count < (width * height * 75 / 100)) {

        // Again, no race conditions please. Let's all agree that black_cell_count
        // is 0 going forward
        black_cell_count = 0;
        __syncthreads();
        threshold += 1;

        for (i = 1; i < width - 1; ++i) {
            for (id = threadIdx.x + 1; id < height - 1; id += blockDim.x) {
                gradient_x = input_bmp[(id - 1) * width + (i + 1)] - input_bmp[(id - 1) * width + (i - 1)] \
                            + 2 * input_bmp[id * width + (i + 1)] - 2 * input_bmp[id * width + (i - 1)] \
                            + input_bmp[(id + 1) * width + (i + 1)] - input_bmp[(id + 1) * width + (i - 1)];

                gradient_y = input_bmp[(id - 1) * width + (i - 1)] + 2 * input_bmp[(id - 1) * width + i] \
                            + input_bmp[(id - 1) * width + (i + 1)] - input_bmp[(id + 1) * width + (i - 1)] \
                            - 2 * input_bmp[(id + 1) * width + i] - input_bmp[(id + 1) * width + (i + 1)];

                magnitude = sqrt((double) (gradient_x * gradient_x + gradient_y * gradient_y));

                if (magnitude > threshold) {
                    output_bmp[id * width + i] = WHITE;
                }
                else {
                    output_bmp[id * width + i] = BLACK;
                    atomicAdd(&black_cell_count, 1);
                }
            }
        }

        // Sync threads before next iteration of while loop
        __syncthreads();
    }

    // Clobber the hell out of this, it doesn't matter since all threads
    // have the same final value for threshold
    *final_thresh = threshold;
}

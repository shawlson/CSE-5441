#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024

__global__ void transpose_mult(double *matrix, double* result);

int main() {

    int i, j;
    int array_offset;
    double rand_val;
    size_t mem_size;
    double *host_matrix, *host_result;
    double *device_matrix, *device_result;
    struct timespec time_before, time_after;
    unsigned long time_dif; // in nanoseconds
    double gflops;
    
    #ifdef DEBUG
    int k;
    double *host_verify;
    int error;
    double host_entry;
    double GPU_entry;
    #endif
    
    // Allocate memory for host and device structures
    mem_size = MATRIX_SIZE * MATRIX_SIZE * sizeof(double);
    host_matrix = (double *) malloc(mem_size);
    host_result = (double *) malloc(mem_size);
    #ifdef DEBUG
    host_verify = (double *) malloc(mem_size);
    #endif
    cudaMalloc((void **) &device_matrix, mem_size);
    cudaMalloc((void **) &device_result, mem_size);

    /* 
     * Initialize host matrix with random values between 1.0 and
     * 2.0. Since CUDA doesn't make it easy to work with 2D arrays,
     * the matrix has been flattened to one dimension
     */
    srand(time(NULL));
    for (i = 0; i < MATRIX_SIZE; ++i) {
        for (j = 0; j < MATRIX_SIZE; ++j) {
            array_offset = i * MATRIX_SIZE;
            rand_val = 1.0 + (rand() / (RAND_MAX / (2.0 - 1.0)));
            host_matrix[array_offset + j] = rand_val;
        }
    }
    
    // Copy host matrix to GPU
    cudaMemcpy(device_matrix, host_matrix, mem_size, cudaMemcpyHostToDevice);

    // Start timer
    clock_gettime(CLOCK_REALTIME, &time_before);

    // Launch kernel
    dim3 blocks(MATRIX_SIZE);
    dim3 threads_per_block(MATRIX_SIZE);
    transpose_mult<<<blocks, threads_per_block>>>(device_matrix, device_result);

    // Copy result back to host and free CUDA memory
    cudaMemcpy(host_result, device_result, mem_size, cudaMemcpyDeviceToHost);
    cudaFree(device_matrix);
    cudaFree(device_result);

    // End timer and calculate GFLOPS
    clock_gettime(CLOCK_REALTIME, &time_after);
    time_dif = ((time_after.tv_sec - time_before.tv_sec) * 1000000000) + (time_after.tv_nsec - time_before.tv_nsec);
    gflops = ((double) MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE * 2.0) / (double) time_dif;
    printf("CUDA estimated GFLOPS: %lf\n", gflops);

    #ifdef DEBUG
    // Multiple matrix by its transpose serially to verify results
    for (i = 0; i < MATRIX_SIZE; ++i) {
        for (j = 0; j < MATRIX_SIZE; ++j) {
            for (k = 0; k < MATRIX_SIZE; ++k) {
                host_verify[i * MATRIX_SIZE + j] += host_matrix[k * MATRIX_SIZE + i] \
                * host_matrix[k * MATRIX_SIZE + j];
            }
        }
    }

    error = 0;
    for (i = 0; i < MATRIX_SIZE; ++i) {
        for (j = 0; j < MATRIX_SIZE; ++j) {
            host_entry = host_verify[i * MATRIX_SIZE + j];
            GPU_entry = host_result[i * MATRIX_SIZE + j];
            if (host_entry - GPU_entry > 1.0 || host_entry - GPU_entry < -1.0) {
                error = 1;
                printf("Serial had %lf, but cuda had %lf\n");
            }
        }
    }
    
    if (!error) printf("No errors!\n");
    #endif
}

__global__ void transpose_mult(double *matrix, double *result) {

    int i, j;
    int k;
    double entry = 0.0;

    i = blockIdx.x;
    j = threadIdx.x;

    for (k = 0; k < MATRIX_SIZE; ++k) {
        // entry += matrix[k][i] * matrix[k][j];
        entry += matrix[k * MATRIX_SIZE + i] * matrix[k * MATRIX_SIZE + j];
    }

    result[i * MATRIX_SIZE + j] = entry;
}

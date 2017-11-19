#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024

int main() {

    int i, j, k;
    static double matrix[MATRIX_SIZE][MATRIX_SIZE], result[MATRIX_SIZE][MATRIX_SIZE];
    struct timespec time_before, time_after;
    unsigned long time_dif; // in nanoseconds
    double gflops;
    
    // Initialize matrix with random values between 1.0 and 2.0
    srand(time(NULL));
    for (i = 0; i < MATRIX_SIZE; ++i) {
        for (j = 0; j < MATRIX_SIZE; ++j) {
            matrix[i][j] = 1.0 + (rand() / (RAND_MAX / (2.0 - 1.0)));
        }
    }
    
    // Start timer
    clock_gettime(CLOCK_REALTIME, &time_before);

    // Multiply matrix by its transpose
    for (i = 0; i < MATRIX_SIZE; ++i) {
        for (j = 0; j < MATRIX_SIZE; ++j) {
            for (k = 0; k < MATRIX_SIZE; ++k) {
                result[i][j] += matrix[k][i] * matrix[k][j];
            }
        }
    }

    // End timer and calculate GFLOPS
    clock_gettime(CLOCK_REALTIME, &time_after);
    time_dif = ((time_after.tv_sec - time_before.tv_sec) * 1000000000) + (time_after.tv_nsec - time_before.tv_nsec);
    gflops = ((double) MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE * 2.0) / (double) time_dif;
    printf("Serial estimated GFLOPS: %lf\n", gflops);
}

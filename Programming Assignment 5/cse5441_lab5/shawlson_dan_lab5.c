#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "read_bmp.h"

unsigned int start_row(unsigned int, unsigned int, unsigned int);
unsigned int end_row(unsigned int, unsigned int, unsigned int);

#define BLACK 0
#define WHITE 255

#define MASTER 0
#define PIXELS 0

int main(int argc, char *argv[]) {

    // Initalize MPI environment and get size of/rank within communicator
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Read image file (each process)
    FILE *in_file = fopen(argv[1], "rb");
    unsigned char *input_bmp = (unsigned char *) read_bmp_file(in_file);
    unsigned int width = get_image_width();
    unsigned int height = get_image_height();

    // Sync all processes before starting timer
    MPI_Barrier(MPI_COMM_WORLD);

    // Start timer (master only)
    struct timespec time_before;
    if (rank == MASTER) {
        clock_gettime(CLOCK_REALTIME, &time_before);
    }

    /* Sobel loop */
    int threshold = 0;
    unsigned int global_black_cell_count = 0;
    
    /* 
     * Each process will have a have a start row (inclusive) and
     * an end row (exclusive) for which it will perform the Sobel
     * calculations. We take into account the fact that two rows,
     * the first and last, need no calculations
     */
    int start = start_row(height - 2, size, rank) + 1;
    int end = end_row(height - 2, size, rank) + 1;
    unsigned char *output_bmp = (unsigned char *) calloc((end - start) * width, sizeof(unsigned char));

    #ifdef DEBUG
    printf("Process %d computing rows %d through %d\n\t...allocating %d bytes\n", 
            rank, start, end, (end - start) * width * sizeof(unsigned char));
    #endif

    while (global_black_cell_count < (width * height * 75 / 100)) {
        
        // Wait for all processes before starting iteration
        MPI_Barrier(MPI_COMM_WORLD);

        // black_cell_count and threshold are local to each process
        unsigned int black_cell_count = 0;
        ++threshold;

        // Sobel calculations
        int row, column;
        unsigned int gradient_x, gradient_y, magnitude;
        for (row = start; row < end; ++row) {
            for (column = 1; column < width - 1; ++column) {
                gradient_x = input_bmp[(row - 1) * width + (column + 1)] \
                             - input_bmp[(row - 1) * width + (column - 1)] \
                             + 2 * input_bmp[row * width + (column + 1)] \
                             - 2 * input_bmp[row * width + (column - 1)] \
                             + input_bmp[(row + 1) * width + (column + 1)] \
                             - input_bmp[(row + 1) * width + (column - 1)];

                gradient_y = input_bmp[(row - 1) * width + (column - 1)] \
                             + 2 * input_bmp[(row - 1) * width + column] \
                             + input_bmp[(row - 1) * width + (column + 1)] \
                             - input_bmp[(row + 1) * width + (column - 1)] \
                             - 2 * input_bmp[(row + 1) * width + column] \
                             - input_bmp[(row + 1) * width + (column + 1)];

                magnitude = sqrt(gradient_x * gradient_x + gradient_y * gradient_y);
                if (magnitude > threshold) {
                    output_bmp[(row - start) * width + column] = WHITE;
                }
                else {
                    output_bmp[(row - start) * width + column] = BLACK;
                    ++black_cell_count;
                }
            }
        }

        // Reduce black cell count among all processes
        MPI_Allreduce(&black_cell_count, &global_black_cell_count, 1, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);
    }

    // Send image pixels from slaves to master after convergence
    if (rank != MASTER) {
        MPI_Send(output_bmp, (end - start) * width, MPI_BYTE, MASTER, PIXELS, MPI_COMM_WORLD);
    }

    // From master, gather all image pixels, stop timer, and write image to file
    if (rank == MASTER) {
        unsigned char *final_bmp = (unsigned char *) calloc(get_num_pixel(), sizeof(unsigned char));
        unsigned int offset = width;
        memcpy(final_bmp + offset, output_bmp, (end - start) * width * sizeof(unsigned char));

        #ifdef DEBUG
        printf("\nStarting at offset %d, writing %d bytes to file\n", offset, (end - start) * width * sizeof(unsigned char));
        #endif
        
        offset += (end - start) * width;

        int process;
        MPI_Status status;
        #ifdef DEBUG
        int recvd_tag, recvd_from, recvd_count, error_code;
        #endif

        for (process = 1; process < size; ++process) {
            int count = (end_row(height - 2, size, process) - start_row(height - 2, size, process)) * width;
            MPI_Recv(output_bmp, count, MPI_BYTE, process, PIXELS, MPI_COMM_WORLD, &status);

            #ifdef DEBUG
            printf("From process %d, expecting to receive %d bytes\n", process, count);
            recvd_tag = status.MPI_TAG;
            recvd_from = status.MPI_SOURCE;
            error_code = status.MPI_ERROR;
            MPI_Get_count(&status, MPI_BYTE, &recvd_count);
            printf("\t...received %d bytes\n", recvd_count);
            printf("\t...writing to offset %d\n", offset);
            #endif

            memcpy(final_bmp + offset, output_bmp, count * sizeof(unsigned char));
            offset += count;
        }

        // Stop timer
        struct timespec time_after;
        clock_gettime(CLOCK_REALTIME, &time_after);
        unsigned long time_dif = ((time_after.tv_sec - time_before.tv_sec) * 1000) \
                               + ((time_after.tv_nsec - time_before.tv_nsec) / 1000000);

        // Print results
        printf("Time: %d (ms)\n", time_dif);
        printf("Threshold: %d\n", threshold);

        #ifdef DEBUG
        int i;
        int corrupt = 0;
        for (i = 0; i < get_num_pixel(); ++i) {
            if (final_bmp[i] != BLACK && final_bmp[i] != WHITE) {
                printf("Pixel %d is %d\n", i, final_bmp[i]);
                corrupt = 1;
            }
        }
        if (!corrupt) printf("Image is not corrupt!\n");
        #endif

        // Write image to file
        FILE *out_file = fopen(argv[2], "wb");
        write_bmp_file(out_file, final_bmp);        
    }

    // End MPI
    MPI_Finalize();
}

unsigned int start_row(unsigned int height, unsigned int size, unsigned int rank) {
    int start;
    if (rank < height % size) {
        start = ((height / size) + 1) * rank;
    }
    else {
        start = ((height / size) + 1) * (height % size);
        start += (height / size) * (rank - (height % size));
    }
    return start;
}

unsigned int end_row(unsigned int height, unsigned int size, unsigned int rank) {
    int start, end;
    start = start_row(height, size, rank);
    end = start + (height / size);
    if (rank < height % size) ++end;
    return end;
}

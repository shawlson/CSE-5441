#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "box.h"

#include <time.h>

typedef int bool;
#define false 0
#define true 1


int *read_neighbors(char *, int);

int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Parameters: AFFECT RATE, EPSILON, NUM_THREADS\n");
        return(-1);
    }

    char *end;
    double affect_rate = strtod(argv[1], &end);
    double epsilon = strtod(argv[2], &end);
    int num_threads = atoi(argv[3]);

    char buff[128];

    int grid_boxes, grid_rows, grid_cols;
    fgets(buff, 128, stdin);
    sscanf(buff, "%d%d%d", &grid_boxes, &grid_rows, &grid_cols);
    box_t boxes[grid_boxes]; // Need grid_boxes at most

    int id;
    fgets(buff, 128, stdin);
    sscanf(buff, "%d", &id);

    int num_boxes = 0;
    while (id != -1) {
        
        ++num_boxes;

        // Top left coordinate, width, and, height
        int y, x, height, width;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d%d%d%d", &y, &x, &height, &width);

        // Top neighbors
        int num_top, *top_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_top);
        top_nbrs = read_neighbors(buff, num_top);

        // Bottom neighbors
        int num_bot, *bot_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_bot);
        bot_nbrs = read_neighbors(buff, num_bot);

        // Left neighbors
        int num_left, *left_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_left);
        left_nbrs = read_neighbors(buff, num_left);

        // Right neighbors
        int num_right, *right_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_right);
        right_nbrs = read_neighbors(buff, num_right);

        // Temp
        double temp;
        fgets(buff, 128, stdin);
        sscanf(buff, "%lf", &temp);

        // Instantiate the box
        boxes[id] = (box_t) {
            .x = x, .y = y, .height = height, .width = width,
            .num_top = num_top, .top_nbrs = top_nbrs,
            .num_bot = num_bot, .bot_nbrs = bot_nbrs,
            .num_left = num_left, .left_nbrs = left_nbrs,
            .num_right = num_right, .right_nbrs = right_nbrs,
            .temp = temp
        };
       
        // Get next box id 
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &id);
    }


    double min = INFINITY;
    double max = -INFINITY;
    double updated_temps[num_boxes];
    bool converged = false;
    int iterations = 0;
    unsigned int min_threads = 0xFFFFFFFF;
    unsigned int max_threads = 0x0;

    /* Timing Mechanisms */
    time_t time_before, time_after;
    time_before = time(NULL);

    clock_t clock_before, clock_after;
    clock_before = clock();

    struct timespec gettime_before, gettime_after;
    clock_gettime(CLOCK_REALTIME, &gettime_before);
    /* ****************** */
    #pragma omp parallel num_threads(num_threads)
    {

        // omp_get_num_threads() will be the same across all
        // threads, so there's no need to worry about race conditions
        int thread_count = omp_get_num_threads();
        if (thread_count < min_threads) min_threads = thread_count;
        if (thread_count > max_threads) max_threads = thread_count;
        
        while (!converged) {

            // Calculate updated DSVs
            int t_id = omp_get_thread_num();
            int i;
            for (i = t_id; i < num_boxes; i += omp_get_num_threads()) {
                double adjacent_temp = calc_adjacent_temp(i, boxes);
                double updated_temp = boxes[i].temp - (boxes[i].temp - adjacent_temp) * affect_rate;
                updated_temps[i] = updated_temp;
            }

            #pragma omp barrier
            #pragma omp single
            {
                // Commit updated DSVs and check for new min/max values
                for (i = 0; i < num_boxes; ++i) {
                    boxes[i].temp = updated_temps[i];
                    // I don't think a critical section is needed here,
                    // since only one thread can execute this
                    if (updated_temps[i] < min) min = updated_temps[i];
                    if (updated_temps[i] > max) max = updated_temps[i];
                }
            
                // Check for convergence
                if ((max - min) <= (epsilon * max)) converged = true;
                else {
                    ++iterations;
                    max = -INFINITY;
                    min = INFINITY;
                }
            }
        }
    }

    /* Timing mechanisms */
    time_after = time(NULL);
    clock_after = clock();
    clock_gettime(CLOCK_REALTIME, &gettime_after);


    long time_diff = time_after - time_before;
    double clock_diff = ((double) (clock_after - clock_before)) / CLOCKS_PER_SEC;
    long  gettime_diff_sec = (gettime_after.tv_sec - gettime_before.tv_sec) * 1000;
    double gettime_diff_nsec = ((double) (gettime_after.tv_nsec - gettime_before.tv_nsec) / 1000000);
    double gettime_diff = gettime_diff_sec + gettime_diff_nsec;
    /* ***************** */

    // Results
    printf("Affect rate: %lf\tEpsilon: %lf\n", affect_rate, epsilon);
    printf("Converged in %d iterations\n", iterations);
    printf("Min thread count: %d\n", min_threads);
    printf("Max thread count: %d\n", max_threads);
    printf("Minimum DSV: %lf\n", min);
    printf("Maximum DSV: %lf\n", max);
    printf("Convergence loop time [time()] (s): %ld\n",
            time_diff);
    printf("Convergence loop time [clock()] (s): %lf\n",
            clock_diff);
    printf("Convergence loop time [clock_gettime()] (ms): %lf\n",
           gettime_diff);
}

int *read_neighbors(char *buff, int num_neighbors) {
    
    int *neighbors;
    if (num_neighbors == 0) neighbors = NULL;
    else {
        neighbors = (int *) malloc(num_neighbors * sizeof(int));
    }
    
    const char *token;
    const char *delims = " \t";
    /*
     * First call will consume the number of neighbors,
     * which we already know.
     */
    token = strtok(buff, delims);
    token = strtok(NULL, delims);

    int i;
    for (i = 0; i < num_neighbors; ++i) {
        int neighbor;
        sscanf(token, "%d", &neighbor);
        neighbors[i] = neighbor;
        token = strtok(NULL, delims);
    }
    
    return neighbors;
}


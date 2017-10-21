#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "box.h"

#include <time.h>
#include <pthread.h>


void *calc_dsvs(void *);
int *read_neighbors(char *, int);

typedef int bool;
#define false 0
#define true 1

typedef struct {
    int start;
    int end;
} thread_param_t;

double affect_rate;
box_t *boxes;
double *updated_temps;

int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Parameters: AFFECT RATE, EPSILON, NUM_THREADS\n");
        return(-1);
    }

    // Get user-specified parameters
    char *end;
    affect_rate = strtod(argv[1], &end);
    double epsilon = strtod(argv[2], &end);
    int num_threads = atoi(argv[3]);

    // Read in environment description
    char buff[128];

    int grid_boxes, grid_rows, grid_cols;
    fgets(buff, 128, stdin);
    sscanf(buff, "%d%d%d", &grid_boxes, &grid_rows, &grid_cols);
    boxes = (box_t *) malloc(grid_boxes * sizeof(box_t)); // Need grid_boxes at most

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

    double min, max;
    updated_temps = (double *) malloc(num_boxes * sizeof(double));
    bool converged = false;
    int iterations = 0;

    /* Timing Mechanisms */
    time_t time_before, time_after;
    time_before = time(NULL);

    clock_t clock_before, clock_after;
    clock_before = clock();

    struct timespec gettime_before, gettime_after;
    clock_gettime(CLOCK_REALTIME, &gettime_before);
    /* ****************** */

    // Convergence loop
    while (!converged) {

        printf("Current iteration: %d\n", iterations);

        min = INFINITY;
        max = -INFINITY;

        // Create threads to calculate new DSVs
        pthread_t threads[num_threads];
        int boxes_per_thread = num_boxes / num_threads;
        int remaining = num_boxes % num_threads;

        // If num_threads doesn't evenly divide num_boxes,
        // some threads will have to do more work than others.
        // The (first num_boxes mod num_threads) will calculate
        // one more box than the rest of the threads
        int start = 0;
        int tid;
        for (tid = 0; tid < remaining; ++tid) {
            thread_param_t *params = (thread_param_t *) malloc(sizeof(thread_param_t));
            params->start = start;
            params->end = start + (boxes_per_thread + 1);
            printf("Creating thread that calculates from %d to %d\n", params->start, params->end);
            pthread_create(&threads[tid], NULL, calc_dsvs, (void *) params);
            start += boxes_per_thread + 1;
        }
        for (; tid < num_threads; ++tid) {
            thread_param_t *params = (thread_param_t *) malloc(sizeof(thread_param_t));
            params->start = start;
            params->end = start + boxes_per_thread;
            printf("Creating thread that calculates from %d to %d\n", params->start, params->end);
            pthread_create(&threads[tid], NULL, calc_dsvs, (void *) params);
            start += boxes_per_thread;
        }

        // Wait for threads to exit
        int i;
        for (i = 0; i < num_threads; ++i) {
            pthread_join(threads[i], NULL);
        }

        // Commit DSVs
        for (i = 0; i < num_boxes; ++i) {
            if (updated_temps[i] < min) min = updated_temps[i];
            if (updated_temps[i] > max) max = updated_temps[i];
            boxes[i].temp = updated_temps[i];
        }

        // Check for convergence
        if ((max - min) <= (epsilon * max)) converged = true;
        else ++iterations;
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
    printf("Minimum DSV: %lf\n", min);
    printf("Maximum DSV: %lf\n", max);
    printf("Convergence loop time [time()] (s): %ld\n",
            time_diff);
    printf("Convergence loop time [clock()] (s): %lf\n",
            clock_diff);
    printf("Convergence loop time [clock_gettime()] (ms): %lf\n",
           gettime_diff);
}

void *calc_dsvs(void *arg) {

    thread_param_t *params = (thread_param_t *) arg;

     // Calculate updated DSVs
     int i;
     for (i = params->start; i < params->end; ++i) {
        double adjacent_temp = calc_adjacent_temp(i, boxes);
        double updated_temp = boxes[i].temp - (boxes[i].temp - adjacent_temp) * affect_rate;
        updated_temps[i] = updated_temp;
    }

    free(params);

    return NULL;
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

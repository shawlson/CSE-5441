#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "box.h"

#include <time.h>

typedef int bool;
#define false 0
#define true 1


int *read_neighbors(char *, int);

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "Parameters: AFFECT RATE, EPSILON\n");
        return(-1);
    }

    char *end;
    double affect_rate = strtod(argv[1], &end);
    double epsilon = strtod(argv[2], &end);

    char buff[128];

    int grid_boxes, grid_rows, grid_cols;
    fgets(buff, 128, stdin);
    sscanf(buff, "%d%d%d", &grid_boxes, &grid_rows, &grid_cols);
    box_t boxes[grid_boxes];

    int id;
    fgets(buff, 128, stdin);
    sscanf(buff, "%d", &id);

    int num_boxes = 0;
    while (id != -1) {
        
        ++num_boxes;

        int y, x, height, width;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d%d%d%d", &y, &x, &height, &width);

        int num_top, *top_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_top);
        top_nbrs = read_neighbors(buff, num_top);

        int num_bot, *bot_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_bot);
        bot_nbrs = read_neighbors(buff, num_bot);

        int num_left, *left_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_left);
        left_nbrs = read_neighbors(buff, num_left);

        int num_right, *right_nbrs;
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &num_right);
        right_nbrs = read_neighbors(buff, num_right);

        double temp;
        fgets(buff, 128, stdin);
        sscanf(buff, "%lf", &temp);

        boxes[id] = (box_t) {
            .x = x, .y = y, .height = height, .width = width,
            .num_top = num_top, .top_nbrs = top_nbrs,
            .num_bot = num_bot, .bot_nbrs = bot_nbrs,
            .num_left = num_left, .left_nbrs = left_nbrs,
            .num_right = num_right, .right_nbrs = right_nbrs,
            .temp = temp
        };
        
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &id);
    }


    double min;
    double max;
    double updated_temps[num_boxes];
    bool converged = false;
    int iterations = 0;

    time_t seconds_before, seconds_after;
    seconds_before = time(NULL);

    clock_t clock_timer;
    clock_timer = clock();
    while (!converged) {

        int i;
        max = -INFINITY;
        min = INFINITY;
        for (i = 0; i < num_boxes; ++i) {
            double adjacent_temp = calc_adjacent_temp(i, boxes);
            double updated_temp = boxes[i].temp - (boxes[i].temp - adjacent_temp) * affect_rate;
            updated_temps[i] = updated_temp;
            if (updated_temp > max) max = updated_temp;
            if (updated_temp < min) min = updated_temp;
        }

        for (i = 0; i < num_boxes; ++i) {
            boxes[i].temp = updated_temps[i];
        }

        if ((max - min) <= (epsilon * max)) converged = true;
        else ++iterations;
    }

    seconds_after = time(NULL);
    clock_timer = clock() - clock_timer;

    printf("Affect rate: %lf\tEpsilon: %lf\n", affect_rate, epsilon);
    printf("Converged in %d iterations\n", iterations);
    printf("Minimum DSV: %lf\n", min);
    printf("Maximum DSV: %lf\n", max);
    printf("Convergence loop time [time()]: %ld\n",
            seconds_after - seconds_before);
    printf("Convergence loop time [clock()]: %lf\n",
            ((double) clock_timer) / CLOCKS_PER_SEC);
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

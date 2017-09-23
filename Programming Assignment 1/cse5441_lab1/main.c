#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "box.h"

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
            .num_top_neighbors = num_top, .top_neighbors = top_nbrs,
            .num_bottom_neighbors = num_bot, .bottom_neighbors = bot_nbrs,
            .num_left_neighbors = num_left, .left_neighbors = left_nbrs,
            .num_right_neighbors = num_right, .right_neighbors = right_nbrs,
            .temp = temp, .adjacent_temp = NAN
        };
        
        fgets(buff, 128, stdin);
        sscanf(buff, "%d", &id);
    }

    int iterations = 0;
    bool converged = false;
    while (!converged) {
        int i;
        for (i = 0; i < num_boxes; ++i) {
            calc_adjacent_temp(i, boxes);
        }

        double max = -INFINITY;
        double min = INFINITY;
        for (i = 0; i < num_boxes; ++i) {
            double new_temp = boxes[i].temp - (boxes[i].temp - boxes[i].adjacent_temp) * affect_rate;
            if (new_temp > max) max = new_temp;
            else if (new_temp < min) min = new_temp;
            boxes[i].temp = new_temp;
            boxes[i].adjacent_temp = NAN;
        }

        double delta = max - min;
        if (delta <= max * epsilon) converged = true;
        else ++iterations;
    }

    printf("Converged in %d iterations\n", iterations);
}

int *read_neighbors(char *buff, int num_neighbors) {
    
    int *neighbors;
    if (num_neighbors == 0) neighbors = NULL;
    else {
        neighbors = (int *) malloc(num_neighbors * sizeof(int));
    }
    
    int i;
    for (int i = 0; i < num_neighbors; ++i) {
        int neighbor;
        int offset = 2*i + 2;
        sscanf(buff + offset, "%d", &neighbor);
        neighbors[i] = neighbor;
    }

    return neighbors;
}

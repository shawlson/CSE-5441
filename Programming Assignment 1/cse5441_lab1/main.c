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

    int grid_boxes, grid_rows, grid_cols;
    scanf("%d%d%d", &grid_boxes, &grid_rows, &grid_cols);
    box_t boxes[grid_boxes];

    int id;
    int num_boxes = 0;
    scanf("%d", &id);
    while (id != -1) {
        
        ++num_boxes;

        int y, x, height, width;
        scanf("%d%d%d%d", &y, &x, &height, &width);
        
        char line[128];
        fgets(line, 128, stdin);
        int num_top_neighbors;
        sscanf(line, "%d", &num_top_neighbors);
        int *top_neighbors = read_neighbors(line, num_top_neighbors);

        memset(line, '\0', 128);
        fgets(line, 128, stdin);
        int num_bottom_neighbors;
        sscanf(line, "%d", &num_bottom_neighbors);
        int *bottom_neighbors = read_neighbors(line, num_bottom_neighbors);

        memset(line, '\0', 128);
        fgets(line, 128, stdin);
        int num_left_neighbors;
        sscanf(line, "%d", &num_left_neighbors);
        int *left_neighbors = read_neighbors(line, num_left_neighbors);

        memset(line, '\0', 128);
        fgets(line, 128, stdin);
        int num_right_neighbors;
        sscanf(line, "%d", &num_right_neighbors);
        int *right_neighbors = read_neighbors(line, num_right_neighbors);

        double temp;
        scanf("%lf", &temp);

        boxes[id] = (box_t) {
            .x = x, .y = y, .height = height, .width = width,
            .num_top_neighbors = num_top_neighbors, .top_neighbors = top_neighbors,
            .num_bottom_neighbors = num_bottom_neighbors, .bottom_neighbors = bottom_neighbors,
            .num_left_neighbors = num_left_neighbors, .left_neighbors = left_neighbors,
            .num_right_neighbors = num_right_neighbors, .right_neighbors = right_neighbors,
            .temp = temp, .adjacent_temp = NAN
        };
        scanf("%d", &id);
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

    printf("Converged in %d iterations", iterations);
}

int *read_neighbors(char *line, int num_neighbors) {
    int *neighbors = (int *) malloc(num_neighbors * sizeof(int));

    int i;
    for (int i = 0; i < num_neighbors; ++i) {
        int neighbor;
        int offset = 2i + 2;
        sscanf(line + offset, "%d", &neighbor);
        neighbors[i] = neighbor;
    }

    return neighbors;
}

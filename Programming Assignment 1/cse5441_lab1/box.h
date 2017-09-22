#ifndef BOX_HEADER
#define BOX_HEADER

typedef struct {
    int x;
    int y;
    int height;
    int width;
    int num_top_neighbors;
    int *top_neighbors;
    int num_bottom_neighbors;
    int *bottom_neighbors;
    int num_left_neighbors;
    int *left_neighbors;
    int num_right_neighbors;
    int *right_neighbors;
    double temp;
    double adjacent_temp;
} box_t;

double calc_adjacent_temp(int id, box_t *boxes);

#endif
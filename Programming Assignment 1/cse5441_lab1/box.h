#ifndef BOX_HEADER
#define BOX_HEADER

typedef struct {
    int x;
    int y;
    int height;
    int width;
    int num_top;
    int *top_nbrs;
    int num_bot;
    int *bot_nbrs;
    int num_left;
    int *left_nbrs;
    int num_right;
    int *right_nbrs;
    double temp;
} box_t;

double calc_adjacent_temp(int id, box_t *boxes);

#endif
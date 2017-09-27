#include "box.h"

enum overlap_type {HORIZONTAL, VERTICAL};
static int overlap(box_t *one, box_t *two, enum overlap_type type);
static inline int perimeter(box_t *box);


double
calc_adjacent_temp(int id, box_t *boxes) {
    
    box_t *box = &(boxes[id]);
    double sum = 0.0;
    int i;

    // Sum temp(neighbor) * overlap(neighbor) over all neighbors
    if (!box->num_top) sum += box->temp * box->width;
    else {
        for (i = 0; i < box->num_top; ++i) {
            int neighbor_id = box->top_nbrs[i];
            box_t *neighbor = &(boxes[neighbor_id]);
            sum += neighbor->temp * overlap(box, neighbor, HORIZONTAL);
        }
    }

    if (!box->num_bot) sum += box->temp * box->width;
    else {
        for (i = 0; i < box->num_bot; ++i) {
            int neighbor_id = box->bot_nbrs[i];
            box_t *neighbor = &(boxes[neighbor_id]);
            sum += neighbor->temp * overlap(box, neighbor, HORIZONTAL);
        }
    }

    if (!box->num_left) sum += box->temp * box->height;
    else {
        for (i = 0; i < box->num_left; ++i) {
            int neighbor_id = box->left_nbrs[i];
            box_t *neighbor = &(boxes[neighbor_id]);
            sum += neighbor->temp * overlap(box, neighbor, VERTICAL);
        }
    }

    if (!box->num_right) sum += box->temp * box->height;
    else {
        for (i = 0; i < box->num_right; ++i) {
            int neighbor_id = box->right_nbrs[i];
            box_t *neighbor = &(boxes[neighbor_id]);
            sum += neighbor->temp * overlap(box, neighbor, VERTICAL);
        }
    }

    double adjacent_temp = sum / perimeter(box);
    return adjacent_temp;
}

static int
overlap(box_t *box, box_t *neighbor, enum overlap_type type) {
    int box_min, box_max, box_dist;
    int nbr_min, nbr_max, nbr_dist;

    switch (type) {
        case HORIZONTAL:
            box_min = box->x;
            box_max = box->x + box->width;
            box_dist = box->width;
            nbr_min = neighbor->x;
            nbr_max = neighbor->x + neighbor->width;
            nbr_dist = neighbor->width;
            break;
        case VERTICAL:
            box_min = box->y;
            box_max = box->y + box->height;
            box_dist = box->height;
            nbr_min = neighbor->y;
            nbr_max = neighbor->y + neighbor->height;
            nbr_dist = neighbor->height;
            break;
    }

    if (nbr_min <= box_min && nbr_max <= box_max) {;
        return nbr_max - box_min;
    } else if (nbr_min >= box_min && nbr_max > box_max) {
        return box_max - nbr_min;
    } else if (nbr_min < box_min && nbr_max > box_max) {
        return box_dist;
    } else {
        return nbr_dist;
    }
}

static inline int
perimeter(box_t *box) {
    return (box->width << 1) + (box->height << 1);
}

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#ifdef BLACK
#undef BLACK
#endif
#define BLACK 0

#ifdef WHITE
#undef WHITE
#endif
#define WHITE 255

typedef struct
{
    local_t *edge;
    flag_t  *done;
}boundary_t;

extern boundary_t serch_left_black_line(pixel_t *image,local_t start,local_t end,local_t median);
extern boundary_t serch_right_black_line(pixel_t *image,local_t start,local_t end,local_t median);
extern boundary_t serch_left_black_line_f(pixel_t *image,local_t start,local_t end,local_t median);
extern boundary_t serch_right_black_line_f(pixel_t *image,local_t start,local_t end,local_t median);
extern void five_point_smooth(local_t start,local_t end,local_t *mids);
extern double least_square(const local_t start,const local_t end,const local_t map_start,const local_t map_end,local_t *mids);
extern void get_frame(pixel_t *dst,pixel_t *src);
extern flag_t is_start(local_t start,local_t end,boundary_t left_edge,boundary_t right_edge);

#endif
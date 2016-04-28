#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

typedef struct
{
    uint8 *edge;
    uint8 *done;
}boundary;

extern boundary serch_left_black_line(uint8 *image,int32 start,int32 end,int32 median);
extern boundary serch_right_black_line(uint8 *image,int32 start,int32 end,int32 median);
extern void five_point_smooth(int32 start,int32 end,uint8 *mids);
extern double least_square(const int32 start,const int32 end,const int32 map_start,const int32 map_end,uint8 *mids);

#endif
#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

extern uint8* serch_left_black_line(uint8 *image,int32 start,int32 end,int32 median);
extern uint8* serch_right_black_line(uint8 *image,int32 start,int32 end,int32 median);
extern void five_point_smooth(int32 start,int32 end,uint8 *mids);
extern double least_square(int start,int end,uint8 *mids);

#endif
#include "include.h"
#include "algorithm.h"

#define BLACK 0
#define WHITE 255

uint8* serch_left_black_line(uint8 *image,int32 start,int32 end,int32 median)
{
    register int32 x,y;
    *uint8 left_edge[CAMERA_H];
    for(y=start;y<=end;y++)
    {
        for(x=median;x>0;x--)
        {
            if(image[y*CAMERA_W+x]==BLACK)
            {
                if(image[y*CAMERA_W+x-1]==BLACK &&
                   image[y*CAMERA_W+x-2]==BLACK &&
                   image[y*CAMERA_W+x-3]==BLACK)
                {
                    ;
                }
            }
        }
    }
}
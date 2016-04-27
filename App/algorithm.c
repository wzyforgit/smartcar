#include "include.h"
#include "algorithm.h"
#include <string.h>

#undef BLACK
#define BLACK 0

#undef WHITE
#define WHITE 255

uint8* serch_left_black_line(uint8 *image,int32 start,int32 end,int32 median)
{
    static uint8 left_edge[CAMERA_H]={0};
    register int32 x,y;
    for(y=start;y<=end;y++)
    {
        for(x=median;x>2;x--)
        {
            if(image[y*CAMERA_W+x]==BLACK)
            {
                if(image[y*CAMERA_W+x-1]==BLACK &&
                   image[y*CAMERA_W+x-2]==BLACK &&
                   image[y*CAMERA_W+x-3]==BLACK)
                {
                    left_edge[y]=x;
                    break;
                }
            }
        }
    }
    return left_edge;
}

uint8* serch_right_black_line(uint8 *image,int32 start,int32 end,int32 median)
{
    static uint8 right_edge[CAMERA_H]={0};
    register int32 x,y;
    for(y=start;y<=end;y++)
    {
        for(x=median;x<CAMERA_W-3;x++)
        {
            if(image[y*CAMERA_W+x]==BLACK)
            {
                if(image[y*CAMERA_W+x+1]==BLACK &&
                   image[y*CAMERA_W+x+2]==BLACK &&
                   image[y*CAMERA_W+x+3]==BLACK)
                {
                    right_edge[y]=x;
                    break;
                }
            }
        }
    }
    return right_edge;
}

void five_point_smooth(int32 start,int32 end,uint8 *mids)
{
    register int32 count;
    for(count=start;count<=end-4;++count)
    {
        mids[count]=(int32)((mids[count]+mids[count+1]+mids[count+2]+mids[count+3]+mids[count+4])/5);
    }
}
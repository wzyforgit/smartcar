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
        mids[count]=(mids[count]+mids[count+1]+mids[count+2]+mids[count+3]+mids[count+4])/5;
    }
}

double least_square(int end,int start,uint8 *mids)//start为最低端行数，与其他函数相反
{
    double rowba;
    double midba;
    int32 midsum;
    int32 row;
    for(row=start,midsum=0;row>=end;row--)//计算中点和
    {
  	    midsum+=mids[row];
    }
    midba=(double)midsum/(start-end+1);//计算mid均值
    rowba=(double)(end+start)/2.0;//计算row均值
    
    double sumx1;
    double sumx2;
    for(row=start,sumx1=0,sumx2=0;row>=end;row--)//计算斜率的分子(sumx1)/分母(sumx2)
    {
	    double temp;
	    temp=row-rowba;
	    sumx1+=temp*(mids[row]-midba);
	    sumx2+=pow(temp,2);
    }
    
    double k,b;
    k=sumx1/sumx2;//计算斜率
    b=midba-k*rowba;//计算截距
    for(row=start;row>=end;row--)//计算mids
    {
	    mids[row]=(int16)(k*row+b+0.5);
    }
    return k;
}
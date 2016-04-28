#include "include.h"
#include "algorithm.h"
#include <string.h>

#undef BLACK
#define BLACK 0

#undef WHITE
#define WHITE 255

boundary serch_left_black_line(uint8 *image,int32 start,int32 end,int32 median)
{
    static uint8 left_edge[CAMERA_H]={0};
    static uint8 left_edge_flag[CAMERA_H]={0};
    static boundary left_edges={left_edge,left_edge_flag};
    register int32 x,y;
    memset(left_edge_flag,0,sizeof(uint8)*CAMERA_H);
    
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
                    left_edge_flag[y]=1;
                    break;
                }
                else
                {
                    x--;
                    continue;
                }
            }
        }
    }
    return left_edges;
}

boundary serch_right_black_line(uint8 *image,int32 start,int32 end,int32 median)
{
    static uint8 right_edge[CAMERA_H]={0};
    static uint8 right_edge_flag[CAMERA_H]={0};
    static boundary right_edges={right_edge,right_edge_flag};
    register int32 x,y;
    memset(right_edge_flag,0,sizeof(uint8)*CAMERA_H);
    
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
                    right_edge_flag[y]=1;
                    break;
                }
                else
                {
                    x++;
                    continue;
                }
            }
        }
    }
    return right_edges;
}

void five_point_smooth(int32 start,int32 end,uint8 *mids)
{
    register int32 count;
    for(count=start;count<=end-4;++count)
    {
        mids[count]=(mids[count]+mids[count+1]+mids[count+2]+mids[count+3]+mids[count+4])/5;
    }
}

double least_square(const int32 end,const int32 start,const int32 map_start,const int32 map_end,uint8 *mids)//start为最低端行数，与其他函数相反
{
    double rowba;
    double midba;
    int32 midsum;
    register int32 row;
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
    for(row=map_start;row<=map_end;row++)//映射到对应区间
    {
	    mids[row]=(uint8)(k*row+b+0.5);
    }
    return k;
}
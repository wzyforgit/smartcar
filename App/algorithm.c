#include "include.h"
#include "algorithm.h"
#include <string.h>

boundary_t serch_left_black_line(pixel_t *image,local_t start,local_t end,local_t median)
{
    static local_t left_edge[CAMERA_H]={0};
    static flag_t left_edge_flag[CAMERA_H]={0};
    static boundary_t left_edges={left_edge,left_edge_flag};
    register count_t x,y;
    memset(left_edge_flag,0,sizeof(flag_t)*CAMERA_H);
    
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
                    if(image[y*CAMERA_W+x+1]==WHITE &&
                       image[y*CAMERA_W+x+2]==WHITE &&
                       image[y*CAMERA_W+x+3]==WHITE)
                    {
                        left_edge[y]=x;
                        left_edge_flag[y]=1;
                        break;
                    }
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

boundary_t serch_right_black_line(pixel_t *image,local_t start,local_t end,local_t median)
{
    static local_t right_edge[CAMERA_H]={0};
    static flag_t right_edge_flag[CAMERA_H]={0};
    static boundary_t right_edges={right_edge,right_edge_flag};
    register count_t x,y;
    memset(right_edge_flag,0,sizeof(flag_t)*CAMERA_H);
    
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
                    if(image[y*CAMERA_W+x-1]==WHITE &&
                       image[y*CAMERA_W+x-2]==WHITE &&
                       image[y*CAMERA_W+x-3]==WHITE)
                    {
                        right_edge[y]=x;
                        right_edge_flag[y]=1;
                        break;
                    }
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

void five_point_smooth(local_t start,local_t end,local_t *mids)
{
    register count_t count;
    for(count=start;count<=end-4;++count)
    {
        mids[count]=(mids[count]+mids[count+1]+mids[count+2]+mids[count+3]+mids[count+4])/5;
    }
}

double least_square(const local_t end,const local_t start,const local_t map_start,const local_t map_end,local_t *mids)//start为最低端行数，与其他函数相反
{
    double rowba;
    double midba;
    int32 midsum;
    register count_t row;
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

/*起跑线识别*/
#ifdef offset
#undef offset
#endif
#define offset 15
/*由于摄像头的二值化，偏移量应该在能看见起跑线的行数内*/
flag_t is_start(local_t start,local_t end,boundary_t left_edge,boundary_t right_edge)
{
    register count_t row,count;
    local_t edge_diff[offset+1];
    for(row=end-offset,count=0;row<=end;row++,count++)//空间换时间
    {
        edge_diff[count]=right_edge.edge[row]-left_edge.edge[row];
    }
    
    flag_t before=0;
    for(count=1;count<=offset-3;count++)
    {
        if(abs(edge_diff[count]-edge_diff[count-1])>10)
        {
            if(abs(edge_diff[count]-edge_diff[count+1])<3 &&
               abs(edge_diff[count]-edge_diff[count+2])<5 &&
               abs(edge_diff[count]-edge_diff[count+3])<7)
            {
                before=1;
                break;
            }
        }
    }
    
    if(before==0)
    {
        return 0;
    }
    
    local_t back_end=count+4;
    for(count=offset;count>=back_end;count--)
    {
        if(abs(edge_diff[count]-edge_diff[count+1])>10)
        {
            if(abs(edge_diff[count]-edge_diff[count-1])<3 &&
               abs(edge_diff[count]-edge_diff[count-2])<5 &&
               abs(edge_diff[count]-edge_diff[count-3])<7)
            {
                return 1;
            }
        }
    }
    
    return 0;
}
#include "include.h"
#include "algorithm.h"
#include <string.h>

static local_t serch_left_black_line(pixel_t *image,local_t line,local_t median)
{
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=median;x>1;x--)
    {
        if(image[x]==BLACK)
        {
            if(image[x-1]==BLACK &&
               image[x-2]==BLACK)
            {
                if(image[x+1]==WHITE &&
                   image[x+2]==WHITE)
                {
                    return x;
                }
            }
            else
            {
                x--;
                continue;
            }
        }
    }
    return CAMERA_W;
}

static local_t serch_right_black_line(pixel_t *image,local_t line,local_t median)
{
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=median;x<CAMERA_W-2;x++)
    {
        if(image[x]==BLACK)
        {
            if(image[x+1]==BLACK &&
               image[x+2]==BLACK)
            {
                if(image[x-1]==WHITE &&
                   image[x-2]==WHITE)
                {
                    return x;
                }
            }
            else
            {
                x++;
                continue;
            }
        }
    }
    return CAMERA_W;
}

boundary_t serch_left_edge(pixel_t *image,local_t start,local_t end,local_t median)
{
    static local_t left_edge[CAMERA_H]={0};
    static flag_t left_edge_flag[CAMERA_H]={0};
    static boundary_t left_edges={left_edge,left_edge_flag};
    register count_t y;
    memset(left_edge_flag,0,sizeof(flag_t)*CAMERA_H);
    
    for(y=start;y<=end;y++)
    {
        local_t temp=serch_left_black_line(image,y,median);
        if(temp>=CAMERA_W)
        {
            continue;
        }
        else
        {
            left_edge[y]=temp;
            left_edge_flag[y]=1;
        }
    }
    return left_edges;
}

boundary_t serch_right_edge(pixel_t *image,local_t start,local_t end,local_t median)
{
    static local_t right_edge[CAMERA_H]={0};
    static flag_t right_edge_flag[CAMERA_H]={0};
    static boundary_t right_edges={right_edge,right_edge_flag};
    register count_t y;
    memset(right_edge_flag,0,sizeof(flag_t)*CAMERA_H);
    
    for(y=start;y<=end;y++)
    {
        local_t temp=serch_right_black_line(image,y,median);
        if(temp>=CAMERA_W)
        {
            continue;
        }
        else
        {
            right_edge[y]=temp;
            right_edge_flag[y]=1;
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
#ifdef end_offset
#undef end_offset
#endif
#define end_offset 10
/*由于摄像头的二值化，偏移量应该在能看见起跑线的行数内*/

static flag_t serch_left_black_block(pixel_t *image,local_t line,local_t median)
{
    local_t status;
    status=serch_left_black_line(image,line,median);
    if(status>=CAMERA_W||status<median-25)//左黑方块右侧丢失
    {
        return 0;
    }
    
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=status;x>status-20&&x>1;x--)//寻找左黑块左侧
    {
        if(image[x]==WHITE&&image[x-1]==WHITE&&image[x-2]==WHITE)
        {
            status=x;
            break;
        }
    }
    if(x==status-20||x<=1)//左黑方块左侧丢失
    {
        return 0;
    }
    
    for(x=status;x>1;x--)//寻找左边界
    {
        if(image[x]==BLACK&&image[x-1]==BLACK&&image[x-2]==BLACK)
        {
            return 1;
        }
    }
    return 0;
}

static flag_t serch_right_black_block(pixel_t *image,local_t line,local_t median)
{
    local_t status;
    status=serch_right_black_line(image,line,median);
    if(status>=CAMERA_W||status>median+25)//右黑方块左侧丢失
    {
        return 0;
    }
    
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=status;x<status+20&&x<=CAMERA_W-1;x++)//寻找右黑块右侧
    {
        if(image[x]==WHITE&&image[x+1]==WHITE&&image[x+2]==WHITE)
        {
            status=x;
            break;
        }
    }
    if(x==status+20||x>=CAMERA_W-1)//右黑方块右侧丢失
    {
        return 0;
    }
    
    for(x=status;x<CAMERA_W-2;x++)//寻找右边界
    {
        if(image[x]==BLACK&&image[x+1]==BLACK&&image[x+2]==BLACK)
        {
            return 1;
        }
    }
    return 0;
}

flag_t is_start(pixel_t *image,local_t end)
{
    count_t y;
    for(y=end;y>end-end_offset;y--)
    {
        if(serch_left_black_block(image,y,CAMERA_W/2) && serch_right_black_block(image,y,CAMERA_W/2))
        {
            return 1;
        }
    }
    return 0;
}
#undef end_offset

/*障碍物*/
static flag_t is_left_obstacle(pixel_t *image,local_t start,local_t end)
{
    count_t y;
    count_t lines=0;
    for(y=end;y>=start;y--)
    {
        if(serch_left_black_block(image,y,CAMERA_W/2+5))
        {
            lines++;
        }
    }
    if(lines>=3)
    {
        led(LED2,LED_ON);
        led(LED3,LED_OFF);
        return 1;
    }
    else
    {
        return 0;
    }
}

static flag_t is_right_obstacle(pixel_t *image,local_t start,local_t end)
{
    count_t y;
    count_t lines=0;
    for(y=end;y>=start;y--)
    {
        if(serch_right_black_block(image,y,CAMERA_W/2-5))
        {
            lines++;
        }
    }
    if(lines>=3)
    {
        led(LED2,LED_OFF);
        led(LED3,LED_ON);
        return 1;
    }
    else
    {
        return 0;
    }
}

count_t is_obstacle(pixel_t *image,local_t start,local_t end,local_t *mids)
{
    static count_t obstacle_type=NO_OBSTACLE;
    if(is_left_obstacle(image,start,end))
    {
        obstacle_type=LEFT_OBSTACLE;
    }
    else if(is_right_obstacle(image,start,end))
    {
        obstacle_type=RIGHT_OBSTACLE;
    }
    else
    {
        led(LED2,LED_OFF);
        led(LED3,LED_OFF);
        obstacle_type=NO_OBSTACLE;
    }
    return obstacle_type;
}
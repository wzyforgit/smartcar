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

double least_square(const local_t end,const local_t start,const local_t map_start,const local_t map_end,local_t *mids)//startΪ��Ͷ������������������෴
{
    double rowba;
    double midba;
    int32 midsum;
    register count_t row;
    for(row=start,midsum=0;row>=end;row--)//�����е��
    {
  	    midsum+=mids[row];
    }
    midba=(double)midsum/(start-end+1);//����mid��ֵ
    rowba=(double)(end+start)/2.0;//����row��ֵ
    
    double sumx1;
    double sumx2;
    for(row=start,sumx1=0,sumx2=0;row>=end;row--)//����б�ʵķ���(sumx1)/��ĸ(sumx2)
    {
	    double temp;
	    temp=row-rowba;
	    sumx1+=temp*(mids[row]-midba);
	    sumx2+=pow(temp,2);
    }
    
    double k,b;
    k=sumx1/sumx2;//����б��
    b=midba-k*rowba;//����ؾ�
    for(row=map_start;row<=map_end;row++)//ӳ�䵽��Ӧ����
    {
	    mids[row]=(uint8)(k*row+b+0.5);
    }
    return k;
}

/*������ʶ��*/
#ifdef end_offset
#undef end_offset
#endif
#define end_offset 10
/*��������ͷ�Ķ�ֵ����ƫ����Ӧ�����ܿ��������ߵ�������*/

static flag_t serch_left_black_block(pixel_t *image,local_t line,local_t median)
{
    local_t status;
    status=serch_left_black_line(image,line,median);
    if(status>=CAMERA_W||status<median-25)//��ڷ����Ҳඪʧ
    {
        return 0;
    }
    
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=status;x>status-20&&x>1;x--)//Ѱ����ڿ����
    {
        if(image[x]==WHITE&&image[x-1]==WHITE&&image[x-2]==WHITE)
        {
            status=x;
            break;
        }
    }
    if(x==status-20||x<=1)//��ڷ�����ඪʧ
    {
        return 0;
    }
    
    for(x=status;x>1;x--)//Ѱ����߽�
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
    if(status>=CAMERA_W||status>median+25)//�Һڷ�����ඪʧ
    {
        return 0;
    }
    
    image=image+line*CAMERA_W;
    register count_t x;
    for(x=status;x<status+20&&x<=CAMERA_W-1;x++)//Ѱ���Һڿ��Ҳ�
    {
        if(image[x]==WHITE&&image[x+1]==WHITE&&image[x+2]==WHITE)
        {
            status=x;
            break;
        }
    }
    if(x==status+20||x>=CAMERA_W-1)//�Һڷ����Ҳඪʧ
    {
        return 0;
    }
    
    for(x=status;x<CAMERA_W-2;x++)//Ѱ���ұ߽�
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

/*�ϰ���*/
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
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

boundary_t serch_left_black_line_f(pixel_t *image,local_t start,local_t end,local_t median)
{
    static local_t left_edge[CAMERA_H]={0};
    static flag_t left_edge_flag[CAMERA_H]={0};
    static boundary_t left_edges={left_edge,left_edge_flag};
    register count_t x,y;
    memset(left_edge_flag,0,sizeof(flag_t)*CAMERA_H);
    
    for(y=start;y<=end;y++)
    {
        for(x=median;x>=0;x--)
        {
            if(image[y*CAMERA_W+x]==WHITE)
            {
                if(image[y*CAMERA_W+x+1]==BLACK&&
                   image[y*CAMERA_W+x+2]==BLACK&&
                   image[y*CAMERA_W+x+3]==BLACK)
                {
                    left_edge[y]=x;
                    left_edge_flag[y]=1;
                    break;
                }
            }
        }
    }
    return left_edges;
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

/*�߽���ȡ(ȡ��Χ�ĸ���)*/
void get_frame(pixel_t *dst,pixel_t *src)
{
	register count_t x, y;
	for (y = 1; y < CAMERA_H - 1; y++)
	{
		for (x = 1; x < CAMERA_W - 1; x++)
		{
			dst[y*CAMERA_W+x] =
				src[y*CAMERA_W+x] ^
				(src[y*CAMERA_W+x] |
				src[(y+1)*CAMERA_W+x] | 
				src[y*CAMERA_W+x+1] |
				src[y*CAMERA_W+x-1] |
				src[(y-1)*CAMERA_W+x]);
		}
	}
}
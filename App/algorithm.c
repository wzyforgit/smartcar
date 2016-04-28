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

double least_square(int end,int start,uint8 *mids)//startΪ��Ͷ������������������෴
{
    double rowba;
    double midba;
    int32 midsum;
    int32 row;
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
    for(row=start;row>=end;row--)//����mids
    {
	    mids[row]=(int16)(k*row+b+0.5);
    }
    return k;
}
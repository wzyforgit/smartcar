#include "include.h"
#include "algorithm.h"
#include <string.h>

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
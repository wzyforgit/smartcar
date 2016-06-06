#include "include.h"
#include "algorithm.h"
#include <string.h>

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
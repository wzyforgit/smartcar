#include "include.h"
#include "algorithm.h"
#include <string.h>

/*摄像头硬件*/

static pixel_t imgbuff[CAMERA_SIZE];
static pixel_t img[CAMERA_H*CAMERA_W];

static void PORTA_IRQHandler(void)
{
    uint8  n;    //引脚号
    uint32 flag; //不要改为使用flag_t

    while(!PORTA_ISFR);
    flag = PORTA_ISFR;
    PORTA_ISFR  = ~0;             //清中断标志位

    n = 29;                       //场中断
    if(flag & (1 << n))           //PTA29触发中断
    {
        camera_vsync();
    }
#if ( CAMERA_USE_HREF == 1 )      //使用行中断
    n = 28;
    if(flag & (1 << n))           //PTA28触发中断
    {
        camera_href();
    }
#endif
}

static void DMA0_IRQHandler(void)
{
    camera_dma();
}

/*识别*/

#define high_speed 0x01
#define low_speed  0x02
static int32 mode=0;

static void read_DIPswitch(void)
{
    port_init(PTC16, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC16)) &= ~(1 << PTn(PTC16));
    
    port_init(PTC17, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC17)) &= ~(1 << PTn(PTC17));
    
    port_init(PTC18, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC18)) &= ~(1 << PTn(PTC18));
    
    port_init(PTC19, ALT1 | PULLUP );
    GPIO_PDDR_REG(GPIOX_BASE(PTC19)) &= ~(1 << PTn(PTC19));
    
    mode=GPIO_GET_NBIT(3,PTC16);
    LCD_printf(0,61,"%d",mode);
}

static void display_start_end(void);
void discern_init(void)
{
    
    camera_init(imgbuff);
    set_vector_handler(PORTA_VECTORn , PORTA_IRQHandler);   //摄像头场中断
    set_vector_handler(DMA0_VECTORn , DMA0_IRQHandler);     //摄像头DMA中断
    NVIC_SetPriority(PORTA_IRQn,0);
    NVIC_SetPriority(DMA0_IRQn,1);
    read_DIPswitch();
    display_start_end();
}

#define start_line 20
#define end_line (CAMERA_H-5)
#define base_line 38

static local_t* get_midline(pixel_t *image)
{
    static local_t mids[CAMERA_H];
    
    boundary_t left_edge,right_edge;
    left_edge=serch_left_black_line(image,start_line,end_line,base_line+5);
    right_edge=serch_right_black_line(image,start_line,end_line,base_line-5);
    
    register count_t count;
    
    /*扫描前五行*/
#if(start_line<=15)
    if(left_edge.done[start_line]+left_edge.done[start_line+1]+left_edge.done[start_line+2]+left_edge.done[start_line+3]+left_edge.done[start_line+4]<=2 &&
       right_edge.done[start_line]+right_edge.done[start_line+1]+right_edge.done[start_line+2]+right_edge.done[start_line+3]+right_edge.done[start_line+4]<=2)
    {
        return NULL;
    }
#endif
    
    /*补出中线*/
    for(count=start_line;count<=end_line;count++)
    {
        flag_t get_left=left_edge.done[count];
        flag_t get_right=right_edge.done[count];
        if(get_left&&get_right)//都找到
        {
            mids[count]=(local_t)(left_edge.edge[count]+right_edge.edge[count])/2;
        }
        else if(!get_left&&get_right)//左丢失
        {
            mids[count]=(local_t)(right_edge.edge[count]-base_line/2);
            if(mids[count]>=80)
            {
                mids[count]=0;
            }
        }
        else if(get_left&&!get_right)//右丢失
        {
            mids[count]=(local_t)(left_edge.edge[count]+base_line/2);
            if(mids[count]>=80)
            {
                mids[count]=79;
            }
        }
        else//全丢失
        {
            ;
        }
    }
    return mids;
}

static void display_start_end(void)
{
    Site_t lines[CAMERA_W+20];
    register count_t count;
    for(count=0;count<CAMERA_W+20;count++)
    {
        lines[count].x=count;
        lines[count].y=start_line-1;
    }
    LCD_points(lines,CAMERA_W+20,BLUE);
    for(count=0;count<CAMERA_W+20;count++)
    {
        lines[count].x=count;
        lines[count].y=end_line+1;
    }
    LCD_points(lines,CAMERA_W+20,BLUE);
}

static void display_lines(local_t start,local_t end,local_t *lines)
{
    Site_t line_site[CAMERA_H];
    register count_t count;
    for(count=start;count<=end;count++)
    {
        line_site[count].x=lines[count];
        line_site[count].y=count;
    }
    LCD_points(line_site+start,count,RED);
}

static speed_t speed_choose(traffic choose)
{
    speed_t speed=0;
    switch(mode)
    {
        case high_speed:
        switch(choose)
        {
            case curve   :speed=200;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        case low_speed:
        switch(choose)
        {
            case curve   :speed=200;break;
            case beeline :speed=100;break;
            case crossing:speed=100;break;
            default      :speed=100;break;
        }
        break;
        
        default:speed=300;
    }
    return speed;
}

local_t get_average_mid(local_t start,local_t end,local_t *mids)
{
    static const double weight[CAMERA_H]={
        2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.6,
        2.6 ,2.6 ,2.6 ,2.6 ,2.6 ,2.56,2.52,2.48,2.44,2.4,      
        2.36,2.32,2.3 ,2.26,2.22,2.18,2.14,2.1,2.06,2.02,
        1.98,1.94,1.9 ,1.86,1.82,1.78,1.74,1.7,1.66,1.62,
        1.58,1.54,1.5 ,1.47,1.42,1.38,1.34,1.3,1.26,1.22,
        1.18,1.14,1.10,1.07,1.03,1.0 ,1.0 ,1.0,1.0 ,1.0
    };
    
    register count_t count;
    double mid_result=0;
    double div=0;
    for(count=start;count<=end;count++)
    {
        mid_result+=(mids[count]*weight[count]);
        div+=weight[count];
    }
    
    double result=mid_result/div+0.5;
    if(result<0)
    {
        return 0;
    }
    else if(result>CAMERA_W-1)
    {
        return CAMERA_W-1;
    }
    else
    {
        return (local_t)result;
    }
}

static discern_result_t compute_result(local_t *mids)
{
    discern_result_t result={0,0};
    
    result.speed=speed_choose(beeline);
    five_point_smooth(start_line,end_line,mids);
    result.angle=get_average_mid(start_line,end_line,mids)-base_line;
    return result;
}

discern_result_t discern(void)
{
    camera_get_img();//获取图像
    
    img_extract(img, imgbuff, CAMERA_SIZE);//解压为灰度图像
    pixel_t *frame_img=NULL;
    frame_img=get_frame(img);
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, img);//显示图像
    
    local_t *mids;//中线
    mids=get_midline(frame_img);//链接中线与中线有效标记
    
    static discern_result_t result={0,0};
#if(start_line<=15)
    if(mids==NULL)
    {
        return result;
    }
#endif
    result=compute_result(mids);//计算偏角，速度选择
    
    display_lines(start_line,end_line,mids);//显示有效的中线
    return result;//返回识别结果
}
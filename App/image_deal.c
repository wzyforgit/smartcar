#include "include.h"
#include "algorithm.h"
#include <string.h>

/*摄像头硬件*/

static uint8 imgbuff[CAMERA_SIZE];
static uint8 img[CAMERA_H*CAMERA_W];

static void PORTA_IRQHandler(void)
{
    uint8  n;    //引脚号
    uint32 flag;

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

void discern_init(void)
{
    camera_init(imgbuff);
    set_vector_handler(PORTA_VECTORn , PORTA_IRQHandler);   //摄像头场中断
    set_vector_handler(DMA0_VECTORn , DMA0_IRQHandler);     //摄像头DMA中断
    NVIC_SetPriority(PORTA_IRQn,0);
    NVIC_SetPriority(DMA0_IRQn,1);
    read_DIPswitch();
}

#define start_line 5
#define end_line (CAMERA_H-5)
#define base_line 40
static uint8* get_midline(uint8 *image)
{
    static uint8 mids[CAMERA_H];
    memset(mids,CAMERA_W+1,sizeof(uint8)*CAMERA_H);
    
    boundary left_edge,right_edge;
    left_edge=serch_left_black_line(image,start_line,end_line,base_line);
    right_edge=serch_right_black_line(image,start_line,end_line,base_line);
    
    register int32 count;
    
    /*扫描前五行*/
    if(left_edge.done[start_line]+left_edge.done[start_line+1]+left_edge.done[start_line+2]+left_edge.done[start_line+3]+left_edge.done[start_line+4]<=2 &&
       right_edge.done[start_line]+right_edge.done[start_line+1]+right_edge.done[start_line+2]+right_edge.done[start_line+3]+right_edge.done[start_line+4]<=2)
    {
        goto end_of_get_midline;
    }
    /*扫描左右丢线*/
    
    /*补出中线*/
    for(count=start_line;count<=end_line;count++)
    {
        uint8 get_left=left_edge.done[count];
        uint8 get_right=right_edge.done[count];
        if(get_left&&get_right)
        {
            mids[count]=(uint8)(left_edge.edge[count]+right_edge.edge[count])/2;
        }
        else if(!get_left&&get_right)
        {
            mids[count]=(uint8)(right_edge.edge[count]-base_line/2);
        }
        else if(get_left&&!get_right)
        {
            mids[count]=(uint8)(right_edge.edge[count]+base_line/2);
        }
        else
        {
            ;
        }
    }
    
end_of_get_midline:
    return mids;
}

static void display_lines(int32 start,int32 end,uint8 *lines)
{
    Site_t line_site[end-start+1];
    for(int32 count=end;count>=start;--count)
    {
        line_site[count].x=lines[count];
        line_site[count].y=count;
    }
    LCD_points(line_site,end-start+1,RED);
}

static int16 speed_choose(traffic choose)
{
    int16 speed=0;
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

static discern_result compute_midline(uint8 *mids)
{
    discern_result result={0,0};
    
    result.speed=speed_choose(curve);
    
    /*double k=least_square(start_line,end_line,start_line,end_line,mids);
    LCD_printf(0,80,"%4d",(int32)((k-(int32)k)*1000));*/
    
    five_point_smooth(start_line,end_line,mids);
    result.angle=100;
    return result;
}

discern_result discern(void)
{
    camera_get_img();
    
    img_extract(img, imgbuff, CAMERA_SIZE);
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, img);
    uint8 *mids;
    mids=get_midline(img);
    
    discern_result result={0,0};
    result=compute_midline(mids);
    
    display_lines(start_line,end_line,mids);
    return result;
}
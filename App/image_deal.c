#include "include.h"
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

static uint8* get_midline(uint8 *image)
{
    static uint8 mids[CAMERA_H];
    memset(mids,sizeof(uint8)*CAMERA_H,0);
    /*compute*/
    return mids;
}

static void display_midline(uint8 *mids)
{
    Site_t mid_site[CAMERA_H];
    for(int count=CAMERA_H-1;count>=0;--count)
    {
        mid_site[count].x=mids[count];
        mid_site[count].y=count;
    }
    LCD_points(mid_site,CAMERA_H,RED);
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
    /*compute*/
    result.speed=speed_choose(curve);
    result.angle=100;
    return result;
}

discern_result discern(void)
{
    camera_get_img();
    LCD_Img_Binary_Z((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, imgbuff, (Size_t){CAMERA_W, CAMERA_H});
    
    img_extract(img, imgbuff, CAMERA_SIZE);
    uint8 *mids;
    mids=get_midline(img);
    display_midline(mids);
    return compute_midline(mids);
}
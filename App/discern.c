#include "include.h"
#include "algorithm.h"
#include <string.h>

/*摄像头硬件*/
/*warning:由于山外库的BUG，在使用摄像头的时候请勿开启任何编译器优化*/

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

#define high_speed   0x01
#define median_speed 0x02
#define low_speed    0x03
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

static speed_t speed_choose(traffic_t choose)
{
    speed_t speed=0;
    switch(mode)
    {
        case high_speed:
        switch(choose)
        {
            case curve   :speed=400;break;
            case beeline :speed=400;break;
            case crossing:speed=400;break;
            case obstacle:speed=400;break;
            default      :speed=400;break;
        }
        break;
        
        case median_speed:
        switch(choose)
        {
            case curve   :speed=350;break;
            case beeline :speed=350;break;
            case crossing:speed=350;break;
            case obstacle:speed=350;break;
            default      :speed=350;break;
        }
        break;
        
        case low_speed:
        switch(choose)
        {
            case curve   :speed=250;break;
            case beeline :speed=250;break;
            case crossing:speed=250;break;
            case obstacle:speed=250;break;
            default      :speed=250;break;
        }
        break;
        
        default:speed=250;
    }
    return speed;
}

static discern_result_t compute_result(pixel_t *image)
{
    discern_result_t result={0,0};
    traffic_t traffic_status=beeline;
    result.speed=speed_choose(traffic_status);
    result.angle=0;
    return result;
}

discern_result_t discern(void)
{
    camera_get_img();//获取图像
    
    img_extract(img, imgbuff, CAMERA_SIZE);//解压为灰度图像
    
    LCD_Img_gray((Site_t){0, 0}, (Size_t){CAMERA_W, CAMERA_H}, img);//显示原始图像
    
    static discern_result_t result={0,300};//初始偏角，初始速度
    
    result=compute_result(img);//计算偏角，速度选择
    
    return result;//返回识别结果
}
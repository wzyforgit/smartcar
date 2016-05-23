#include "include.h"
#include <math.h>

#if(motor_control==1)

#define quad_module     FTM2

/*为了不影响图像采集和显示，推荐不要小于20*/
#define sampling_period 100

static volatile speed_t motor_speed=0;
static volatile speed_t goal_speed=0;

speed_t get_speed(void)
{
    return motor_speed;
}

/*Ku=4.5,Tu=3*sampling_period=0.3s*/
#define P 0.9
#define I 0.3
#define D 0
#define A (P+I+D)
#define B (P+2*D)
#define C (D)
static void speed_control(void)
{
    static speed_t speed_diff[3]={0,0,0};
    
    int32 _goal_speed=goal_speed;
    int32 _motor_speed=motor_speed;
    
    speed_diff[2]=speed_diff[1];
    speed_diff[1]=speed_diff[0];
    speed_diff[0]=_goal_speed-_motor_speed;
    
    if(abs(speed_diff[0])<5)
    {
        return;
    }
    
    int32 result=(int32)(A*speed_diff[0]+B*speed_diff[1]+C*speed_diff[2]);
//    LCD_printf(0,95,"%5d %5d",_motor_speed,result);
    if(result>0)
    {
        if(result>(800))
            result=800;
        set_motor(motor_forward,(speed_t)(result));
    }
    else
    {
        if(result<(-800))
            result=-800;
        set_motor(motor_back,(speed_t)(-result));
    }
}

static void PIT0_IRQHandler(void)
{
#if(sampling_period==20)
    motor_speed=ftm_quad_get(quad_module);
#else
    //转换为20ms内的编码值
    motor_speed=(speed_t)((double)ftm_quad_get(quad_module)/sampling_period*20);
#endif
    speed_control();
    ftm_quad_clean(quad_module);
    PIT_Flag_Clear(PIT0);
}

void E6A2_init(void)
{
    ftm_quad_init(quad_module);                                 //FTM2 正交解码初始化（所用的管脚可查 port_cfg.h ）
    pit_init_ms(PIT0, sampling_period);                         //初始化PIT0，定时时间为： sampling_period ms
    set_vector_handler(PIT0_VECTORn ,PIT0_IRQHandler);          //设置PIT0的中断服务函数为 PIT0_IRQHandler
    NVIC_SetPriority(PIT0_IRQn,2);
    enable_irq (PIT0_IRQn);                                     //使能PIT0中断
}

#endif//motor_control

void set_speed(speed_t want_speed)
{
#if(motor_control==1)
    goal_speed=want_speed;
#else
    if(want_speed>0)
    {
        set_motor(motor_forward,(speed_t)(want_speed));
    }
    else
    {
        set_motor(motor_back,(speed_t)(-want_speed));
    }
#endif
}

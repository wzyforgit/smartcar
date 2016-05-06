#include "include.h"
#include <math.h>

#define quad_module     FTM2
#define sampling_period 20

static vint16 motor_speed=0;
static vint16 goal_speed=0;

int16 get_speed(void)
{
    return motor_speed;
}

void set_speed(int16 want_speed)
{
    goal_speed=want_speed;
}

#define P 2.5
#define I 0.4
#define D 0.15
#define A (P+I+D)
#define B (P+2*D)
#define C (D)
static void speed_control(void)
{
    static int16 speed_diff[3]={0,0,0};
    static int16 last_result=0;
    
    int16 _goal_speed=goal_speed;
    int16 _motor_speed=motor_speed;
    
    speed_diff[2]=speed_diff[1];
    speed_diff[1]=speed_diff[0];
    speed_diff[0]=_goal_speed-_motor_speed;
    
    if(speed_diff[0]<10)
    {
        return;
    }
    
    int16 result=(int16)(A*speed_diff[0]+B*speed_diff[1]+C*speed_diff[2]+last_result);
    if(result>0)
    {
        if(result>(700))
            result=700;
        set_motor(motor_forward,result);
    }
    else
    {
        if(result<(-700))
            result=-700;
        set_motor(motor_back,(-result));
    }
    last_result=result;
}

static void PIT0_IRQHandler(void)
{
    motor_speed=ftm_quad_get(quad_module)/sampling_period*20;   //转换为20ms内的编码值
    speed_control();
    ftm_quad_clean(quad_module);
    PIT_Flag_Clear(PIT0);
}

void E6A2_init(void)
{
    ftm_quad_init(quad_module);                                 //FTM2 正交解码初始化（所用的管脚可查 port_cfg.h ）
    pit_init_ms(PIT0, sampling_period);                         //初始化PIT0，定时时间为： sampling_period ms
    set_vector_handler(PIT0_VECTORn ,PIT0_IRQHandler);
    enable_irq (PIT0_IRQn);                                     //使能PIT0中断
}

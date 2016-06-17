#include "include.h"
#include <math.h>

#if(motor_control==1)

#define quad_module     FTM2

/*Ϊ�˲�Ӱ��ͼ��ɼ�����ʾ���Ƽ���ҪС��20*/
#define sampling_period 100

static volatile speed_t goal_speed=0;

#define P 0.8
#define I 0.3
#define D 0
#define A (P+I+D)
#define B (P+2*D)
#define C (D)
static void speed_control(speed_t motor_speed)
{
    static speed_t speed_diff[3]={0,0,0};
    
    speed_t _goal_speed=goal_speed;
    
    speed_diff[2]=speed_diff[1];
    speed_diff[1]=speed_diff[0];
    speed_diff[0]=_goal_speed-motor_speed;
    
    if(abs(speed_diff[0])<5)
    {
        return;
    }
    
    int32 result=(int32)(A*speed_diff[0]+B*speed_diff[1]+C*speed_diff[2]);
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
    speed_t motor_speed=ftm_quad_get(quad_module);
#else
    //ת��Ϊ20ms�ڵı���ֵ
    speed_t motor_speed=(speed_t)((double)ftm_quad_get(quad_module)/sampling_period*20);
#endif
    speed_control(motor_speed);
    vcan_sendware(&motor_speed,sizeof(motor_speed));//�����ٶȵ�����ʾ����
    ftm_quad_clean(quad_module);
    PIT_Flag_Clear(PIT0);
}

void E6A2_init(void)
{
    uart_init(UART4,115200);
    ftm_quad_init(quad_module);                                 //FTM2 ���������ʼ�������õĹܽſɲ� port_cfg.h ��
    pit_init_ms(PIT0, sampling_period);                         //��ʼ��PIT0����ʱʱ��Ϊ�� sampling_period ms
    set_vector_handler(PIT0_VECTORn ,PIT0_IRQHandler);          //����PIT0���жϷ�����Ϊ PIT0_IRQHandler
    NVIC_SetPriority(PIT0_IRQn,2);
    enable_irq (PIT0_IRQn);                                     //ʹ��PIT0�ж�
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

#include "include.h"

#define Inverters 0

#define MOTOR_FTM   FTM0
#define MOTOR1_PWM  FTM_CH3
#define MOTOR2_PWM  FTM_CH4

#define MOTOR1_PWM_IO  FTM0_CH3
#define MOTOR2_PWM_IO  FTM0_CH4

#define MOTOR_HZ    50//(20*FTM0_PRECISON0)

void motor_init(void)
{
    ftm_pwm_init(MOTOR_FTM, MOTOR1_PWM,MOTOR_HZ,FTM0_PRECISON);      //初始化 电机 PWM
    ftm_pwm_init(MOTOR_FTM, MOTOR2_PWM,MOTOR_HZ,FTM0_PRECISON);      //初始化 电机 PWM
}

void set_motor(motor_path path,uint32 duty)
{
    #if (Inverters==1)
    uint32 pwm_out=FTM0_PRECISON-duty;
    #else
    uint32 pwm_out=duty;
    #endif
    
    if(path==motor_forward)
    {
        ftm_pwm_duty(MOTOR_FTM, MOTOR1_PWM,pwm_out);
        #if (Inverters==1)
        ftm_pwm_duty(MOTOR_FTM, MOTOR2_PWM,FTM0_PRECISON);
        #else
        ftm_pwm_duty(MOTOR_FTM, MOTOR2_PWM,0);
        #endif
    }
    else if(path==motor_back)
    {
        #if (Inverters==1)
        ftm_pwm_duty(MOTOR_FTM, MOTOR2_PWM,FTM0_PRECISON);
        #else
        ftm_pwm_duty(MOTOR_FTM, MOTOR2_PWM,0);
        #endif
        ftm_pwm_duty(MOTOR_FTM, MOTOR2_PWM,pwm_out);
    }
}

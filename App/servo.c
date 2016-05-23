#include "include.h"
#include <math.h>

#define servo_FTM   FTM1
#define servo_CH    FTM_CH0
#define servo_HZ    (300)
#define median      (5500)

void servo_init(void)
{
    ftm_pwm_init(servo_FTM, servo_CH,servo_HZ,FTM1_PRECISON);
}

void set_servo(servo_path path,duty_t angle)
{
    duty_t pwm_out=0;
    
    if(path==servo_right)
    {
        if(angle>1100)
            pwm_out=FTM1_PRECISON-(median-1100);
        else
            pwm_out=FTM1_PRECISON-(median-angle);
    }
    else if(path==servo_left)
    {
        if(angle>1150)
            pwm_out=FTM1_PRECISON-(median+1150);
        else
            pwm_out=FTM1_PRECISON-(median+angle);
    }
    
    ftm_pwm_duty(servo_FTM, servo_CH,pwm_out);
}

#define P_base 42
#define P_step 0.075
static void angle_control(angle_t want_angle)
{
    LCD_printf(0,95,"%3d",want_angle);
    if(abs(want_angle)<2)
    {
        return;
    }
    double P;
    if(want_angle>=0)
    {
        P=P_base*(want_angle*P_step+1);
        int32 result=(int32)((P*want_angle)+0.5);
        set_servo(servo_right,result);
    }
    else
    {
        P=P_base*(want_angle*P_step-1);
        int32 result=(int32)((P*want_angle)+0.5);//P和want_angle均为负值
        set_servo(servo_left,result);
    }
}

void set_angle(angle_t angle)
{
    angle_control(angle);
}

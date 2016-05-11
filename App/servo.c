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

void set_servo(servo_path path,uint32 angle)
{
    uint32 pwm_out=0;
    
    if(path==servo_right)
    {
        pwm_out=FTM1_PRECISON-(median-angle);
    }
    else if(path==servo_left)
    {
        pwm_out=FTM1_PRECISON-(median+angle);
    }
    
    ftm_pwm_duty(servo_FTM, servo_CH,pwm_out);
}

#define P 4
static void angle_control(int32 want_angle)
{
    static int32 last_angle=0;
    
    int32 result=P*(want_angle-last_angle)+last_angle;
    if(result>0)
    {
        set_servo(servo_right,result);
    }
    else
    {
        set_servo(servo_left,abs(result));
    }
    last_angle=result;
}

void set_angle(int32 angle)
{
    angle_control(angle);
}

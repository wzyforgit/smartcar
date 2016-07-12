#include "include.h"
#include <math.h>

#define servo_FTM   FTM1
#define servo_CH    FTM_CH0
#define servo_HZ    (300)
#define median      (5500)

#define high_speed   0x01
#define median_speed 0x02
#define low_speed    0x03

double la=0;
double lb=0;
double ra=0;
double rb=0;
double D=0;
speed_t run_speed=0;
int32 mode;

static void read_DIPswitch(void)
{
    gpio_init(PTC16,GPI,0);
    gpio_init(PTC17,GPI,0);
    gpio_init(PTC18,GPI,0);
    gpio_init(PTC19,GPI,0);
    
    mode=GPIO_GET_NBIT(3,PTC16);
    switch(mode)
    {
        case high_speed:
             la=0.26;
             lb=7.5;
             ra=2.1;
             rb=5;
             run_speed=240;
             LCD_printf(0,61,"high_speed:%d",mode);
             break;
        
        case median_speed:
             la=0.18;
             lb=7.5;
             ra=1.8;
             rb=5;
             run_speed=200;
             LCD_printf(0,61,"median_speed:%d",mode);
             break;

        case low_speed:
             la=0.10;
             lb=5;
             ra=0.41;
             rb=5;
             run_speed=150;
             LCD_printf(0,61,"low_speed:%d",mode);
             break;

        default:
             LCD_printf(0,0,"mode error:%d",mode);
             while(1);
    }
}

void servo_init(void)
{
    ftm_pwm_init(servo_FTM, servo_CH,servo_HZ,FTM1_PRECISON);
    read_DIPswitch();
}

void set_servo(servo_path path,duty_t angle)
{
    duty_t pwm_out=0;
    
    if(path==servo_right)
    {
        if(angle>1200)
            pwm_out=FTM1_PRECISON-(median-1250);
        else
            pwm_out=FTM1_PRECISON-(median-angle);
    }
    else if(path==servo_left)
    {
        if(angle>1250)
            pwm_out=FTM1_PRECISON-(median+1250);
        else
            pwm_out=FTM1_PRECISON-(median+angle);
    }
    
    ftm_pwm_duty(servo_FTM, servo_CH,pwm_out);
}

static void angle_control(angle_t err)
{
    static double errs[4]={0};
    errs[3]=errs[2];
    errs[2]=errs[1];
    errs[1]=errs[0];
    errs[0]=err;
    if(abs(err)<4&&fabs(errs[0]-errs[1])<4)
    {
        return;
    }
    double P,D;
    int32 result;
    if(err>=0)
    {
        P=(err*err)*ra+rb;
    }
    else
    {
        P=(err*err)*la+lb;
    }
    if(mode==high_speed||mode==median_speed)
    {
        D=P/3;
    }
    else
    {
        D=0;
    }
    result=(int32)(err*P+D*(errs[0]-errs[2])+0.5);
    if(result>=0)
    {
        set_servo(servo_right,result);
    }
    else
    {
        set_servo(servo_left,-result);
    }
}

void set_angle(angle_t angle)
{
    angle_control(angle);
}

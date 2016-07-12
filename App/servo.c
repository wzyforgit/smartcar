#include "include.h"
#include <math.h>

#define servo_FTM   FTM1
#define servo_CH    FTM_CH0
#define servo_HZ    (300)
#define median      (5500)

static double a=0;
static double b=0;

void servo_init(void)
{
    ftm_pwm_init(servo_FTM, servo_CH,servo_HZ,FTM1_PRECISON);
    flash_init();
    uint32 data = flash_read(FLASH_SECTOR_NUM-1, 0, uint32);
    a=(double)(data>>16)/100;
    b=(double)(data&0xffff)/10;
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

void angle_debuger(void)
{
    if(key_get(KEY_U)==KEY_DOWN)
    {
        DELAY_MS(10);
        if(key_get(KEY_U)==KEY_DOWN)
        {
            a+=0.1;
        }
        while(key_get(KEY_U)==KEY_DOWN);
    }

    if(key_get(KEY_D)==KEY_DOWN)
    {
        DELAY_MS(10);
        if(key_get(KEY_D)==KEY_DOWN)
        {
            a-=0.1;
        }
        while(key_get(KEY_D)==KEY_DOWN);
    }

    if(key_get(KEY_B)==KEY_DOWN)
    {
        DELAY_MS(10);
        if(key_get(KEY_B)==KEY_DOWN)
        {
            flash_erase_sector(FLASH_SECTOR_NUM-1);
            if(flash_write(FLASH_SECTOR_NUM-1, 0, ((int)(a*100)<<16|(int)(b*10)))==1)
            {
                LCD_printf(0,110,"success!");
            }
            else
            {
                LCD_printf(0,110,"fail!");
            }
        }
        while(key_get(KEY_B)==KEY_DOWN);
    }

    if(key_get(KEY_L)==KEY_DOWN)
    {
        DELAY_MS(10);
        if(key_get(KEY_L)==KEY_DOWN)
        {
            b-=0.5;
        }
        while(key_get(KEY_L)==KEY_DOWN);
    }

    if(key_get(KEY_R)==KEY_DOWN)
    {
        DELAY_MS(10);
        if(key_get(KEY_R)==KEY_DOWN)
        {
            b+=0.5;
        }
        while(key_get(KEY_R)==KEY_DOWN);
    }
    LCD_printf(0,95,"%3d %3d",(int)(a*100),(int)(b*10));
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
//    angle_debuger();//ÎåÖá°´¼üµ÷ÊÔ
    if(err>=0)
    {
        P=(err*err)*0.41+5;//1.0 15
    }
    else
    {
        P=(err*err)*0.10+7.5;
    }
    D=0;
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

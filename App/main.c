 /*!
  *     COPYRIGHT NOTICE
  *     Copyright (c) 2013,山外科技
  *     All rights reserved.
  *     技术讨论：山外论坛 http://www.vcan123.com
  *
  *     除注明出处外，以下所有内容版权均属山外科技所有，未经允许，不得用于商业用途，
  *     修改内容时必须保留山外科技的版权声明。
  *
  * @file       main.c
  * @brief      山外K60 平台主程序
  * @author     山外科技
  * @version    v5.0
  * @date       2013-08-28
  */

#include "include.h"

#define motor_debug   1
#define servo_debug   2
#define E6A2_debug    3
#define LCD_debug     4
#define discern_debug 5

#define HDdebug servo_debug

#if(HDdebug!=motor_debug&&HDdebug!=servo_debug&&HDdebug!=E6A2_debug&&HDdebug!=LCD_debug&&HDdebug!=discern_debug)
#error Unknown debug mode
#endif

void init(void)
{
    DisableInterrupts;
    
#if(HDdebug==motor_debug)
    motor_init();     //电机
#elif(HDdebug==servo_debug)
    servo_init();     //舵机
#elif(HDdebug==E6A2_debug)
    E6A2_init();      //编码器
#elif(HDdebug==discern_debug)
    discern_init();   //识别模块
#endif

#if(HDdebug==E6A2_debug||HDdebug==LCD_debug||HDdebug==discern_debug)
    LCD_init();       //LCD屏
#endif

    EnableInterrupts;
}

void main(void)
{
    init();
    
#if(HDdebug==motor_debug)
    set_motor(motor_back,500);
#elif(HDdebug==servo_debug)
    set_servo(servo_right,0);
//    set_servo(servo_left,900);
#elif(HDdebug==E6A2_debug)
    while(1)
    {
        LCD_printf(0,0,"%5d",get_speed());
    }
#elif(HDdebug==discern_debug)
    discern_result control_result;
    while(1)
    {
        control_result=discern();
        LCD_printf(0,61,"%3d  %3d",control_result.speed,control_result.angle);
    }
#else
    LED_printf(0,0,"12345");
#endif
    while(1);
}

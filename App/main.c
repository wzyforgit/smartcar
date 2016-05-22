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

void LED_init(void)
{
    led_init(LED0);
    led_init(LED1);
    led_init(LED2);
    led_init(LED3);
}

void init(void)
{
    DisableInterrupts;
    
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);//中断优先级组
    motor_init();     //电机
    servo_init();     //舵机
#if(motor_control==1)
    E6A2_init();      //编码器
#endif
    LCD_init();       //LCD屏
    discern_init();   //识别模块
    LED_init();
    
    EnableInterrupts;
    
    set_speed(0);
    set_servo(servo_right,0);
}

void main(void)
{
    init();
    discern_result_t control_result;
    while(1)
    {
        control_result=discern();
        set_angle(control_result.angle);
#if(motor_control==1)
        set_speed(control_result.speed);
        LCD_printf(0,95,"%5d  %5d",control_result.angle,control_result.speed);
#else
        set_speed(250);
#endif
    }
}

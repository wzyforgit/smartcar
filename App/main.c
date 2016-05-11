 /*!
  *     COPYRIGHT NOTICE
  *     Copyright (c) 2013,ɽ��Ƽ�
  *     All rights reserved.
  *     �������ۣ�ɽ����̳ http://www.vcan123.com
  *
  *     ��ע�������⣬�����������ݰ�Ȩ����ɽ��Ƽ����У�δ����������������ҵ��;��
  *     �޸�����ʱ���뱣��ɽ��Ƽ��İ�Ȩ������
  *
  * @file       main.c
  * @brief      ɽ��K60 ƽ̨������
  * @author     ɽ��Ƽ�
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
    motor_init();     //���
#elif(HDdebug==servo_debug)
    servo_init();     //���
#elif(HDdebug==E6A2_debug)
    E6A2_init();      //������
#elif(HDdebug==discern_debug)
    discern_init();   //ʶ��ģ��
#endif

#if(HDdebug==E6A2_debug||HDdebug==LCD_debug||HDdebug==discern_debug)
    LCD_init();       //LCD��
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

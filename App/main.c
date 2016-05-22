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
    
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);//�ж����ȼ���
    motor_init();     //���
    servo_init();     //���
#if(motor_control==1)
    E6A2_init();      //������
#endif
    LCD_init();       //LCD��
    discern_init();   //ʶ��ģ��
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

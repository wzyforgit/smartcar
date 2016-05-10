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

void init(void)
{
    DisableInterrupts;
    
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);//�ж����ȼ���
    motor_init();     //���
    servo_init();     //���
    E6A2_init();      //������
    LCD_init();       //LCD��
    discern_init();   //ʶ��ģ��
    
    EnableInterrupts;
}

void main(void)
{
    init();
    set_speed(0);
    set_servo(servo_right,0);
    discern_result_t control_result;
    while(1)
    {
        control_result=discern();
        set_angle(control_result.angle);
        set_speed(control_result.speed);
        LCD_printf(0,95,"%5d  %5d",control_result.angle,control_result.speed);
    }
}

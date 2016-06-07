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
    uart_init(UART4,9600);
    
    EnableInterrupts;
    
    set_speed(0);
    set_servo(servo_right,0);
}

void main(void)
{
    init();
    discern_result_t control_result;
    uint8 distance[2];
    uint16 distance_result;
    while(1)
    {
        control_result=discern();
        set_angle(control_result.angle);
        
        uart_putchar(UART4,0x55);
        uart_getchar(UART4,(char*)&distance[0]);
        uart_getchar(UART4,(char*)&distance[1]);
        distance_result=(distance[0]<<8)|distance[1];
        LCD_printf(0,110,"%5u",distance_result);
        
#if(motor_control==1)
        set_speed(control_result.speed);
        LCD_printf(0,90,"%5d",control_result.speed);
#else
        set_speed(500);
#endif
    }
}

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

void debuger_init(void)
{
    led_init(LED0);
    led_init(LED1);
    led_init(LED2);
    led_init(LED3);

    key_init(KEY_U);
    key_init(KEY_D);
    key_init(KEY_L);
    key_init(KEY_R);
    key_init(KEY_B);
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
    debuger_init();

    EnableInterrupts;
    
    set_speed(0);
    set_servo(servo_right,0);
}

void main(void)
{
    extern flag_t f_start;
    count_t end_count=0;
    init();
    discern_result_t control_result={0,0};
    while(1)
    {
        control_result=discern();
        set_angle(control_result.angle);
        if(GPIO_GET_NBIT(1,PTC19))
        {
            if(!f_start)
            {
                set_speed(150);
            }
            else if(f_start&&end_count<=20)
            {
                set_speed(150);
                end_count++;
            }
            else if(end_count>20)
            {
                set_speed(0);
            }
        }
        else
        {
            set_speed(0);
        }
    }
}

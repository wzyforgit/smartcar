#ifndef _SERVO_H_
#define _SERVO_H_

typedef enum
{
    servo_left,
    servo_right
}servo_path;

extern void servo_init(void);
extern void set_servo(servo_path path,uint32 angle);
extern void set_angle(int32 angle);

#endif
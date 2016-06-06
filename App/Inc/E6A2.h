#ifndef _E6A2_H_
#define _E6A2_H_

#define motor_control 1

#if(motor_control==1)
extern void  E6A2_init(void);
extern speed_t get_speed(void);
#endif

extern void set_speed(speed_t want_speed);

#endif
#ifndef _DISCERN_H_
#define _DISCERN_H_

typedef struct
{
    angle_t angle;
    speed_t speed;
}discern_result_t;

typedef enum
{
    curve,
    beeline,
    crossing
}traffic;

extern void discern_init(void);
extern discern_result_t discern(void);

#endif
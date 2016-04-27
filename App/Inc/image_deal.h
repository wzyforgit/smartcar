#ifndef _IMAGE_DEAL_H_
#define _IMAGE_DEAL_H_

typedef struct
{
    int32 angle;
    int16 speed;
}discern_result;

typedef enum
{
    curve,
    beeline,
    crossing
}traffic;

extern void discern_init(void);
extern discern_result discern(void);

#endif
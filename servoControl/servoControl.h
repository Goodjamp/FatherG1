#ifndef __SERVO_CONTROL_H__
#define __SERVO_CONTROL_H__

#include "stdio.h"

typedef void (*timerCB)(void);

typedef enum {
    CLOCKWISE,
    COUNTERCLOCKWISE,
} SERVO_CONTROL_DIRECTION;

void servoControlInit(timerCB cbIn);
void servoControlStart(uint32_t speed, SERVO_CONTROL_DIRECTION direction);
void servoControlStop(void);
void servoControlSetDirection(SERVO_CONTROL_DIRECTION direction);
void servoControlSetSpeed(uint32_t speed);

#endif

#ifndef __SERVO_CONTROL_H__
#define __SERVO_CONTROL_H__

#include "stdio.h"

typedef enum {
    CLOCKWISE,
    COUNTERCLOCKWISE,
} SERVO_CONTROL_DIRECTION;

void servoControlInit(void);
void servoControlStart(uint32_t speed, SERVO_CONTROL_DIRECTION direction);
void servoControlStop(void);
void servoControlSetDirection(SERVO_CONTROL_DIRECTION direction);
void servoControlSetSpeed(uint32_t speed);

#endif

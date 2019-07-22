/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stdint.h"

#include "stm32f10x_conf.h"

#include "servoControl.h"

uint32_t sysTime = 0;

void sysTimeProcessing(void) {
    sysTime++;
}

uint32_t updateTime = 100;
uint16_t pwmVal     = 0;

int main(void)
{
    servoControlInit(sysTimeProcessing);
    servoControlStart(20, CLOCKWISE);
    while(1)
    {
        if(sysTime > updateTime) {
            updateTime = sysTime + 25;
            pwmVal += 100;
            if(pwmVal >= 1100) {
                pwmVal = 0;
            }
            servoControlSetSpeed(pwmVal);
        }
    }
}

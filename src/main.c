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
#include "stm32f10x_conf.h"

int main(void)
{
    servoControlInit();
    servoControlStart(20, CLOCKWISE);
    while(1)
    {

    }
}

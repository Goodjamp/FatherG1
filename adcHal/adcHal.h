#ifndef __MESSUREMENT_H__
#define __MESSUREMENT_H__

#include "stdint.h"

typedef void (*BufferReadCB)(uint16_t rezBuff[]);

void measurementInit(BufferReadCB bufferReadyCB, uint16_t buffer[], uint32_t bufferSize);
void measurementStart(void);
void measurementStop(void);

#endif

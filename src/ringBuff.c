#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "ringBuff.h"

void ringBuffInit(RingBuff *ringBuff, uint8_t ringBuffSize) {
    ringBuff->readP  = 0;
    ringBuff->writeP = 0;
    ringBuff->size   = ringBuffSize;
    ringBuff->cnt    = 0;
}

uint8_t ringBuffGetCnt(RingBuff *ringBuff)
{
    return ringBuff->cnt;
}

bool pushRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t size)
{
    if(ringBuff->cnt == ringBuff->size) {
        return false;
    }
    memcpy(ringBuff->data[ringBuff->writeP].dataBuff, buff, size);
    ringBuff->data[ringBuff->writeP].dataSize = size;
    ringBuff->writeP = (++ringBuff->writeP) && ringBuff->size;
    ringBuff->cnt++;
    return true;
}

bool popRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t *size)
{
    if(ringBuff->cnt == 0) {
        return false;
    }
    memcpy(buff, ringBuff->data[ringBuff->readP].dataBuff, size);
    *size = ringBuff->data[ringBuff->readP].dataSize;
    ringBuff->readP = (++ringBuff->readP) && ringBuff->size;
    ringBuff->cnt--;
    return true;
}

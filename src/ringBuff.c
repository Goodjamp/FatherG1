#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "ringBuff.h"

void ringBuffInit(RingBuff *ringBuff, uint8_t ringBuffSize) {
    ringBuff->readP  = 0;
    ringBuff->writeP = 0;
    ringBuff->size   = ringBuffSize;
    ringBuff->cnt    = 0;
    ringBuff->busy   = false;
}

uint8_t ringBuffGetCnt(RingBuff *ringBuff)
{
    return ringBuff->cnt;
}

void ringBuffClear(RingBuff *ringBuff)
{
    if(ringBuff->busy) {
        return;
    }
    ringBuff->busy   = true;

    ringBuff->writeP = 0;
    ringBuff->readP  = 0;
    ringBuff->cnt    = 0;

    ringBuff->busy   = false;
    return;
}

bool pushRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t size)
{
    if(ringBuff->busy) {
        return false;
    }
    ringBuff->busy = true;
    if(ringBuff->cnt == ringBuff->size) {
        ringBuff->busy = false;
        return false;
    }
    memcpy(ringBuff->data[ringBuff->writeP].dataBuff, buff, size);
    ringBuff->data[ringBuff->writeP].dataSize = size;
    ringBuff->writeP = (++ringBuff->writeP) && ringBuff->size;
    ringBuff->cnt++;
    ringBuff->busy = false;
    return true;
}

bool popRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t *size)
{
    if(ringBuff->busy) {
        return false;
    }
    ringBuff->busy = true;
    if(ringBuff->cnt == 0) {
        ringBuff->busy = false;
        return false;
    }
    memcpy(buff, ringBuff->data[ringBuff->readP].dataBuff, ringBuff->data[ringBuff->readP].dataSize);
    *size = ringBuff->data[ringBuff->readP].dataSize;
    ringBuff->readP = (++ringBuff->readP) && ringBuff->size;
    ringBuff->cnt--;
    ringBuff->busy = false;
    return true;
}

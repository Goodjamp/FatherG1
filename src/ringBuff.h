#ifndef _RING_BUFF_H__
#define _RING_BUFF_H__

#include "stdint.h"
#include "stdbool.h"

#define BUFF_SIZE       64
#define RING_BUFF_DEPTH  8

typedef struct RingBuff {
    struct {
        uint8_t  dataBuff[BUFF_SIZE];
        uint16_t dataSize;
    } data[RING_BUFF_DEPTH];
    uint8_t writeP;
    uint8_t readP;
    uint8_t cnt;
    uint32_t size;
    bool    busy;
} RingBuff;

void ringBuffInit(RingBuff *ringBuff, uint8_t ringBuffSize);
uint8_t ringBuffGetCnt(RingBuff *ringBuff);
bool pushRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t size);
bool popRingBuff(RingBuff *ringBuff, uint8_t buff[], uint32_t *size);
void ringBuffClear(RingBuff *ringBuff);


#endif

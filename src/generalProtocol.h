#ifndef _GENERAL_PROTOCOL_H__
#define _GENERAL_PROTOCOL_H__

#include "stdint.h"
#include "stdbool.h"

#define CHANNEL_CNT 0x7

typedef void (*GpSendCb)(uint8_t buff[], uint32_t size);
typedef void (*GpStopCommandCb)(uint8_t channel);
typedef void (*GpStartClockWiseCommandCb)(uint8_t channel);
typedef void (*GpStartContrClockWiseCommandCb)(uint8_t channel);

typedef struct GpInitCb {
    GpSendCb                  gpSendCb;
    GpStopCommandCb           gpStopCommandCb;
    GpStartClockWiseCommandCb gpStartClockWiseCommandCb;
    GpStartContrClockWiseCommandCb gpStartContrClockWiseCommandCb;
} GpInitCb;

void gpInit(const GpInitCb *gpCbIn);
void gpDecode(uint8_t buff[],  uint32_t size);
bool gpSendADC(uint16_t buff[],  uint16_t adCnt);

#endif

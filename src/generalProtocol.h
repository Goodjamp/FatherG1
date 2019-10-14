#ifndef _GENERAL_PROTOCOL_H__
#define _GENERAL_PROTOCOL_H__

#include "stdint.h"
#include "stdbool.h"

#define CHANNEL_CNT 0x7

typedef enum {
    COMMAND_DATA_FLAG_8_BIT_SIZE,
    COMMAND_DATA_FLAG_16_BIT_SIZE,
    COMMAND_DATA_FLAG_32_BIT_SIZE,
}COMMAND_DATA_FLAG;

typedef void (*GpSendCb)(uint8_t buff[], uint32_t size);
typedef void (*GpStopCommandCb)(uint8_t channel);
typedef void (*GpStartClockWiseCommandCb)(uint8_t channel);
typedef void (*GpStartContrClockWiseCommandCb)(uint8_t channel);
typedef void (*GpStartAutoSwitcherCommandCb)(uint8_t channel,
                                             uint16_t offTime,
                                             uint16_t onTime,
                                             uint32_t cnt);

typedef struct GpInitCb {
    GpSendCb                       gpSendCb;
    GpStopCommandCb                gpStopCommandCb;
    GpStartClockWiseCommandCb      gpStartClockWiseCommandCb;
    GpStartContrClockWiseCommandCb gpStartContrClockWiseCommandCb;
    GpStartAutoSwitcherCommandCb   gpStartAutoSwitcherCommandCb;
} GpInitCb;

void gpInit(const GpInitCb *gpCbIn);
void gpDecode(uint8_t buff[],  uint32_t size);
bool gpSendDataCommand(uint8_t data[],  uint16_t cnt, COMMAND_DATA_FLAG flags, uint8_t channel);

#endif

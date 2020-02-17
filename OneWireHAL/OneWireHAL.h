#ifndef __ONE_WIRE_INTERFACE_API_H__
#define __ONE_WIRE_INTERFACE_API_H__


typedef enum{
    ONE_WIRE_NORMAL,
    ONE_WIRE_BUSY,
    ONE_WIRE_RESET_ERROR,
}OneWireState;

typedef void (*OneWireCB)(bool);

void oneWireInit(OneWireCB oneWireCB);
OneWireState oneWireGateState(void);
void         oneWireGetRxData(uint8_t buff[], uint32_t dataSize);
OneWireState oneWireReset(void);
OneWireState oneWireSend(uint8_t data[], uint32_t dataSize);
OneWireState oneWireReceive(uint32_t dataSize);
OneWireState oneWireResetBloking(void);
OneWireState oneWireSendBloking(uint8_t data[], uint32_t dataSize);
OneWireState oneWireReceiveBloking(uint8_t data[], uint32_t dataSize);

#endif

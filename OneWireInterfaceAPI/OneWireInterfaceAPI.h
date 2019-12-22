#ifndef __ONE_WIRE_INTERFACE_API_H__
#define __ONE_WIRE_INTERFACE_API_H__

typedef void (*OneWireCB)(bool);

void oneWireInit(OneWireCB oneWireCB);
bool oneWireGateState(void);
bool oneWireGetRxData(uint8_t buff[], uint32_t dataSize);
bool oneWireReset(void);
bool oneWireSend(uint8_t data[], uint32_t dataSize);
bool oneWireReceive(uint32_t dataSize);
bool oneWireResetBloking(void);
void oneWireSendBloking(uint8_t data[], uint32_t dataSize);
void oneWireReceiveBloking(uint8_t data[], uint32_t dataSize);

#endif

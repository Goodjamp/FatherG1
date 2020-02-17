#include "FreeRTOS.h"
#include "task.h"

#include "TemperatureSensorHAL.h"
#include "DS18B20Sensor.h"
#include "OneWireHAL.h"

#define ONE_WIRE_TRANSACRION_CNT_MAX 10

/*
typedef enum{
    ONE_WIRE_NORMAL,
    ONE_WIRE_BUSY,
    ONE_WIRE_RESET_ERROR,
}OneWireState;

typedef void (*OneWireCB)(bool);

void oneWireInit(OneWireCB oneWireCB);
OneWireState oneWireResetBloking(void);
OneWireState oneWireSendBloking(uint8_t data[], uint32_t dataSize);
OneWireState oneWireReceiveBloking(uint8_t data[], uint32_t dataSize);

typedef struct {
    ReadCb  readCb;
    WriteCb writeCb;
    ResetCb resetCb;
} Ds18b20Cb;

typedef enum {
    DS18B20_RESOLUTION_9_BIT  = 0b00011111,
    DS18B20_RESOLUTION_10_BIT = 0b00111111,
    DS18B20_RESOLUTION_11_BIT = 0b01011111,
    DS18B20_RESOLUTION_12_BIT = 0b01111111,
    DS18B20_RESOLUTION_MASK   = 0B01111111,
}DS18B20_RESOLUTION;

void ds18b20SetCb(Ds18b20Cb ds18b20Cb);
bool ds18b20SetResolution(DS18B20_RESOLUTION resolution);
bool ds18b20GetTemperature(int32_t *temperature);
bool ds18b20ReadRom(uint8_t rom[8]);
bool ds18b20SetResolutionByRom(DS18B20_RESOLUTION resolution,
                               uint8_t rom[8]);
bool ds18b20GetTemperatureByRom(int32_t *temperature,
                                uint8_t rom[8]);

*/

bool oneWireRead(uint8_t data[], uint32_t dataSize)
{
    uint32_t k = ONE_WIRE_TRANSACRION_CNT_MAX;
    while(k--) {
       if(oneWireReceiveBloking(data, dataSize) == ONE_WIRE_NORMAL) {
           return true;
       }
    }
    return false;
}

bool oneWireWriteCb(uint8_t data[], uint32_t dataSize)
{
    uint32_t k = ONE_WIRE_TRANSACRION_CNT_MAX;
    while(k--) {
       if(oneWireSendBloking(data, dataSize) == ONE_WIRE_NORMAL) {
           return true;
       }
    }
    return false;
}

bool oneWireResetCb(void)
{
    uint32_t k = ONE_WIRE_TRANSACRION_CNT_MAX;
    while(k--) {
       if(oneWireResetBloking() == ONE_WIRE_NORMAL) {
           return true;
       }
    }
    return false;
}

bool temperatureInit(void)
{
    Ds18b20Cb ds18b20Cb = {
        .readCb  = oneWireRead,
        .writeCb = oneWireWriteCb,
        .resetCb = oneWireResetCb,
    };
    oneWireInit(NULL);
    ds18b20SetCb(ds18b20Cb);
    if(ds18b20SetResolution(DS18B20_RESOLUTION_12_BIT)) {
        return true;
    }
    vTaskDelay(10);
    return false;
}

bool temperatureGetTemperature(int32_t *temperature)
{
    if(ds18b20GetTemperature(temperature)) {
        return true;
    }
    return false;
}



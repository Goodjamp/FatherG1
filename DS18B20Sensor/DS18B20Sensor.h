#ifndef __DS18B20_SENSOR_H__
#define __DS18B20_SENSOR_H__

#include "stdint.h"
#include "stdbool.h"

typedef bool (*ReadCb)(uint8_t data[], uint32_t dataSize);
typedef bool (*WriteCb)(uint8_t data[], uint32_t dataSize);
typedef bool (*ResetCb)(void);

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

#endif

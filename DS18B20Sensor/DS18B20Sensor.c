#include "DS18B20Sensor.h"
#include "stdint.h"
#include "stdlib.h"

enum {
    SEARCH_ROM        = 0xF0,
    READ_ROM          = 0x33,
    MATCH_ROM         = 0x55,
    SKIP_ROM          = 0xCC,
    ALARM_SEARCH      = 0xEC,
    CONVERT_T         = 0x44,
    WRITE_SCRATCHPAD  = 0x4E,
    READ_SCRATCHPAD   = 0xBE,
    COPY_SCRATCHPAD   = 0x48,
    RECALL_E2         = 0xB8,
    READ_POWER_SUPPLY = 0xB4
} DS18B20_COMMANS;

static Ds18b20Cb cbList = {
    .readCb  = NULL,
    .writeCb = NULL,
    .resetCb = NULL,
};

#pragma pack(push, 1)
struct Ds18b20Scratchpad {
    int16_t  temperature;
    uint8_t  tH;
    uint8_t  tL;
    uint8_t  configuration;
    uint8_t  reserved[3];
    uint8_t  crc;
};
#pragma pack(pop)

union Ds18b20ScratchpadBuff {
    struct Ds18b20Scratchpad ds18b20Scratchpad;
    uint8_t                  buff[sizeof(struct Ds18b20Scratchpad)];
};

void ds18b20SetCb(Ds18b20Cb ds18b20Cb)
{
    cbList = ds18b20Cb;
}

static bool ds18b20IsCbSet(void)
{
    if( cbList.readCb && cbList.resetCb && cbList.writeCb) {
        return true;
    }
    return false;
}

bool ds18b20SetResolution(DS18B20_RESOLUTION resolution)
{
    if(!ds18b20IsCbSet()) {
        return false;
    }
    union Ds18b20ScratchpadBuff ds18b20ScratchpadBuff;
    /*Read current sketchpad */
    uint8_t command = SKIP_ROM;
    if(cbList.resetCb() == false) {
        return false;
    }
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    command = READ_SCRATCHPAD;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    if(cbList.readCb(ds18b20ScratchpadBuff.buff, sizeof(ds18b20ScratchpadBuff)) == false) {
        return false;
    }
     /* compare current resolution with user settings */
    if(ds18b20ScratchpadBuff.ds18b20Scratchpad.configuration == resolution) {
        return true;
    }
    /*write new resolution*/
    if(cbList.resetCb() == false) {
        return false;
    }
    command = SKIP_ROM;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    command = WRITE_SCRATCHPAD;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    uint8_t scratchpadData[] = {ds18b20ScratchpadBuff.ds18b20Scratchpad.tL,
                                ds18b20ScratchpadBuff.ds18b20Scratchpad.tH,
                                resolution};
    if(cbList.writeCb(scratchpadData, sizeof(scratchpadData)) == false) {
        return false;
    }
    /*copy scratchpad to sensor eeprom*/
    if(cbList.resetCb() == false) {
        return false;
    }
    command = SKIP_ROM;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    command = COPY_SCRATCHPAD;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    return true;
}

bool ds18b20GetTemperature(int32_t *temperature)
{
    if(!ds18b20IsCbSet()) {
        return false;
    }
    union Ds18b20ScratchpadBuff ds18b20ScratchpadBuff;
    uint8_t command;
    /*Mes temperature*/
    command = SKIP_ROM;
    if(cbList.resetCb() == false) {
        return false;
    }
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    command = CONVERT_T;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    /*Waite for convert complete*/
    uint32_t cnt = 0;
    uint8_t readData = 0;
    while(cnt < 1000) {
        if(cbList.readCb(&readData, 1) == false) {
            return false;
        }
        if(readData) {
            break;
        }
    }
    /*read scratchpad*/
    command = SKIP_ROM;
    if(cbList.resetCb() == false) {
        return false;
    }
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    command = READ_SCRATCHPAD;
    if(cbList.writeCb(&command, 1) == false) {
        return false;
    }
    if(cbList.readCb(ds18b20ScratchpadBuff.buff, sizeof(ds18b20ScratchpadBuff)) == false) {
        return false;
    }
    /*calculate temperature*/
    bool isMinus = (ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature < 0) ?
                   true : false;
    *temperature = 0;
    if(isMinus) {
        ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature = ~ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature;
        ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature++;
    }
    uint32_t mult = 625;
    for(uint32_t k = 0; k < 4; k++) {
        if(ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature & 0b1) {
            *temperature += mult;
        }
        mult *= 2;
        ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature >>= 1;
    }
    *temperature = (*temperature + ds18b20ScratchpadBuff.ds18b20Scratchpad.temperature
                   * 10000)
                   * ((isMinus) ? (-1) : (1));
    return true;
}



bool ds18b20ReadRom(uint8_t rom[8])
{
    return true;
}

bool ds18b20SetResolutionByRom(DS18B20_RESOLUTION resolution,
                               uint8_t rom[8])
{
    return true;
}

bool ds18b20GetTemperatureByRom(int32_t *temperature,
                                uint8_t rom[8])
{
    return true;
}

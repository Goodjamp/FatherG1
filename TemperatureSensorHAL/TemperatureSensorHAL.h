#ifndef __TEMPERATURE_SENSOR_HAL_H__
#define __TEMPERATURE_SENSOR_HAL_H__

#include "stdint.h"
#include "stdbool.h"

bool temperatureConfig(void);
bool temperatureGetTemperature(int32_t *temperature);


#endif

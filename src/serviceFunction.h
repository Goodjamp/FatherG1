#ifndef __SERVIS_FUNCTION_H__
#define __SERVIS_FUNCTION_H__

#include "stdint.h"

#define REZ_CNT 200
typedef struct MesPeriod{
    uint32_t timeMes[REZ_CNT];
    uint32_t prevTime;
    uint32_t k;
} MesPeriod;

void     initSysTic(void);
uint32_t getSysTime(void);
void     addTimeMes(MesPeriod *mesPeriod);

#endif

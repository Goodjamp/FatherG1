#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

typedef enum {
    MEAS_SHORT,
    MEAS_START,
    MEAS_STOP,
    MEAS_ACTION_CNT
} MEAS_ACTION;

typedef void (*MeasShortCompliteCb)(uint32_t rezMeasShort);
typedef void (*MeasContinuousCompleteCb)(uint32_t rezMeasBuff, uint32_t buffSize);

void measSetContinuousCompliteCb(MeasContinuousCompleteCb measContinuousCompleteCb);
void measSetShortCompliteCb(MeasShortCompliteCb measShortCompliteCb);
void measSetMeasAction(MEAS_ACTION measAction);
void vMeasTask(void *pvParameters);

#endif

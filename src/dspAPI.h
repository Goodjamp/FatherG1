#ifndef __DSP_API_H__
#define __DSP_API_H__

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    LOW_PATH,
    BAND_PATH,
    HIGH_PATH
}FirFilterType;

typedef enum {
    SINC,
}FirFilterCalcType;

typedef enum {
    WITHOUT_WINDOW,
    BUTTERWORTH,
    BLACKMAN,
    HAMMING,
    NUTTALL
}WindowType;

typedef struct FilterConfig {
    FirFilterType     type;
    FirFilterCalcType calcType;
    WindowType        windowType;
    double            f0;  //central frequency, Gz
    double            fs;  //sampling rate, Gz
    double            df;  //band with, Gz
    uint32_t          q;   // filter order
}FilterConfig;

typedef struct FiltrationHandler {
    int32_t *first;  // first element of rezults buff
    int32_t *last;   // last element of rezults buff
    int32_t *write;  // write pointer of buff
    int32_t *coeff;  // filter coefficients
    uint32_t q;       // filter order
    int32_t  scallingCoeff;
} FiltrationHandler;


bool    dspInitFiltr(FiltrationHandler *handler,
                     FilterConfig filtrConfig,
                     int32_t coeffBuff[],
                     int32_t buff[],
                     uint32_t maxInput);
bool    dspFiltrationReset(FiltrationHandler *filtrationHandler);
int32_t dspFiltration(FiltrationHandler *filtrationHandler, int32_t value);

#endif

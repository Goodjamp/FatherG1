#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

#include "stm32f10x_conf.h"

#include "servoControl.h"
#include "measurement.h"
#include "serviceFunction.h"
#include "ringbuff.h"
#include "generalProtocol.h"
#include "usbHIDInterface.h"
#include "usb_user_setings.h"
#include "dspAPI.h"

#define ADC_REZ_CNT        4
#define ADC_REZ_BUFF_SIZE  (CHANNEL_CNT * ADC_REZ_CNT * 2)
#define EP_N               1

/*FIR LPF settings*/
#define FILTER_ORDER  128
#define CUTOFF_HZ     200
#define MAX_DSP_VAL   4095
FiltrationHandler filtrationHandler;
FilterConfig firFilterConfig = {
    .type = LOW_PATH,
    .calcType = SINC,
    .windowType = BLACKMAN,
    .df = CUTOFF_HZ,
    .q  = FILTER_ORDER,
};
int32_t filterCoeff[FILTER_ORDER];
int32_t filteringBuff[FILTER_ORDER];

void gpSendCb(uint8_t buff[], uint32_t size);
void gpStopCommandCb(uint8_t channel);
void gpStartClockWiseCommandCb(uint8_t channel);
void gpStartContrClockWiseCommandCb(uint8_t channel);

uint16_t  adcRezBuff[ADC_REZ_BUFF_SIZE];
MesPeriod timeAdc;
RingBuff  rxRingBuff;
RingBuff  txRingBuff;
uint8_t   txBuff[BUFF_SIZE];
uint8_t   rxBuff[BUFF_SIZE];
bool      isMesDisabled = true;
const GpInitCb gpInitCb = {
    .gpSendCb = gpSendCb,
    .gpStopCommandCb = gpStopCommandCb,
    .gpStartClockWiseCommandCb = gpStartClockWiseCommandCb,
    .gpStartContrClockWiseCommandCb = gpStartContrClockWiseCommandCb,
};

struct {
    bool     isReady;
    uint16_t buff[ADC_REZ_BUFF_SIZE / 2];
}adcRez;

uint8_t testState = 0;

#define REZ_SIZE_TEST  1024
uint16_t rezBuff[REZ_SIZE_TEST];
uint16_t rezCnt = 0;
void adcCompletedCb(uint16_t rez[])
{
    /*
    addTimeMes(&timeAdc);

    if(testState) {
        testPinSet();
        testState = 0;
    } else {
        testPinReset();
        testState = 1;
    }
    */
    if(rez == adcRezBuff) {
        testPinSet();
        testState = 0;
    } else {
        testPinReset();
        testState = 1;
    }
    if(isMesDisabled) {
        return;
    }
    //CHANNEL_CNT * ADC_REZ_CNT
    for(uint8_t k = 0; k < ADC_REZ_CNT; k++) {
        rezBuff[rezCnt++] = rez[k * CHANNEL_CNT];
        if(rezCnt >= REZ_SIZE_TEST) {
            rezCnt = 0;
        }
    }
    //gpSendADC(rez, CHANNEL_CNT * ADC_REZ_CNT );
    memcpy((uint8_t*)adcRez.buff, (uint8_t*)rez, sizeof(adcRez.buff));
    adcRez.isReady = true;
}

void usbHIDRxCB(uint8_t epNumber, uint8_t numRx, uint8_t *rxData)
{
    pushRingBuff(&rxRingBuff, rxData, numRx);
}

void txCompleteCB(void)
{
}

#define REZ_SIZE 256
bool rezPush[REZ_SIZE] = {[0 ... (REZ_SIZE - 1)] = true};
uint16_t cntRez = 0;

void gpSendCb(uint8_t buff[], uint32_t size)
{
    rezPush[cntRez++]= pushRingBuff(&txRingBuff, buff, size);
    if(cntRez >= REZ_SIZE) {
        cntRez = 0;
    }
}

void gpStopCommandCb(uint8_t channel)
{
    servoControlStop();
    measurementStop();
    ringBuffClear(&txRingBuff);
    isMesDisabled = true;
}

void gpStartClockWiseCommandCb(uint8_t channel)
{
    servoControlStart(1, CLOCKWISE);
    measurementStart();
    isMesDisabled = false;
}

void gpStartContrClockWiseCommandCb(uint8_t channel)
{
    servoControlStart(1, COUNTERCLOCKWISE);
    measurementStart();
    isMesDisabled = false;
}

void initDSP(void)
{
    /*
    8 - ADC clocking divider
    2 - ADC bus cloaking divider
    7 - ADC channel quantity
    */
    firFilterConfig.fs = (72000000)/(8 * 2 * (240 + 12) * 7);
    dspInitFiltr(&filtrationHandler, firFilterConfig, filterCoeff, filteringBuff, MAX_DSP_VAL);
}

#define REZ_PROCESSING_SIZE 28
struct {
    uint16_t buf[REZ_PROCESSING_SIZE];
    uint8_t  cnt;
} filtRezBuff;

void rccConfig(void) {
    RCC_PCLK2Config(RCC_HCLK_Div2);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

int main(void)
{
    uint32_t rxSize;
    uint32_t txSize;
    initTestGpio();
    initDSP();
    gpInit(&gpInitCb);
    usbHIDInit();
    usbHIDAddRxCB(usbHIDRxCB);
    usbHIDAddTxCompleteCB(txCompleteCB);
    rccConfig();
    uint32_t cnt = 72000;
    while(cnt-- > 2){}
    measurementInit(adcCompletedCb, adcRezBuff, ADC_REZ_BUFF_SIZE);
    rccConfig();

    //initSysTic();
    ringBuffInit(&rxRingBuff, RING_BUFF_DEPTH);
    ringBuffInit(&txRingBuff, RING_BUFF_DEPTH);
    servoControlInit(NULL);
    //measurementStart();
    rccConfig();
    while(1)
    {
        if(adcRez.isReady) {
            adcRez.isReady= false;
            for(uint8_t k = 0; k < ADC_REZ_CNT; k++) {
                filtRezBuff.buf[filtRezBuff.cnt++] = dspFiltration(&filtrationHandler,
                                                     (uint16_t)(adcRez.buff[k * CHANNEL_CNT]));
            //adcRez.buff[k * CHANNEL_CNT];
                                                               //dspFiltration(&filtrationHandler,
                                                               //    (uint16_t)(adcRez.buff[k * CHANNEL_CNT]));

                if(filtRezBuff.cnt >= REZ_PROCESSING_SIZE) {
                    gpSendDataCommand((uint8_t*)filtRezBuff.buf,
                                      REZ_PROCESSING_SIZE * sizeof(int16_t),
                                      COMMAND_DATA_FLAG_16_BIT_SIZE,
                                      0);
                    filtRezBuff.cnt = 0;
                }
            }
        }
        if(popRingBuff(&rxRingBuff, rxBuff, &rxSize)) {
            gpDecode(rxBuff, rxSize);
        };
        if(usbHIDEPIsReadyToTx(EP_01) ) {
            if(popRingBuff(&txRingBuff, txBuff, &txSize)) {
                usbHIDTx(EP_01, txBuff, BUFF_SIZE);
            }
        }
    }
}

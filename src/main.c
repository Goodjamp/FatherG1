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

#define ADC_REZ_CNT        4
#define ADC_REZ_BUFF_SIZE  (CHANNEL_CNT * ADC_REZ_CNT * 2)
#define EP_N               1

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
uint8_t testState = 0;


void adcCompletedCb(uint16_t rez[])
{
    /*
    addTimeMes(&timeAdc);
    */
    if(testState) {
        testPinSet();
        testState = 0;
    } else {
        testPinReset();
        testState = 1;
    }
    if(isMesDisabled) {
        return;
    }
    gpSendADC(rez, CHANNEL_CNT * ADC_REZ_CNT );
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

int main(void)
{
    uint32_t rxSize;
    uint32_t txSize;
    initTestGpio();
    gpInit( &gpInitCb);
    usbHIDInit();
    usbHIDAddRxCB(usbHIDRxCB);
    usbHIDAddTxCompleteCB(txCompleteCB);
    measurementInit(adcCompletedCb, adcRezBuff, ADC_REZ_BUFF_SIZE);
    servoControlInit(NULL);

    //initSysTic();
    ringBuffInit(&rxRingBuff, RING_BUFF_DEPTH);
    ringBuffInit(&txRingBuff, RING_BUFF_DEPTH);
    measurementStart();

    while(1)
    {
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

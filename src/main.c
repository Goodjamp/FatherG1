#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "stm32f10x_conf.h"

#include "servoControl.h"
#include "measurement.h"
#include "serviceFunction.h"
#include "ringbuff.h"
#include "generalProtocol.h"
#include "usbHIDInterface.h"
#include "usb_user_setings.h"

#define ADC_REZ_SIZE  (56 * 2)
#define EP_N           1

void gpSendCb(uint8_t buff[], uint32_t size);
void gpStopCommandCb(uint8_t channel);
void gpStartClockWiseCommandCb(uint8_t channel);
void gpStartContrClockWiseCommandCb(uint8_t channel);

uint16_t  adcRezBuff[ADC_REZ_SIZE];
MesPeriod timeAdc;
RingBuff  rxRingBuff;
RingBuff  txRingBuff;
uint8_t   txBuff[BUFF_SIZE];
uint8_t   rxBuff[BUFF_SIZE];
bool      TX_BUSY_F = false;
const GpInitCb gpInitCb = {
    .gpSendCb = gpSendCb,
    .gpStopCommandCb = gpStopCommandCb,
    .gpStartClockWiseCommandCb = gpStartClockWiseCommandCb,
    .gpStartContrClockWiseCommandCb = gpStartContrClockWiseCommandCb,
};
/*
void protoPrepareMesRezMessage(uint8_t payload, uint16_t size) {
    dataCommand.command.headr = REZ_MEASUREMENT;
    dataCommand.command.size  = size;
    memcpy(dataCommand.command.buff, payload, size);
}
*/

void adcCompletedCb(uint16_t rez[])
{
    /*
    addTimeMes(&timeAdc);
    if(TX_F) {
        return;
    }
    protoPrepareMesRezMessage((uint8_t *)rez, sizeof(dataCommand.buff));
    TX_F = true;
    */
    gpSendADC(rez,  uint16_t size);
}

void usbHIDRxCB(uint8_t epNumber, uint8_t numRx, uint8_t *rxData)
{
    pushRingBuff(&rxRingBuff, rxData, numRx);
}

void txCompleteCB(void)
{
    TX_BUSY_F = false;
}

void gpSendCb(uint8_t buff[], uint32_t size)
{
    pushRingBuff(&txRingBuff, buff, size);
}

void gpStopCommandCb(uint8_t channel)
{
    servoControlStop();
}

void gpStartClockWiseCommandCb(uint8_t channel)
{
    servoControlStart(1, CLOCKWISE);
}

void gpStartContrClockWiseCommandCb(uint8_t channel)
{
    servoControlStart(1, COUNTERCLOCKWISE);
}

int main(void)
{
    uint32_t rxSize;
    uint32_t txSize;
    gpInit( &gpInitCb);
    usbHIDInit();
    measurementInit(adcCompletedCb, adcRezBuff, ADC_REZ_SIZE);
    usbHIDAddRxCB(usbHIDRxCB);
    usbHIDAddTxCompleteCB(txCompleteCB);

    //initSysTic();
    measurementStart();
    servoControlStart(20, CLOCKWISE);
    ringBuffInit(&rxRingBuff, RING_BUFF_DEPTH);
    ringBuffInit(&txRingBuff, RING_BUFF_DEPTH);
    while(1)
    {
        if(popRingBuff(&rxRingBuff, rxBuff, &rxSize)) {
            gpDecode(rxBuff, rxSize);
        };
        if(popRingBuff(&txRingBuff, txBuff, &txSize)) {
            if( usbHIDEPIsReadyToTx(EP_01) ) {
                usbHIDTx(EP_01, txBuff, txSize);
            }
            TX_BUSY_F = true;
        }
    }
}

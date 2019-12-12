#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

#include "misc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#include "stm32f10x_conf.h"

#include "ringbuff.h"
#include "generalProtocol.h"
#include "usbHIDInterface.h"
#include "usb_user_setings.h"
#include "Measurement.h"

#define EP_N  1

/*FIR LPF settings*/
#define FILTER_ORDER  128
#define CUTOFF_HZ     20
#define MAX_DSP_VAL   4095

int32_t filterCoeff[FILTER_ORDER];
int32_t filteringBuff[FILTER_ORDER];

void gpSendCb(uint8_t buff[], uint32_t size);
void gpStopCommandCb(uint8_t channel);
void gpStartClockWiseCommandCb(uint8_t channel);
void gpStartContrClockWiseCommandCb(uint8_t channel);

RingBuff  rxRingBuff;
RingBuff  txRingBuff;
uint8_t   txBuff[BUFF_SIZE];
uint8_t   rxBuff[BUFF_SIZE];
const GpInitCb gpInitCb = {
    .gpSendCb                       = gpSendCb,
    .gpStopCommandCb                = gpStopCommandCb,
    .gpStartClockWiseCommandCb      = gpStartClockWiseCommandCb,
    .gpStartContrClockWiseCommandCb = gpStartContrClockWiseCommandCb,
};

void usbHIDRxCB(uint8_t epNumber, uint8_t numRx, uint8_t *rxData)
{
    pushRingBuff(&rxRingBuff, rxData, numRx);
}

void txCompleteCB(void)
{
}

void gpSendCb(uint8_t buff[], uint32_t size)
{
    pushRingBuff(&txRingBuff, buff, size);
}

void gpStopCommandCb(uint8_t channel)
{
    measSetMeasAction(MEAS_STOP);
}

void gpStartClockWiseCommandCb(uint8_t channel)
{
    measSetMeasAction(MEAS_START);
}

void gpStartContrClockWiseCommandCb(uint8_t channel)
{
    measSetMeasAction(MEAS_START);
}

void vUsbProtoTask( void *pvParameters )
{
    uint32_t rxSize;
    uint32_t txSize;

    gpInit(&gpInitCb);
    usbHIDInit();
    usbHIDAddRxCB(usbHIDRxCB);
    usbHIDAddTxCompleteCB(txCompleteCB);

    ringBuffInit(&rxRingBuff, RING_BUFF_DEPTH);
    ringBuffInit(&txRingBuff, RING_BUFF_DEPTH);

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

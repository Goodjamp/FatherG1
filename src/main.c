#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "stm32f10x_conf.h"

#include "servoControl.h"
#include "measurement.h"
#include "serviceFunction.h"

#include "usbHIDInterface.h"
#include "usb_user_setings.h"

#define ADC_REZ_SIZE  (56 * 2)
#define EP_N           1
uint16_t adcRezBuff[ADC_REZ_SIZE];

MesPeriod timeAdc;


bool TX_BUSY_F = false;
bool RX_F = false;
bool TX_F = false;

typedef enum {
    SEND_DATA,
    RUN_CLOCKWISE,
    RUN_COUNTERCLOCKWISE,
    STOP,
} COMMAND_LIST;


#pragma pack(push, 1)
#define PROTOCOL_BUFF_SIZE ((64 - 4 - 4) / 2)
typedef struct SendDataCommandDesc{
    uint32_t headr;
    uint32_t size;
    uint16_t buff[PROTOCOL_BUFF_SIZE];
} PSendDataCommand;

typedef union {
    PSendDataCommand command;
    uint8_t buff[sizeof(PSendDataCommand)];
} SendDataCommand ;
#pragma pack(pop)

SendDataCommand dataCommand = {
    .command = {
        .headr = SEND_DATA,
    }
};

void getAdcRez(uint16_t rez[])
{
    addTimeMes(&timeAdc);
    /**copy Data to USB Buffer*/
    if(TX_F) {
        return;
    }
    TX_F = true;
    dataCommand.command.size = sizeof(dataCommand.buff);
    memcpy(dataCommand.command.buff, rez, sizeof(dataCommand.command.buff));
}

void usbHIDRxCB(uint8_t epNumber, uint8_t numRx, uint8_t* rxData)
{
}

void usbHIDTxCompleteCB(void)
{
    TX_BUSY_F = false;
}


void txCompleteCB(void)
{
    TX_BUSY_F = false;
}

int main(void)
{

    usbHIDInit();
    measurementInit(getAdcRez, adcRezBuff, ADC_REZ_SIZE);
    usbHIDAddRxCB(usbHIDRxCB);
    usbHIDAddTxCompleteCB(txCompleteCB);

    //initSysTic();
    measurementStart();
    //servoControlStart(20, CLOCKWISE);
    while(1)
    {
        if(RX_F) {

        };
        if(TX_F) {
            if( usbHIDEPIsReadyToTx(EP_01) ) {
                usbHIDTx(EP_01, dataCommand.buff, sizeof(SendDataCommand));
            }
            TX_F      = false;
            TX_BUSY_F = true;
        }
    }
}

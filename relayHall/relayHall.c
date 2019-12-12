#include "stdint.h"
#include "stdbool.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "relayHall.h"

static const struct {
    GPIO_TypeDef *port;
    uint32_t      portBuss;
    uint16_t      pin;
} relayPinList[] = {
    [0] = {
        .port      = GPIOB,
        .portBuss  = RCC_APB2ENR_IOPBEN,
        .pin       = GPIO_Pin_4,
        },
};

static bool isGpioInit = false;

static void initGpio(void) {
    GPIO_InitTypeDef gpioInit;
    for(uint16_t k = 0; k < sizeof(relayPinList) / sizeof(relayPinList[0]); k++) {
        RCC_APB2PeriphClockCmd(relayPinList[k].portBuss, ENABLE);
        gpioInit.GPIO_Pin   = relayPinList[k].pin;
        gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
        gpioInit.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(relayPinList[k].port, &gpioInit);
    }
    isGpioInit = true;
}

bool relayOn(uint16_t relayNumber)
{
    if(relayNumber >= sizeof(relayPinList) / sizeof(relayPinList[0])) {
        return false;
    }
    if(!isGpioInit) {
        initGpio();
    }
    GPIO_WriteBit(relayPinList[relayNumber].port, relayPinList[relayNumber].pin, Bit_SET);
    return true;
}

bool relayOff(uint16_t relayNumber)
{
    if(relayNumber >= sizeof(relayPinList) / sizeof(relayPinList[0])) {
        return false;
    }
    if(!isGpioInit) {
        initGpio();
    }
    GPIO_WriteBit(relayPinList[relayNumber].port, relayPinList[relayNumber].pin, Bit_RESET);
    return true;
}

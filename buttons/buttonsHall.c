#include "stdint.h"
#include "stdbool.h"

#include "buttonsHall.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

static const struct {
    GPIO_TypeDef *port;
    uint32_t      portBuss;
    uint16_t      pin;
} buttonsPinList[] = {
    [0] = {
        .port      = GPIOB,
        .portBuss  = RCC_APB2ENR_IOPBEN,
        .pin       = GPIO_Pin_4,
        },
    [1] = {
        .port      = GPIOB,
        .portBuss  = RCC_APB2ENR_IOPBEN,
        .pin       = GPIO_Pin_5,
        },
};

static bool isGpioInit = false;

static void initGpio(void) {
    GPIO_InitTypeDef gpioInit;
    for(uint16_t k = 0; k < sizeof(buttonsPinList) / sizeof(buttonsPinList[0]); k++) {
        RCC_APB2PeriphClockCmd(buttonsPinList[k].portBuss, ENABLE);
        gpioInit.GPIO_Pin   = buttonsPinList[k].pin;
        gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
        gpioInit.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_Init(buttonsPinList[k].port, &gpioInit);
    }
    isGpioInit = true;
}

bool readButton(uint16_t buttonNumber, bool *rezRead)
{
    if(buttonNumber >= sizeof(buttonsPinList) / sizeof(buttonsPinList[0])) {
        return false;
    }
    if(!isGpioInit) {
        initGpio();
    }
    *rezRead = !GPIO_ReadInputDataBit(buttonsPinList[buttonNumber].port,
                                     buttonsPinList[buttonNumber].pin);
    return true;
}


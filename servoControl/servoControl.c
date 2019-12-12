#include "stdint.h"

#include "stm32f10x_GPIO.h"
#include "stm32f10x_TIM.h"
#include "stm32f10x_RCC.h"
#include "stm32f10x.h"

#include "servoControl.h"

#define SERVO_TIM         TIM1
#define SERVO_ENABLE_TIM  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 ,ENABLE);
#define SERVO_FRQ         50
#define TIM_ARP           10000
#define PWM_INIT_TIME_100us  20
#define PWM_CLOCKWISE_100us        1
#define PWM_COUNTERCLOCKWISE_100us 20

#define SERVO_PORT        GPIOA
#define SERVO_PIN         GPIO_Pin_8
#define SERVO_ENABLE_GPIO RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

static timerCB timerUpCB = NULL;

static uint16_t getOCValue(SERVO_CONTROL_DIRECTION direction)
{
    switch(direction) {
        case CLOCKWISE:
            return (TIM_ARP * PWM_CLOCKWISE_100us * SERVO_FRQ / 10000);
        case COUNTERCLOCKWISE:
            return (TIM_ARP * PWM_COUNTERCLOCKWISE_100us * SERVO_FRQ / 10000);
        default:
            return 0;
    }
}

void servoControlInit(timerCB cbIn)
{
    timerUpCB = cbIn;
    RCC_ClocksTypeDef clock;
    GPIO_InitTypeDef initGpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    SERVO_ENABLE_GPIO;

    initGpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    initGpio.GPIO_Pin   = SERVO_PIN;
    initGpio.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(SERVO_PORT, &initGpio);

    TIM_OCInitTypeDef       pwmInit;
    TIM_TimeBaseInitTypeDef baseInit;

    RCC_GetClocksFreq(&clock);
    SERVO_ENABLE_TIM;
    TIM_TimeBaseStructInit(&baseInit);
    baseInit.TIM_ClockDivision     = TIM_CKD_DIV1;
    baseInit.TIM_CounterMode       = TIM_CounterMode_Up;
    baseInit.TIM_Period            = TIM_ARP;
    baseInit.TIM_Prescaler         = clock.PCLK2_Frequency * 2 / (TIM_ARP * SERVO_FRQ) - 1;
    baseInit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(SERVO_TIM, &baseInit);

    TIM_OCStructInit(&pwmInit);
    pwmInit.TIM_OCMode       = TIM_OCMode_PWM1;
    pwmInit.TIM_OutputState  = TIM_OutputState_Enable;
    pwmInit.TIM_OutputNState = TIM_OutputNState_Enable;
    pwmInit.TIM_Pulse        = TIM_ARP * PWM_INIT_TIME_100us * SERVO_FRQ / 1000000;
    pwmInit.TIM_OCPolarity   = TIM_OCPolarity_High;
    pwmInit.TIM_OCNPolarity  = TIM_OCNPolarity_High;
    pwmInit.TIM_OCIdleState  = TIM_OCNIdleState_Reset;
    pwmInit.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(SERVO_TIM, &pwmInit);
    TIM_OC1PreloadConfig(SERVO_TIM, TIM_OCPreload_Enable);
    TIM_GenerateEvent(SERVO_TIM, TIM_EventSource_Update);
    TIM_CtrlPWMOutputs(SERVO_TIM, ENABLE);
    TIM_ITConfig(SERVO_TIM, TIM_IT_Update, ENABLE);
    //enable DMA request
    //TIM_DMACmd(SERVO_TIM, TIM_DMA_CC1, ENABLE);
    NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void TIM1_UP_IRQHandler(void)
{
    TIM_ClearITPendingBit(SERVO_TIM, TIM_IT_Update);
    if(timerUpCB) {
        timerUpCB();
    }
}

void servoControlStart(uint32_t speed, SERVO_CONTROL_DIRECTION direction)
{
    TIM_SetCompare1(SERVO_TIM, getOCValue(direction));
    TIM_Cmd(SERVO_TIM, ENABLE);
}

void servoControlSetSpeed(uint32_t speed) {
    TIM_SetCompare1(SERVO_TIM, speed);
}

void servoControlStop(void)
{
    TIM_Cmd(SERVO_TIM, DISABLE);
}

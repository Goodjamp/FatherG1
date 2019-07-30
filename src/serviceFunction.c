#include "stdint.h"

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"

#include "serviceFunction.h"

volatile static uint32_t sysTimeCnt = 0;
#define SYS_TIMER   TIM2

void initSysTic(void)
{
    #define TICK_VAL_us 10
    RCC_ClocksTypeDef rccClocks;
    RCC_GetClocksFreq(&rccClocks);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef timeBaseInitStruct = {
        .TIM_Prescaler     = (rccClocks.PCLK1_Frequency / 1000000) * 2 - 1,
        .TIM_CounterMode   = TIM_CounterMode_Up,
        .TIM_Period        = TICK_VAL_us,
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };
    TIM_TimeBaseInit(SYS_TIMER, &timeBaseInitStruct);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM_ITConfig(SYS_TIMER, TIM_IT_Update, ENABLE);
    TIM_Cmd(SYS_TIMER, ENABLE);
}

void TIM2_IRQHandler(void) {
    TIM_ClearITPendingBit(SYS_TIMER, TIM_IT_Update);
    sysTimeCnt++;
}

uint32_t getSysTime(void)
{
    return sysTimeCnt;
}

void addTimeMes(MesPeriod *mesPeriod)
{
    mesPeriod->timeMes[mesPeriod->k++] = getSysTime() - mesPeriod->prevTime;
    mesPeriod->prevTime = sysTimeCnt;
    if(mesPeriod->k >= REZ_CNT) {
        mesPeriod->k = 0;
    }
}

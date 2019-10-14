#include "stdint.h"
#include "stddef.h"

#include "STM32F10x.h"
#include "stm32f10x_RCC.h"
#include "stm32f10x_DMA.h"
#include "stm32f10x_ADC.h"

#include "measurement.h"

/***Total channel quantity***/
#define CH_CNT           7
/***ADC initializations  definitions***/
#define SEL_ADC          ADC1
#define SEL_ADC_ENABLE   RCC_APB2Periph_ADC1
/***DMA initializations  definitions***/
#define SEL_DMA_CH       DMA1_Channel1
#define SEL_DMA_CH_IT    DMA1_Channel1_IRQn
#define SEL_DMA_ENABLE   RCC_AHBPeriph_DMA1
#define SEL_DMA_CB       DMA1_Channel1_IRQHandler
#define SEL_DMA_CH_IT_HT DMA1_FLAG_HT1
#define SEL_DMA_CH_IT_TC DMA1_FLAG_TC1

static const struct {
    struct {
        uint8_t      channel;
        uint8_t      chPos;
        uint8_t      sampleTime;
    } adcSettings;
    struct {
        uint32_t     enableCmd;
        GPIO_TypeDef *port;
        uint16_t     pin;
    } gpioSettings;
} adcChSettings[CH_CNT] = {
    { /***Ch 0 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_1,
                         .chPos   = 1,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_1,
                         }
    },
    { /***Ch 1 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_2,
                         .chPos  = 2,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_2,
                         }
    },
    { /***Ch 2 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_3,
                         .chPos  = 3,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_3,
                         }
    },
    { /***Ch 3 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_4,
                         .chPos  = 4,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_4,
                         }
    },
    { /***Ch 4 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_5,
                         .chPos  = 5,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_5,
                         }
    },
    { /***Ch 5 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_6,
                         .chPos  = 6,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_6,
                         }
    },
    { /***Ch 6 settings***/
         .adcSettings = {
                         .channel = ADC_Channel_7,
                         .chPos  = 7,
                         .sampleTime = ADC_SampleTime_239Cycles5,
                        },
         .gpioSettings = {
                         .enableCmd = RCC_APB2Periph_GPIOA,
                         .port      = GPIOA,
                         .pin       = GPIO_Pin_7,
                         }
    },
};

static BufferReadCB dmaCb = NULL;
static uint16_t    *beginBuff;
static uint16_t    *middleBuff;
static uint16_t    dmaBufferSize;

void measurementInit(BufferReadCB bufferReadyCB, uint16_t buffer[], uint32_t bufferSize)
{
    GPIO_InitTypeDef chPosit = {
        .GPIO_Speed = GPIO_Speed_10MHz,
        .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    };
    dmaCb      = bufferReadyCB;
    beginBuff  = buffer;
    dmaBufferSize = bufferSize;
    middleBuff = buffer + bufferSize / 2;
    /***GAFIO config***/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /***GPIO config***/

    for(uint32_t k = 0; k < CH_CNT; k++) {
        chPosit.GPIO_Pin = adcChSettings[k].gpioSettings.pin;
        RCC_APB2PeriphClockCmd(adcChSettings[k].gpioSettings.enableCmd, ENABLE);
        GPIO_Init(adcChSettings[k].gpioSettings.port, &chPosit);
    }
    /***ADC config***/
    ADC_InitTypeDef adcInitStruct;
    RCC_APB2PeriphClockCmd(SEL_ADC_ENABLE, ENABLE);
    ADC_Cmd(SEL_ADC, ENABLE);
    ADC_StructInit(&adcInitStruct);
    adcInitStruct.ADC_Mode               = ADC_Mode_Independent;
    adcInitStruct.ADC_ContinuousConvMode = ENABLE;
    adcInitStruct.ADC_ScanConvMode       = ENABLE;
    adcInitStruct.ADC_NbrOfChannel       = CH_CNT;
    adcInitStruct.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adcInitStruct.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_Init(SEL_ADC, &adcInitStruct);
    for(uint8_t k = 0; k < CH_CNT; k++) {
        ADC_RegularChannelConfig(SEL_ADC,
                                 adcChSettings[k].adcSettings.channel,
                                 adcChSettings[k].adcSettings.chPos,
                                 adcChSettings[k].adcSettings.sampleTime);
    }
    ADC_DMACmd(SEL_ADC, ENABLE);
    /***DMA config***/
    DMA_InitTypeDef dmaInitStruct;
    RCC_AHBPeriphClockCmd(SEL_DMA_ENABLE, ENABLE);
    DMA_StructInit(&dmaInitStruct);
    dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&SEL_ADC->DR;
    dmaInitStruct.DMA_MemoryBaseAddr     = (uint32_t)buffer ;
    dmaInitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dmaInitStruct.DMA_BufferSize         = dmaBufferSize;
    dmaInitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dmaInitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmaInitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dmaInitStruct.DMA_Mode               = DMA_Mode_Circular;
    dmaInitStruct.DMA_Priority           = DMA_Priority_High;
    dmaInitStruct.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(SEL_DMA_CH, &dmaInitStruct);
    DMA_ITConfig(SEL_DMA_CH, DMA_IT_HT, ENABLE);
    DMA_ITConfig(SEL_DMA_CH, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(SEL_DMA_CH_IT);

    ADC_Cmd(SEL_ADC, ENABLE);
}

void SEL_DMA_CB(void)
{
    if(DMA_GetITStatus(SEL_DMA_CH_IT_HT)) {
        DMA_ClearFlag(SEL_DMA_CH_IT_HT);
        if(dmaCb) {
            dmaCb(beginBuff);
        }
    } else if(DMA_GetITStatus(SEL_DMA_CH_IT_TC)) {
        DMA_ClearFlag(SEL_DMA_CH_IT_TC);
         if(dmaCb) {
            dmaCb(middleBuff);
        }
    }
}

void measurementStart(void)
{
    DMA_SetCurrDataCounter(SEL_DMA_CH, dmaBufferSize);
    DMA_Cmd(SEL_DMA_CH, ENABLE);
    ADC_Cmd(SEL_ADC, ENABLE);
    ADC_SoftwareStartConvCmd(SEL_ADC, ENABLE);
}

void measurementStop(void)
{
    DMA_Cmd(SEL_DMA_CH, DISABLE);
    ADC_Cmd(SEL_ADC, DISABLE);
    ADC_SoftwareStartConvCmd(SEL_ADC, DISABLE);
}

#include "stdint.h"
#include "stdbool.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"

#include "OneWireHAL.h"
/**One wire line bit order**/
#define MSB_FIRST
/**GPIO initializations definitions**/
#define GPIO_PORT               GPIOA
#define GPIO_PIN                GPIO_Pin_0
#define DATA_BIT                0
#define GPIO_BUS_SOURCES        RCC_APB2ENR_IOPAEN
/**TIMER initializations definitions**/
//CCR1 channel - is use for generate time slots
//CCR2 channel - is use for read data from GPIO
#define TIMER_DEF               TIM2
#define TIMER_ENABLE            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
#define TIMER_CLOCK_SOURCE      rccClocks.PCLK2_Frequency
#define TIMER_PWM_INIT          TIM_OC1Init
#define TIMER_PRELODE_INIT      TIM_OC1PreloadConfig
#define TIM_SET_WRITE_SLOT      TIM_SetCompare
#define TIMER_DMA_SLOT_CHANNEL  TIM_DMA_CC1
#define TIMER_DMA_READ_CHANNEL  TIM_DMA_CC2
#define TIM_SET_READ_SLOT       TIM_SetCompare2
#define TIMER_PULSES_PER_US     8
/**DMA initializations definitions**/
//  - tim2_UP  dma1_CH2
//  - tim2_CH2 dma1_CH7
#define DMA_ENABLE                            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#define DMA_TIME_SLOT_CHANNEL                 DMA1_Channel5
#define DMA_TIME_SLOT_IRQ                     DMA1_Channel5_IRQn
#define DMA_TRANSACTION_COMPLETE_IRQ_HANDLER  DMA1_Channel5_IRQHandler
#define DMA_TRANSACTION_COMPLETE_FLAG         DMA1_IT_TC5

#define DMA_RECEIVE_CHANNEL     DMA1_Channel7
/**general time slots**/
#define TIME_SLOT_US            64
#define RECOVERY_PULS_US        10
/**total transfer period (data transfer plus recovery impulse)**/
#define DATA_TIME_PERIOD_US           (TIME_SLOT_US + RECOVERY_PULS_US)
/**one wire write time slots**/
#define MASTER_WRITE_1_US             15
#define MASTER_WRITE_0_US             TIME_SLOT_US
/**one wire read time slots**/
#define MASTER_READ_US                8
#define MASTER_READE_RELEASE_PULS_US  1
/**reset time slots**/
#define MASTER_RESET_US               500
#define MASTER_RESET_RECOVERY_PULS_US 25
#define SLAVE_RESET_RECOVERY_PULS_US  500
#define MASTER_RESET_RECOVERY_READ_US 40
/**transmit and receive buffers sizes**/
#define TIM_CNT_BUFFER_SIZE_BYTES     9
#define RECEIVE_BUFFER_SIZE_BYTES     9
/**Selected firs MSB or firs LSB operation**/
#define LSB_FIRST
#ifdef LSB_FIRST
    #define BIT_MARKER (uint8_t)0b1
    #define SHIFT_BIT_MARKER(X) (X <<= 1)
#else
    #define BIT_MARKER (uint8_t)0b10000000
    #define SHIFT_BIT_MARKER(X) (X >>= 1)
#endif


static const uint16_t masterResetPulsWith = TIMER_PULSES_PER_US * MASTER_RESET_US;
static const uint16_t masterResetPeriod   = TIMER_PULSES_PER_US * (MASTER_RESET_US
                                            + MASTER_RESET_RECOVERY_PULS_US
                                            + SLAVE_RESET_RECOVERY_PULS_US);
static const uint16_t masterResetRead    = TIMER_PULSES_PER_US * (MASTER_RESET_US
                                            + MASTER_RESET_RECOVERY_PULS_US
                                            + MASTER_RESET_RECOVERY_READ_US);
static const uint16_t masterSendCnt_1   = TIMER_PULSES_PER_US * MASTER_WRITE_1_US;
static const uint16_t masterSendCnt_0   = TIMER_PULSES_PER_US * MASTER_WRITE_0_US;
static const uint16_t masterReadSlot    = TIMER_PULSES_PER_US * MASTER_READE_RELEASE_PULS_US;
static const uint16_t masterRxTxPeriod  = TIMER_PULSES_PER_US * DATA_TIME_PERIOD_US;
//+ 2 is one more slot for set HIGH level on the pin after complete transaction !!!!
static uint16_t timCntBuffer[TIM_CNT_BUFFER_SIZE_BYTES   * 8 + 2];
static uint16_t receiveBuffer[RECEIVE_BUFFER_SIZE_BYTES * 8 + 2];
static volatile bool isOneWireInit = false;
static volatile OneWireCB oneWireCBLocal;
static DMA_InitTypeDef dmaTimeSlotInitStruct;
static DMA_InitTypeDef dmaReceiveInitStruct;
static volatile OneWireState oneWireState;

static void oneWirePeripheralInit(void)
{
    isOneWireInit = true;

    /**Timer initializations**/
    RCC_ClocksTypeDef       rccClocks;
    TIM_TimeBaseInitTypeDef baseInit;
    TIM_OCInitTypeDef       pwmInit;
    RCC_GetClocksFreq(&rccClocks);
    TIMER_ENABLE;
    // set timer pulse period 0.5 us
    baseInit.TIM_Prescaler         = TIMER_CLOCK_SOURCE / (TIMER_PULSES_PER_US * 1000000);
    baseInit.TIM_Period            = masterRxTxPeriod;
    baseInit.TIM_CounterMode       = TIM_CounterMode_Up;
    baseInit.TIM_RepetitionCounter = 0;
    baseInit.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIMER_DEF, &baseInit);
    TIM_ARRPreloadConfig(TIMER_DEF, ENABLE);
    // configuration CCR for generate time slot
    TIM_OCStructInit(&pwmInit);
    pwmInit.TIM_OCMode       = TIM_OCMode_PWM1;
    pwmInit.TIM_OutputState  = TIM_OutputState_Enable;
    pwmInit.TIM_OutputNState = TIM_OutputNState_Enable;
    pwmInit.TIM_Pulse        = 0;
    pwmInit.TIM_OCPolarity   = TIM_OCPolarity_Low;
    pwmInit.TIM_OCNPolarity  = TIM_OCNPolarity_Low;
    pwmInit.TIM_OCIdleState  = TIM_OCNIdleState_Reset;
    pwmInit.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIMER_PWM_INIT(TIMER_DEF, &pwmInit);
    TIMER_PRELODE_INIT(TIMER_DEF, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIMER_DEF, ENABLE);

    /**GPIO initializations**/
    GPIO_InitTypeDef gpioBaseInit;
    RCC_APB2PeriphClockCmd(GPIO_BUS_SOURCES, ENABLE);
    gpioBaseInit.GPIO_Pin   = GPIO_PIN;
    gpioBaseInit.GPIO_Speed = GPIO_Speed_10MHz;
    gpioBaseInit.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init(GPIO_PORT, &gpioBaseInit);

    /**DMA initializations**/
    DMA_ENABLE;
    // Init DMA channel for create time slots: data transmit from memory to DMA every UP event
    dmaTimeSlotInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&TIM2->CCR1;
    dmaTimeSlotInitStruct.DMA_MemoryBaseAddr     = (uint32_t)timCntBuffer;
    dmaTimeSlotInitStruct.DMA_DIR                = DMA_DIR_PeripheralDST;
    dmaTimeSlotInitStruct.DMA_BufferSize         = 0;
    dmaTimeSlotInitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dmaTimeSlotInitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dmaTimeSlotInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmaTimeSlotInitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dmaTimeSlotInitStruct.DMA_Mode               = DMA_Mode_Normal;
    dmaTimeSlotInitStruct.DMA_Priority           = DMA_Priority_High;
    dmaTimeSlotInitStruct.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA_TIME_SLOT_CHANNEL, &dmaTimeSlotInitStruct);
    DMA_ITConfig(DMA_TIME_SLOT_CHANNEL, DMA_IT_TC, ENABLE);
    NVIC_EnableIRQ(DMA_TIME_SLOT_IRQ);
    // Init DMA channel for read data from GPIO: data transmit gpio data reg to memory CCR event
    dmaReceiveInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->IDR;
    dmaReceiveInitStruct.DMA_MemoryBaseAddr     = (uint32_t)receiveBuffer;
    dmaReceiveInitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dmaReceiveInitStruct.DMA_BufferSize         = 0;
    dmaReceiveInitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dmaReceiveInitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dmaReceiveInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmaReceiveInitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dmaReceiveInitStruct.DMA_Mode               = DMA_Mode_Normal;
    dmaReceiveInitStruct.DMA_Priority           = DMA_Priority_High;
    dmaReceiveInitStruct.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA_RECEIVE_CHANNEL, &dmaReceiveInitStruct);
}

void DMA_TRANSACTION_COMPLETE_IRQ_HANDLER(void)
{
    DMA_ClearITPendingBit(DMA_TRANSACTION_COMPLETE_FLAG);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_READ_CHANNEL, DISABLE);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_SLOT_CHANNEL, DISABLE);
    TIM_Cmd(TIMER_DEF, DISABLE);

    oneWireState = ONE_WIRE_NORMAL;
    if(!oneWireCBLocal) {
        return;
    }
    oneWireCBLocal(true);
}

void oneWireInit(OneWireCB oneWireCB)
{
    oneWireCBLocal = oneWireCB;
}

OneWireState oneWireReset(void)
{
    if(oneWireState == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    oneWireState = ONE_WIRE_BUSY;
    if(!isOneWireInit) {
        oneWirePeripheralInit();
    }
    receiveBuffer[0] = 0;
    timCntBuffer[0]  = masterResetPulsWith;
    /*
    Next string is use for set HIGH level on the pin after complete transaction
    AND for generate DMA event ONLY after shift all data to timer CCR!!!!
     */
    timCntBuffer[1] = timCntBuffer[2] = 0;
    /***prepare DMA, time slot channel**/
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, DISABLE);
    DMA_Init(DMA_TIME_SLOT_CHANNEL, &dmaTimeSlotInitStruct);
    DMA_SetCurrDataCounter(DMA_TIME_SLOT_CHANNEL, 3);
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, ENABLE);
    /***prepare DMA, receive channel**/
    DMA_Cmd(DMA_RECEIVE_CHANNEL, DISABLE);
    DMA_Init(DMA_RECEIVE_CHANNEL, &dmaReceiveInitStruct);
    DMA_SetCurrDataCounter(DMA_RECEIVE_CHANNEL, 1);
    DMA_Cmd(DMA_RECEIVE_CHANNEL, ENABLE);
    /***prepare Timer**/
    // configuration CCR generate DMA request for read GPIO
    TIM_Cmd(TIMER_DEF, DISABLE);
    TIM_SetAutoreload(TIMER_DEF, masterResetPeriod);
    TIM_SET_READ_SLOT(TIMER_DEF, masterResetRead);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_SLOT_CHANNEL, ENABLE);
    TIM_GenerateEvent(TIMER_DEF, TIM_IT_CC1);
    TIM_GenerateEvent(TIMER_DEF, TIM_IT_Update);
    TIM_ClearITPendingBit(TIMER_DEF, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 );
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_READ_CHANNEL, ENABLE);
    TIM_Cmd(TIMER_DEF, ENABLE);
    return ONE_WIRE_NORMAL;
}

OneWireState oneWireSend(uint8_t data[], uint32_t dataSize)
{
    if(oneWireState == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    oneWireState = ONE_WIRE_BUSY;
    if(!isOneWireInit) {
        oneWirePeripheralInit();
    }
    uint8_t bitMarker;
    uint8_t bitCnt = 0;
    for(uint32_t k = 0; k < dataSize; k++) {
        bitMarker = BIT_MARKER;
        do {
            timCntBuffer[bitCnt++] = (data[k] & bitMarker) ?
                                      (masterSendCnt_1)
                                      : (masterSendCnt_0);
        }while(SHIFT_BIT_MARKER(bitMarker));
    }
    /*
    Next string is use for set HIGH level on the pin after complete transaction
    AND for generate DMA event ONLY after shift all data to timer CCR!!!!
     */
    timCntBuffer[bitCnt++] = 0;
    //timCntBuffer[bitCnt++] = 0;
    /***prepare DMA**/
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, DISABLE);
    DMA_Init(DMA_TIME_SLOT_CHANNEL, &dmaTimeSlotInitStruct);
    DMA_SetCurrDataCounter(DMA_TIME_SLOT_CHANNEL, bitCnt);
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, ENABLE);
    /***prepare Timer**/
    TIM_Cmd(TIMER_DEF, DISABLE);
    TIM_SetAutoreload(TIMER_DEF, masterRxTxPeriod);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_SLOT_CHANNEL, ENABLE);
    TIM_GenerateEvent(TIMER_DEF, TIM_IT_CC1);
    TIM_ClearITPendingBit(TIMER_DEF, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 );
    TIM_Cmd(TIMER_DEF, ENABLE);
    return ONE_WIRE_NORMAL;
}

OneWireState oneWireReceive(uint32_t dataSize)
{
    if(oneWireState == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    oneWireState = ONE_WIRE_BUSY;
    if(!isOneWireInit) {
        oneWirePeripheralInit();
    }
    uint8_t bitMarker;
    uint8_t bitCnt = 0;
    while(dataSize--) {
        bitMarker = BIT_MARKER;
        do {
            receiveBuffer[bitCnt]  = 0;
            timCntBuffer[bitCnt++] = masterReadSlot;
        } while(SHIFT_BIT_MARKER(bitMarker));
    }
    /*
    Next string is use for set HIGH level on the pin after complete transaction
    AND for generate DMA event ONLY after shift all data to timer CCR!!!!
     */
    timCntBuffer[bitCnt++] = 0;
    timCntBuffer[bitCnt++] = 0;
    /***prepare DMA, time slot channel**/
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, DISABLE);
    DMA_Init(DMA_TIME_SLOT_CHANNEL, &dmaTimeSlotInitStruct);
    DMA_SetCurrDataCounter(DMA_TIME_SLOT_CHANNEL, bitCnt);
    DMA_Cmd(DMA_TIME_SLOT_CHANNEL, ENABLE);
    /***prepare DMA, receive channel**/
    DMA_Cmd(DMA_RECEIVE_CHANNEL, DISABLE);
    DMA_Init(DMA_RECEIVE_CHANNEL, &dmaReceiveInitStruct);
    DMA_SetCurrDataCounter(DMA_RECEIVE_CHANNEL, bitCnt - 2);
    DMA_Cmd(DMA_RECEIVE_CHANNEL, ENABLE);
    /***prepare Timer**/
    TIM_Cmd(TIMER_DEF, DISABLE);
    // configuration CCR generate DMA request for read GPIO
    TIM_SET_READ_SLOT(TIMER_DEF, TIMER_PULSES_PER_US * MASTER_READ_US);
    TIM_SetAutoreload(TIMER_DEF, masterRxTxPeriod);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_SLOT_CHANNEL, ENABLE);
    TIM_GenerateEvent(TIMER_DEF, TIM_IT_CC1);
    TIM_GenerateEvent(TIMER_DEF, TIM_IT_Update);
    TIM_ClearITPendingBit(TIMER_DEF, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_Update);
    TIM_DMACmd(TIMER_DEF, TIMER_DMA_READ_CHANNEL, ENABLE);
    TIM_Cmd(TIMER_DEF, ENABLE);
    return ONE_WIRE_NORMAL;
}

void oneWireGetRxData(uint8_t buff[], uint32_t dataSize)
{
    uint8_t bitMarker;
    uint32_t bitCnt = 0;
    for(uint8_t k = 0; k < dataSize; k++) {
        bitMarker = BIT_MARKER;
        buff[k] = 0;
        do {
            buff[k] |= (receiveBuffer[bitCnt++] & (1 << DATA_BIT)) ? (bitMarker) : (0);
        } while(SHIFT_BIT_MARKER(bitMarker));
    }
}

OneWireState oneWireResetBloking(void)
{
    if(oneWireReset() == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    while(oneWireState == ONE_WIRE_BUSY) {};
    return (receiveBuffer[0] & (1 << DATA_BIT)) ?
               ONE_WIRE_RESET_ERROR :
               ONE_WIRE_NORMAL;
}

OneWireState oneWireSendBloking(uint8_t data[], uint32_t dataSize)
{
    if(oneWireSend(data, dataSize) == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    while(oneWireState == ONE_WIRE_BUSY) {};
    return ONE_WIRE_NORMAL;
}
OneWireState oneWireReceiveBloking(uint8_t data[], uint32_t dataSize)
{
    if(oneWireReceive(dataSize) == ONE_WIRE_BUSY) {
        return ONE_WIRE_BUSY;
    }
    while(oneWireState == ONE_WIRE_BUSY) {};
    oneWireGetRxData(data, dataSize);
    return ONE_WIRE_NORMAL;
}

OneWireState oneWireGetState(void)
{
    return oneWireState;
}

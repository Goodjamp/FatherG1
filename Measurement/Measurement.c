#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

#include "misc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "FreeRTOSConfig.h"

#include "stm32f10x_conf.h"
#include "stm32f10x_gpio.h"

#include "adcHal.h"
#include "serviceFunction.h"
#include "dspAPI.h"
#include "relayHall.h"
#include "Measurement.h"
#include "generalProtocol.h"

#define RELAY_NUMBER       0

#define CHANNEL_CNT        0x7
#define ADC_REZ_CNT        4
#define ADC_REZ_BUFF_SIZE  (CHANNEL_CNT * ADC_REZ_CNT * 2)
#define EP_N               1

/*FIR LPF settings*/
#define FILTER_ORDER        128
#define CUTOFF_HZ           20
#define MAX_DSP_VAL         4095
#define REZ_PROCESSING_SIZE 28
/*Describing event group bits*/
#define MES_SHORT_MES_FLAG  (1 << 0)
#define MES_START_MES_FLAG  (1 << 1)
#define MES_STOP_MES_FLAG   (1 << 2)
#define MES_ADC_READY_FLAG  (1 << 3)
#define MES_ALL_ACTION       (MES_SHORT_MES_FLAG | MES_START_MES_FLAG | MES_STOP_MES_FLAG | MES_ADC_READY_FLAG)

volatile static MeasShortCompliteCb shortCompliteCb;
volatile static MeasContinuousCompleteCb continuousCompleteCb;
static FiltrationHandler filtrationHandler;
static FilterConfig firFilterConfig = {
    .type = LOW_PATH,
    .calcType = SINC,
    .windowType = BLACKMAN,
    .df = CUTOFF_HZ,
    .q  = FILTER_ORDER,
};
static int32_t   filterCoeff[FILTER_ORDER];
static int32_t   filteringBuff[FILTER_ORDER];
static uint16_t  adcRezBuff[ADC_REZ_BUFF_SIZE];
static bool      isMesDisabled = true;
static struct {
    bool     isReady;
    uint16_t buff[ADC_REZ_BUFF_SIZE / 2];
}adcRez;
struct {
    uint16_t buf[REZ_PROCESSING_SIZE];
    uint8_t  cnt;
} filtRezBuff;
static struct {
    uint32_t debouncePeriodMs;
    uint32_t measPeriodMs;
} measTimings = {
    .debouncePeriodMs = 100,
    .measPeriodMs     = 100
};
static EventGroupHandle_t mesEvent;
static bool mesStart = false;

static void adcCompletedCb(uint16_t rez[])
{
    BaseType_t isHightPrioryti;
    memcpy((uint8_t*)adcRez.buff, (uint8_t*)rez, sizeof(adcRez.buff));
    xEventGroupSetBitsFromISR(mesEvent, MES_ADC_READY_FLAG, &isHightPrioryti);
    /*
    if(isHightPrioryti == pdTRUE) {
       vTaskSwitchContext();
    }
    */
}

static void initDSP(void)
{
    /*
    8 - ADC clocking divider
    2 - ADC bus cloaking divider
    7 - ADC channel quantity
    */
    firFilterConfig.fs = (72000000)/(8 * 2 * (240 + 12) * 7);
    dspInitFiltr(&filtrationHandler, firFilterConfig, filterCoeff, filteringBuff, MAX_DSP_VAL);
}

static void rccConfig(void) {
    RCC_PCLK2Config(RCC_HCLK_Div2);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

static void measShort(void)
{
    volatile EventBits_t eventBits;
    uint32_t mesRez = 0;
    uint32_t reCznt = 0;
    measurementStop();
    relayOn(RELAY_NUMBER);
    //wait for complete debounce
    vTaskDelay(measTimings.debouncePeriodMs);
    //wait for complete transition process
    vTaskDelay(measTimings.measPeriodMs);
    //start measurement
    measurementStart();
    while(reCznt <= FILTER_ORDER) {
        eventBits = xEventGroupWaitBits(mesEvent, MES_ADC_READY_FLAG, pdTRUE, pdTRUE, portMAX_DELAY);
        if(!(eventBits &MES_ADC_READY_FLAG)) {
            continue;
        }
        for(uint8_t k = 0; k < ADC_REZ_CNT; k++) {
            mesRez += adcRez.buff[k * CHANNEL_CNT];
            //filtRezBuff.buf[filtRezBuff.cnt++] = adcRez.buff[k * CHANNEL_CNT];
            reCznt++;
        }
    }
    mesRez /= reCznt;
    measurementStop();
    relayOff(RELAY_NUMBER);
    if (shortCompliteCb) {
        shortCompliteCb(mesRez );
    }
}

void measSetMeasAction(MEAS_ACTION measAction)
{
    switch(measAction) {
    case MEAS_SHORT:
        xEventGroupSetBits(mesEvent, MES_SHORT_MES_FLAG);
        break;
    case MEAS_START:
        xEventGroupSetBits(mesEvent, MES_START_MES_FLAG);
        break;
    case MEAS_STOP:
        xEventGroupSetBits(mesEvent, MES_STOP_MES_FLAG);
        break;
    default: return;
    }
}

void measSetShortCompliteCb(MeasShortCompliteCb measShortCompliteCb)
{
    shortCompliteCb = measShortCompliteCb;
}

void measSetContinuousCompliteCb(MeasContinuousCompleteCb measContinuousCompleteCb)
{
    continuousCompleteCb = measContinuousCompleteCb;
}

void vMeasTask( void *pvParameters )
{
    EventBits_t eventBits;
    /*Read configuration of timing (debounse and measurement ) from Flash*/
    initDSP();
    rccConfig();
    uint32_t cnt = 72000;
    while(cnt-- > 2){}
    measurementInit(adcCompletedCb, adcRezBuff, ADC_REZ_BUFF_SIZE);
    rccConfig();
    rccConfig();
    /*create queue measurement action*/
    mesEvent = xEventGroupCreate();
    while(1) {
        eventBits = xEventGroupWaitBits(mesEvent, MES_ALL_ACTION, pdTRUE, pdFALSE, portMAX_DELAY);
        if(eventBits & MES_SHORT_MES_FLAG) {
            measShort();
        }
        if(eventBits & MES_START_MES_FLAG) {
            filtRezBuff.cnt = 0;
            measurementStart();
            mesStart = true;
        }
        if(eventBits & MES_STOP_MES_FLAG) {
            measurementStop();
            filtRezBuff.cnt = 0;
            mesStart = false;
            /*Clear ADC ready beat for avoid processing message */
            eventBits &= ~MES_ADC_READY_FLAG;
        }
        if(eventBits & MES_ADC_READY_FLAG) {
            if(!mesStart) {
                continue;
            }
            for(uint8_t k = 0; k < ADC_REZ_CNT; k++) {
                filtRezBuff.buf[filtRezBuff.cnt++] = adcRez.buff[k * CHANNEL_CNT];
            //adcRez.buff[k * CHANNEL_CNT];
                                                      //dspFiltration(&filtrationHandler,
                                                      //    (uint16_t)(adcRez.buff[k * CHANNEL_CNT]));
                if(filtRezBuff.cnt >= REZ_PROCESSING_SIZE) {
                    gpSendDataCommand((uint8_t*)filtRezBuff.buf,
                                      REZ_PROCESSING_SIZE * sizeof(int16_t),
                                      COMMAND_DATA_FLAG_16_BIT_SIZE,
                                      0);
                    filtRezBuff.cnt = 0;
                }
            }
        }
    }
}

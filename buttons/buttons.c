#include "stdint.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "buttons.h"

#define DEBOUNCE_CNT           3
#define POST_PRESS_TIMEOUT_CNT 25

const uint32_t pressTypeTimings[PRESS_TYPE_CNT] = {
    [PRESS_SHORT]     = 0,
    [PRESS_LONG]      = 25,
    [PRESS_LONG_LONG] = 75,
};

const ButtonActionDescription *buttonDescriptionLocal;
static uint32_t buttonsDescrSize;
static struct buttonProcessing {
    bool     use;
    bool     inProcessing;
    uint32_t debounceTime;
    uint32_t postPressTime;
    uint32_t pressTime;
    bool     pressType[PRESS_TYPE_CNT];
}buttonProcessing[BUTTON_CNT];
static ReadButtonCb readButton;


void buttonsInitButton(const ButtonActionDescription *buttonDescription,
                       uint32_t size,
                       ReadButtonCb readButtonCb)
{
    readButton = readButtonCb;
    buttonDescriptionLocal = buttonDescription;
    buttonsDescrSize = size;
    //clear previous state button configuration structure
    for(uint32_t k = 0; k < BUTTON_CNT; k++) {
        buttonProcessing[k].use = false;
        buttonProcessing[k].inProcessing = false;
        buttonProcessing[k].postPressTime = 0;
        for(uint32_t m = 0; m < PRESS_TYPE_CNT; m++) {
            buttonProcessing[k].pressType[m] = false;
        }
    }
    //update button configuration structure
    for(uint32_t k = 0; k < size; k++) {
        buttonProcessing[buttonDescription[k].buttonNumber].use = true;
        buttonProcessing[buttonDescription[k].buttonNumber].pressType[buttonDescription[k].pressType] = true;
    }
}

void buttonsCallCB(uint32_t button, PRESS_TYPE pressType)
{
    for(uint32_t k = 0; k < buttonsDescrSize; k++) {
        if((buttonDescriptionLocal[k].buttonNumber == button)
           && (buttonDescriptionLocal[k].pressType == pressType)) {
              buttonDescriptionLocal[k].buttonActionCb();
        }
    }
}

static bool buttonsPressTypeDetect(uint32_t button)
{
    if(buttonProcessing[button].inProcessing) {
        uint32_t k = PRESS_TYPE_CNT - 1;
        for(; !buttonProcessing[button].pressType[k]; k--) {
        }
        if(buttonProcessing[button].pressTime >= pressTypeTimings[k]) {
            buttonsCallCB(button, (PRESS_TYPE)k);
            return true;
        }
    } else {
        for(int32_t k = (PRESS_TYPE_CNT - 1); k >= 0; k--) {
            if(!buttonProcessing[button].pressType[k]) {
                buttonsCallCB(button, (PRESS_TYPE)k);
            continue;
            }
            if(buttonProcessing[button].pressTime >= pressTypeTimings[k]) {
                buttonsCallCB(button, (PRESS_TYPE)k);
                return true;
            }
        }
    }
    return false;
}

static void buttonsProcessing(void)
{
    bool buttonState;
    for(uint8_t k = 0; k < BUTTON_CNT; k++) {
        //processing buttons only from configuration structure
        if(!buttonProcessing[k].use) {
            continue;
        }
        if(buttonProcessing[k].postPressTime) {
            if(buttonProcessing[k].postPressTime++ < POST_PRESS_TIMEOUT_CNT) {
                continue;
            }
            buttonProcessing[k].postPressTime = 0;
        }
        // wait for debounce period
        if((buttonProcessing[k].debounceTime <= DEBOUNCE_CNT)
           && buttonProcessing[k].inProcessing) {
            buttonProcessing[k].debounceTime++;
            buttonProcessing[k].pressTime++;
            continue;
        }
        /*******Processing button pressing**************************/
        readButton(k, &buttonState);
        if(buttonState && !buttonProcessing[k].inProcessing) {
        //if first press detect - wait for debounce period
            buttonProcessing[k].inProcessing = true;
            buttonProcessing[k].pressTime++;
            if(buttonsPressTypeDetect(k)) {
                buttonProcessing[k].inProcessing = false;
                buttonProcessing[k].pressTime     = 0;
                buttonProcessing[k].postPressTime++;
                buttonProcessing[k].debounceTime = 0;
            }
            continue;
        } else if(!buttonState && !buttonProcessing[k].inProcessing) {
        // if no press detect and button don't processing now: do nothing
            continue;
        }else if(buttonState && buttonProcessing[k].inProcessing) {
        // if press detect AND button in processing now
            buttonProcessing[k].pressTime++;
            if(!buttonsPressTypeDetect(k)) {
                // still don't detect press type
                continue;
            }
            // press type was detected
            buttonProcessing[k].inProcessing = false;
            buttonProcessing[k].pressTime     = 0;
            buttonProcessing[k].postPressTime++;
            buttonProcessing[k].debounceTime = 0;
        } else if(!buttonState && buttonProcessing[k].inProcessing) {
        // if no press detect BUT button in processing now
            buttonProcessing[k].inProcessing = false;
            buttonsPressTypeDetect(k);
            buttonProcessing[k].pressTime = 0;
            buttonProcessing[k].postPressTime++;
            buttonProcessing[k].debounceTime = 0;
        }
    }
}

void vButtonsTask(void *pvParameters)
{
    while(1) {
        vTaskDelay(20);
        buttonsProcessing();
    }
}

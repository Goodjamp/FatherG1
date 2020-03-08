#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"

#include "stm32f10x_rcc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "Menu.h"
#include "userMenu.h"
#include "DisplayInterfaceInit.h"
#include "frameHall.h"
#include "buttons.h"
#include "buttonsHall.h"
#include "TemperatureSensorHAL.h"
#include "Measurement.h"
#include "bitmap.h"

void buttonShortCb0(void);
void buttonLongCb0(void);
void buttonShortCb1(void);
void buttonLongCb1(void);
void buttonShortCb2(void);
void buttonLongCb2(void);

static const ButtonActionDescription buttonActionDescription[] = {
    [0] = {
        .buttonActionCb = buttonShortCb0,
        .buttonNumber   = 0,
        .pressType      = PRESS_SHORT,
    },
    [1] = {
        .buttonActionCb = buttonLongCb0,
        .buttonNumber   = 0,
        .pressType      = PRESS_LONG,
    },
    [2] = {
        .buttonActionCb = buttonShortCb1,
        .buttonNumber   = 1,
        .pressType      = PRESS_SHORT,
    },
    [3] = {
        .buttonActionCb = buttonLongCb1,
        .buttonNumber   = 1,
        .pressType      = PRESS_LONG,
    },
    [4] = {
        .buttonActionCb = buttonShortCb2,
        .buttonNumber   = 2,
        .pressType      = PRESS_SHORT,
    },
    [5] = {
        .buttonActionCb = buttonLongCb2,
        .buttonNumber   = 2,
        .pressType      = PRESS_LONG,
    },
};

extern const Image buttonInstruct;
extern const Image buttonTemp;
extern const Image buttonMeas;
extern const Image buttonNext;
extern const Image buttonEntr;
extern const Image relayConnection;


PRESS_TYPE pressType = PRESS_SHORT;
uint32_t button = 0;
static FrameDescr   screenFrame;
static MenuItem menuItemRoot;
static MenuItem menuItemF1;
static MenuItem menuItemF2;
static MenuItem menuItemF3;
static MenuItem *menu = &menuItemRoot;
static QueueHandle_t  buttonEventQueue;
static TimerHandle_t  menuTimer;

uint8_t menuStr[40];
volatile static bool     isMeasComplete = false;
volatile static uint32_t measRez = 0;

static void measComplete(uint32_t rezMeasShort)
{
    isMeasComplete = true;
    measRez     = rezMeasShort;
}

void buttonShortCb0(void)
{
    uint8_t event = MENU_EVENT_BS_0;
    xQueueSend(buttonEventQueue, &event, 100);
}

void buttonLongCb0(void)
{
    uint8_t event = MENU_EVENT_BL_0;
    xQueueSend(buttonEventQueue, &event, 100);
}

void buttonShortCb1(void)
{
    uint8_t event = MENU_EVENT_BS_1;
    xQueueSend(buttonEventQueue, &event, 100);
}

void buttonLongCb1(void)
{
    uint8_t event = MENU_EVENT_BL_1;
    xQueueSend(buttonEventQueue, &event, 100);
}

void buttonShortCb2(void)
{
    uint8_t event = MENU_EVENT_BS_2;
    xQueueSend(buttonEventQueue, &event, 100);
}

void buttonLongCb2(void)
{
    uint8_t event = MENU_EVENT_BL_2;
    xQueueSend(buttonEventQueue, &event, 100);
}

void menuTimerCb( TimerHandle_t xTimer )
{
    uint8_t event = MENU_EVENT_TIMER;
    xQueueSend(buttonEventQueue, &event, 100);
}


void displayConfig(void)
{
    displayInterfaceInit();
    frameInit(&screenFrame, displayInterfaceGetFrameBuffer(), FRAME_HEIGHT, FRAME_WIDTH);
}

void buttonsConfig(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    buttonsInitButton(buttonActionDescription,
                      sizeof(buttonActionDescription) / sizeof(buttonActionDescription[0]),
                      readButton);
#define MAX_BUTTON_EVENTS 10
    buttonEventQueue = xQueueCreate(MAX_BUTTON_EVENTS, 1);
    menuTimer        = xTimerCreate( "menuTimer", 1000, pdTRUE, ( void * )0, menuTimerCb);
}

/*Root menu. Use for selected one sub menu and show generic data*/
MenuStatus menuRoot(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    frameClear(&screenFrame);
    frameSetPosition(&screenFrame, 0, 55);
    frameAddImage(&screenFrame, buttonMeas.image, buttonMeas.height, buttonMeas.width, true, (subMenCnt == 0) ? true : false);
    frameSetPosition(&screenFrame, 43, 55);
    frameAddImage(&screenFrame, buttonTemp.image, buttonTemp.height, buttonTemp.width, true, (subMenCnt == 1) ? true : false);
    frameSetPosition(&screenFrame, 86, 55);
    frameAddImage(&screenFrame, buttonInstruct.image, buttonInstruct.height, buttonInstruct.width, true, (subMenCnt == 2) ? true : false);
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        frameSetPosition(&screenFrame, 0, 2);
        sprintf(menuStr,"ROOT, s = %u", subMenCnt);
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 2) {
            subMenCnt = 0;
        }
        frameSetPosition(&screenFrame, 0, 2);
        sprintf(menuStr,"ROOT, s = %u", subMenCnt);
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_1:
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

/**Menu F1. Use for show temperature**/
MenuStatus menuF1(MenuEvent event, uint32_t *children)
{
    switch(event) {
    case MENU_EVENT_ENTER:
    case MENU_EVENT_TIMER:
    {
        /**Measurement and show temperature**/
        uint32_t temp;
        uint32_t temperarure;
        frameClear(&screenFrame);
        frameSetPosition(&screenFrame, 10, 2);
        frameAddString(&screenFrame, "TEMPERATURE", ARIAL_8PTS, false);
        if(temperatureGetTemperature(&temperarure)) {
            frameSetPosition(&screenFrame, 45, 22);
            temp = temperarure / 10000;
            sprintf(menuStr, "%i.%i C", temp, temperarure / 1000 - temp * 10);
            frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        } else {
            frameSetPosition(&screenFrame, 10, 22);
            frameAddString(&screenFrame, "Sensor ERROR", ARIAL_8PTS, false);
        }
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        if(event == MENU_EVENT_ENTER) {
            xTimerStart(menuTimer, portMAX_DELAY);
        }
        return MENU_STATUS_IDLE;
      }
    case MENU_EVENT_BS_0:
        xTimerStop(menuTimer, portMAX_DELAY);
        return MENU_STATUS_EXIT;
    default:
        return MENU_STATUS_IDLE;
    }
}

/**Menu F2. Use for show schematic**/
MenuStatus menuF2(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenuCnt;
    switch(event) {
    case MENU_EVENT_ENTER:
        frameClear(&screenFrame);
        frameSetPosition(&screenFrame, 0, 2);
        sprintf(menuStr,"%s", "menuF2");
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0:
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

/**Menu F1. Use for show measurement results**/
MenuStatus menuF3(MenuEvent event, uint32_t *children)
{
    frameClear(&screenFrame);
    frameSetPosition(&screenFrame, 20, 2);
    sprintf(menuStr,"--MEAS--");
    frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);

    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        frameClear(&screenFrame);
        frameSetPosition(&screenFrame, 20, 2);
        sprintf(menuStr,"--MEAS--");
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        frameSetPosition(&screenFrame, 10, 25);
        sprintf(menuStr,"--------");
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0:
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        isMeasComplete = false;
        measSetMeasAction(MEAS_SHORT);
        while(!isMeasComplete) {}
        frameSetPosition(&screenFrame, 5, 25);
        sprintf(menuStr, "REZ = %i", measRez);
        frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

void menuConfig(void)
{
    ADD_MENU_ITEM(menuItemRoot, menuRoot, NULL,          &menuItemF1, &menuItemF2, &menuItemF3);
    ADD_MENU_ITEM(menuItemF1,   menuF1,   &menuItemRoot);
    ADD_MENU_ITEM(menuItemF2,   menuF2,   &menuItemRoot);
    ADD_MENU_ITEM(menuItemF3,   menuF3,   &menuItemRoot);
}

extern const uint8_t iMESG_IMAGEBitmaps[];

//void measSetShortCompliteCb(MeasShortCompliteCb measShortCompliteCb);


void vUserMenuTask(void *pvParameters)
{
    int32_t temperarure;
    int8_t temp;
    buttonsConfig();
    temperatureConfig();
    displayConfig();
    menuConfig();
    measSetShortCompliteCb(measComplete);

    frameClear(&screenFrame);
    frameSetPosition(&screenFrame, 34, 2);
    frameAddImage(&screenFrame, iMESG_IMAGEBitmaps, 59, 60, false, false);
    displayInterfaceSetCursorXPos(0);
    displayInterfaceSendFrame();

    vTaskDelay(2000);

    while(1) {
        xQueueReceive(buttonEventQueue, &temp, portMAX_DELAY);
        menuProcessing(&menu, (MenuEvent)temp);
    }
}

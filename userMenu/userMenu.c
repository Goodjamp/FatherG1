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

PRESS_TYPE pressType = PRESS_SHORT;
extern const uint8_t schematicBitmaps[];
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
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        frameClear(&screenFrame);
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
        frameClear(&screenFrame);
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
        frameSetPosition(&screenFrame, 45, 22);
        if(temperatureGetTemperature(&temperarure)) {
            temp = temperarure / 10000;
            sprintf(menuStr, "%i.%i C", temp, temperarure / 1000 - temp * 10);
            frameAddString(&screenFrame, menuStr, ARIAL_11PTS, false);
        } else {
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

/**Menu F1. Use for show measurement settings**/
MenuStatus menuF3(MenuEvent event, uint32_t *children)
{
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        frameClear(&screenFrame);
        frameSetPosition(&screenFrame, 0, 2);
        sprintf(menuStr,"%s", "menuF3");
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

void menuConfig(void)
{
    ADD_MENU_ITEM(menuItemRoot, menuRoot, NULL,          &menuItemF1, &menuItemF2, &menuItemF3);
    ADD_MENU_ITEM(menuItemF1,   menuF1,   &menuItemRoot);
    ADD_MENU_ITEM(menuItemF2,   menuF2,   &menuItemRoot);
    ADD_MENU_ITEM(menuItemF3,   menuF3,   &menuItemRoot);
}

extern const uint8_t iMESG_IMAGEBitmaps[];

void vUserMenuTask(void *pvParameters)
{
    int32_t temperarure;
    int8_t temp;
    buttonsConfig();
    temperatureConfig();
    displayConfig();
    menuConfig();

    frameClear(&screenFrame);
    frameSetPosition(&screenFrame, 34, 2);
    frameAddImage(&screenFrame, iMESG_IMAGEBitmaps, 59, 60, false);
    displayInterfaceSetCursorXPos(0);
    displayInterfaceSendFrame();

    vTaskDelay(2000);

    while(1) {
        xQueueReceive(buttonEventQueue, &temp, portMAX_DELAY);
        menuProcessing(&menu, (MenuEvent)temp);
        /*
        vTaskDelay(100);

        temperatureGetTemperature(&temperarure);
        temp = temperarure / 10000;
        sprintf(temperature, "T = %i.%i C", temp, temperarure - temp * 10000);

        frameClear(&screenFrame);
        stringButton[10] = button + '0';
        frameSetPosition(&screenFrame, 0, 2);
        frameAddString(&screenFrame, stringButton, ARIAL_11PTS, false);
        frameSetPosition(&screenFrame, 0, 20);
        frameAddString(&screenFrame, (const uint8_t*)stringPressType[pressType], ARIAL_11PTS, false);

        frameSetPosition(&screenFrame, 0, 40);
        frameAddString(&screenFrame, (const uint8_t*)temperature, ARIAL_11PTS, false);

        displayInterfaceSetCursorXPos(0);
        displayInterfaceSendFrame();
        */
    }
}

#include "stdint.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "userMenu.h"

#include "displaySsd1306HAL.h"
#include "i2c_source.h"
#include "frameHall.h"

#include "buttons.h"

#define I2C_SENSOR           I2C_1
#define I2C_SENSOR_FRQ_HZ    400000

#define I2C1_SCL            GPIO_Pin_6       //PB6 ch1
#define I2C1_SCL_AF_GPIO    GPIO_PinSource6  //PB8 ch1
#define I2C1_SCL_PORT       GPIOB            //CSCL PORT

#define I2C1_SDA            GPIO_Pin_7       //PB7 ch1
#define I2C1_SDA_AF_GPIO    GPIO_PinSource7  //PB7 ch1
#define I2C1_SDA_PORT       GPIOB            //CSDA PORT

void buttonShortCb0(void);
void buttonLongCb0(void);
void buttonLongLongCb0(void);
void buttonShortCb1(void);
void buttonLongCb1(void);
void buttonLongLongCb1(void);

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
        .buttonActionCb = buttonLongLongCb0,
        .buttonNumber   = 0,
        .pressType      = PRESS_LONG_LONG,
    },
    [3] = {
        .buttonActionCb = buttonShortCb1,
        .buttonNumber   = 1,
        .pressType      = PRESS_SHORT,
    },
    [4] = {
        .buttonActionCb = buttonLongCb1,
        .buttonNumber   = 1,
        .pressType      = PRESS_LONG,
    },
    [5] = {
        .buttonActionCb = buttonLongLongCb1,
        .buttonNumber   = 1,
        .pressType      = PRESS_LONG_LONG,
    },
};

PRESS_TYPE pressType = PRESS_SHORT;
extern const uint8_t schematicBitmaps[];
uint32_t button = 0;
uint8_t stringButton[20] = "BUTTON N:  ";
const char *stringPressType[] = {
    [PRESS_SHORT]     = "SHORT",
    [PRESS_LONG]      = "LONG",
    [PRESS_LONG_LONG] = "LONG_LONG",
};

void buttonShortCb0(void)
{
    button = 0;
    pressType = PRESS_SHORT;
}
void buttonLongCb0(void)
{
    button = 0;
    pressType = PRESS_LONG;
}

void buttonLongLongCb0(void)
{
    button = 0;
    pressType = PRESS_LONG_LONG;
}

void buttonShortCb1(void)
{
    button = 1;
    pressType = PRESS_SHORT;
}

void buttonLongCb1(void)
{
    button = 1;
    pressType = PRESS_LONG;
}

void buttonLongLongCb1(void)
{
    button = 1;
    pressType = PRESS_LONG_LONG;
}

//---------------------------------I2C user implementation functions-----------------------
void i2cInitGpio(uint8_t step){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPBEN, ENABLE);

	// config I2C CSCL GPIO
	GPIO_InitStructure.GPIO_Pin = I2C1_SCL;
	GPIO_InitStructure.GPIO_Mode = (step) ? (GPIO_Mode_AF_OD) : (GPIO_Mode_IPU);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStructure);

	// config I2C CSDA GPIO
	GPIO_InitStructure.GPIO_Pin = I2C1_SDA;
	GPIO_InitStructure.GPIO_Mode =  (step) ? (GPIO_Mode_AF_OD) : ( GPIO_Mode_IPU);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStructure);

	// Enable Alternate function
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}


void delay_us(uint32_t timeout)
{
	uint32_t steps = timeout/10;
    volatile uint32_t cnt = 0;
	while( cnt++ < steps){};
}

void i2cRecover(uint32_t i2cFRQ)
{
    uint32_t halfPeriodUs = SystemCoreClock/(i2cFRQ * 2);
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPBEN, ENABLE);
/*
	if(GPIO_ReadInputDataBit(I2C1_SCL_PORT, I2C1_SDA) == Bit_SET)
	{
		return;
	}
*/
	GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
	// config I2C CSCL GPIO
	GPIO_InitStructure.GPIO_Pin   = I2C1_SCL;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	/*Config CSL functions of I2C1*/
	GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStructure);
    GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);

	GPIO_SetBits(I2C1_SDA_PORT, I2C1_SDA);
	// config I2C CSDA GPIO
	GPIO_InitStructure.GPIO_Pin   = I2C1_SDA;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStructure);
	GPIO_SetBits(I2C1_SDA_PORT, I2C1_SDA);
    /*Generate start squensys*/
    delay_us(halfPeriodUs);
    GPIO_ResetBits(I2C1_SDA_PORT, I2C1_SDA);
    delay_us(halfPeriodUs);
    GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
    delay_us(halfPeriodUs);

	//gen 9 pulses
    volatile uint8_t cnt = 0;
	#define NUMBER_PULSES    9
	while(cnt++ < NUMBER_PULSES)
	{
		GPIO_ResetBits(I2C1_SCL_PORT, I2C1_SCL);
		delay_us(halfPeriodUs);
		GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
		delay_us(halfPeriodUs);
	}

	// generate stop order
	GPIO_ResetBits(I2C1_SCL_PORT, I2C1_SCL);
	GPIO_ResetBits(I2C1_SDA_PORT, I2C1_SDA);
	delay_us(halfPeriodUs);
	GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
	delay_us(halfPeriodUs);
	GPIO_SetBits(I2C1_SDA_PORT, I2C1_SDA);

	/*One more time*/
	delay_us(halfPeriodUs);
    /*Generate start squensys*/
    delay_us(halfPeriodUs);
    GPIO_ResetBits(I2C1_SDA_PORT, I2C1_SDA);
    delay_us(halfPeriodUs);
    GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
    delay_us(halfPeriodUs);

	//gen 9 pulses
    cnt = 0;
	#define NUMBER_PULSES    9
	while(cnt++ < NUMBER_PULSES)
	{
		GPIO_ResetBits(I2C1_SCL_PORT, I2C1_SCL);
		delay_us(halfPeriodUs);
		GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
		delay_us(halfPeriodUs);
	}

	// generate stop order
	GPIO_ResetBits(I2C1_SCL_PORT, I2C1_SCL);
	GPIO_ResetBits(I2C1_SDA_PORT, I2C1_SDA);
	delay_us(halfPeriodUs);
	GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL);
	delay_us(halfPeriodUs);
	GPIO_SetBits(I2C1_SDA_PORT, I2C1_SDA);

	i2cInitGpio(1);
}


uint32_t i2cgetTimeMs(void)
{
	return xTaskGetTickCount();
}

void i2c_init(void){
	I2C_configDef i2c_configParamiters = {
			.frequencyI2C = I2C_SENSOR_FRQ_HZ
	};
	i2cConfig(I2C_SENSOR, &i2c_configParamiters);
}

bool sendBuffCBDisplay(uint8_t displayAddres, uint8_t data[], uint16_t dataSize)
{
    i2cTxData(I2C_1, displayAddres, data[0], (dataSize - 1) , &data[1]);
    return true;
}

FrameDescr screenFrame;
DisplayFrame displayFrame;


void displauInit(void)
{
    displayInit(sendBuffCBDisplay, SSD1306_Y_POS_0, SSD1306_Y_POS_64);
    frameInit(&screenFrame, displayFrame.buffer, FRAME_HEIGHT_DOT, FRAME_WIDTH_DOT);
}
uint8_t image[1] = {0xFF};//, 0b111};

#include "OneWireInterfaceAPI.h"
uint8_t dataReadData[] = {0xCC, 0xBE};
uint8_t dataReadRom[] = {0x33};
    uint8_t rezData[9];


void vUserMenuTask(void *pvParameters)
{
    oneWireResetBloking();
    oneWireSendBloking(dataReadData, sizeof(dataReadData));
    oneWireReceiveBloking(rezData, 9);
    oneWireResetBloking();
    oneWireSendBloking(dataReadRom, sizeof(dataReadRom));
    oneWireReceiveBloking(rezData, 9);
    vTaskDelay(10);

    i2c_init();
    displauInit();
    buttonsInitButton(buttonActionDescription,
                     sizeof(buttonActionDescription) / sizeof(buttonActionDescription[0]));

    frameClear(&screenFrame);
    displaySetCursorXPos(0);
    displaySendFrame(&displayFrame);
    displaySetCursorXPos(0);
    frameClear(&screenFrame);
/*
    for(uint32_t k = 0; k < 64; k++) {
        frameSetPosition(&screenFrame, k, k);
        frameAddImage(&screenFrame, image, 11, 1, false);
    }
*/
    frameSetPosition(&screenFrame, 0, 2);
    frameAddImage(&screenFrame, image, 3, 1, false);
    displaySendFrame(&displayFrame);

    while(1) {
        vTaskDelay(100);
        frameClear(&screenFrame);
        stringButton[10] = button + '0';
        frameSetPosition(&screenFrame, 0, 2);
        frameAddString(&screenFrame, stringButton, ARIAL_11PTS, false);
        frameSetPosition(&screenFrame, 0, 20);
        frameAddString(&screenFrame, (const uint8_t*)stringPressType[pressType], ARIAL_11PTS, false);
        displaySetCursorXPos(0);
        displaySetYArea(SSD1306_Y_POS_0, SSD1306_Y_POS_64);
        displaySendFrame(&displayFrame);
        i2c_init();
    }
}

#include "stdint.h"
#include "stdbool.h"

#include "i2c_source.h"
#include "displaySsd1306HAL.h"

#define I2C_SENSOR           I2C_1
#define I2C_SENSOR_FRQ_HZ    400000

#define I2C1_SCL            GPIO_Pin_6       //PB6 ch1
#define I2C1_SCL_AF_GPIO    GPIO_PinSource6  //PB8 ch1
#define I2C1_SCL_PORT       GPIOB            //CSCL PORT

#define I2C1_SDA            GPIO_Pin_7       //PB7 ch1
#define I2C1_SDA_AF_GPIO    GPIO_PinSource7  //PB7 ch1
#define I2C1_SDA_PORT       GPIOB            //CSDA PORT

static DisplayFrame displayFrame;

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

void displayInterfaceInit(void)
{
    i2c_init();
    displayInit(sendBuffCBDisplay, SSD1306_Y_POS_0, SSD1306_Y_POS_64);
}

uint8_t* displayInterfaceGetFrameBuffer(void)
{
    return displayFrame.buffer;
}

bool displayInterfaceSendFrame(void)
{
    displaySendFrame(&displayFrame);
}

bool displayInterfaceSetCursorXPos(uint8_t posX)
{
    displaySetCursorXPos(posX);
}



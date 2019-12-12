#include "stdint.h"
#include "string.h"
#include "stdbool.h"

#include "displaySsd1306HAL.h"

#define BIT_DC_COMAND    0b00000000
#define BIT_DC_DATA      0b01000000

#define BIT_C_CONTONUE   0b10000000
#define BIT_C_SIMPLE     0b00000000

#define CONTROL_COMAND_SIMLE 0b00000000
#define CONTROL_COMAND_CONT  0b10000000

#define CONTROL_DATA_SIMLE 0b01000000
#define CONTROL_DATA_CONT  0b11000000


/*************************COMMAND DESCRIPTION********/
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_EXTERNALVCC         0x1
#define SSD1306_SWITCHCAPVCC        0x2
/***************************************************/
#define SSD1306_PAGE_START     0b10110000
#define SSD1306_PAGE_START_END 0b00100010
#define SSD1306_ROW_START_LO   0b00000000
#define SSD1306_ROW_START_HI   0b00010000

typedef enum {
    setDisplayOnOff_OFF = 0xAE,
    setDisplayOnOff_ON  = 0xAF
}setDisplayOnOffDef;

typedef enum {
    entireDisplayOn_ON = 0xA4,
    entireDisplayOn_Resume  = 0xAF
}entireDisplayOnOffDef;


uint8_t displayAddressWrite = DISPLAY_ADDRESS;//((DISPLAY_ADDRESS << 1) | 1);

static SendBuffCB sendBuff;

#define COMMAND_BUFFER_SIZE   8
static uint8_t commandBuffer[COMMAND_BUFFER_SIZE];


bool sendCommand(uint8_t command[], uint8_t commandSize)
{
    commandBuffer[0] = CONTROL_COMAND_SIMLE;
    memcpy(&commandBuffer[1], command, commandSize);
    return sendBuff(displayAddressWrite, commandBuffer, commandSize + 1);
}

bool sendData(uint8_t data[], uint16_t dataSize)
{
    data[0] = CONTROL_DATA_SIMLE;
    return sendBuff(displayAddressWrite, data, dataSize + 1);
}

bool displaySendFrame(DisplayFrame *frame)
{
    frame->command = CONTROL_DATA_SIMLE;
    union{
        uint8_t *pBuff;
        DisplayFrame *buff;
    }temp = {.buff = frame};
    return sendBuff(displayAddressWrite, temp.pBuff, sizeof(DisplayFrame));
}

uint8_t* displayGetFrame(DisplayFrame *frame)
{
    return frame->buffer;
}

void displayInit(SendBuffCB sendBuffCB, Ssd1306YPos startY, Ssd1306YPos stopY)
{
    uint8_t comandBuff[4];
    sendBuff = sendBuffCB;

    // Init sequence for 128x64 OLED module
    comandBuff[0] = SSD1306_DISPLAYOFF;         //+
    sendCommand(comandBuff,1);            // 0xAE

    comandBuff[0] = SSD1306_SETDISPLAYCLOCKDIV; //+
    comandBuff[1] = 0x80;
    sendCommand(comandBuff,2);            // 0xD5

    comandBuff[0] = SSD1306_SETDISPLAYOFFSET;
    comandBuff[1] = 0x0;
    sendCommand(comandBuff,2);            // no offset

    comandBuff[0] = SSD1306_SETSTARTLINE;       //+
    sendCommand(comandBuff,1);// | 0x0);  // line #0

    comandBuff[0] = SSD1306_CHARGEPUMP;         //+
    comandBuff[1] = 0x14;
    sendCommand(comandBuff,2);            // 0x8D
    //sendCommand(0x14);                  // using internal VCC

    comandBuff[0] = SSD1306_MEMORYMODE;         //++
    comandBuff[1] = 0x00;
    sendCommand(comandBuff,2);
    //sendCommand(0x00);                  // 0x00 horizontal addressing

    comandBuff[0] = SSD1306_SEGREMAP | 0x0;     //A0
    sendCommand(comandBuff,1);            // rotate screen 180

    comandBuff[0] = SSD1306_COMSCANINC;         //C0
    sendCommand(comandBuff,1);            // rotate screen 180

    comandBuff[0] = SSD1306_SETCONTRAST;
    comandBuff[1] = 0xF0;
    sendCommand(comandBuff,2);            // 0x81
    //sendCommand(0xCF);

    comandBuff[0] = SSD1306_SETPRECHARGE;       //D9
    comandBuff[1] = 0xF1;
    sendCommand(comandBuff,2);            // 0xd9
    //sendCommand(0xF1);

    comandBuff[0] = SSD1306_SETVCOMDETECT;      //DB
    comandBuff[1] = 0x40;
    sendCommand(comandBuff,2);            // 0xDB
   // sendCommand(0x40);

    comandBuff[0] = SSD1306_DISPLAYALLON_RESUME;
    sendCommand(comandBuff,1);            // 0xA4

    comandBuff[0] = SSD1306_NORMALDISPLAY;
    sendCommand(comandBuff,1);            // 0xA6

    comandBuff[0] = SSD1306_DISPLAYON;
    sendCommand(comandBuff,1);

    comandBuff[0] = SSD1306_PAGE_START | 0;
    sendCommand(comandBuff,1);

    comandBuff[0] = SSD1306_ROW_START_LO | 0;
    sendCommand(comandBuff,1);

    comandBuff[0] = SSD1306_ROW_START_HI | 0;
    sendCommand(comandBuff,1);

    comandBuff[0] = SSD1306_PAGE_START_END;
    comandBuff[1] = startY; // first page
    comandBuff[2] = stopY; // last page
    sendCommand(comandBuff,3);

    //Next configuration need for switch SSD1306 for work in 128*32 or 128*64 mode
    comandBuff[0] = SSD1306_SETCOMPINS;         //DA
    comandBuff[1] = 0b010010;
    sendCommand(comandBuff,2);

    comandBuff[0] = SSD1306_SETMULTIPLEX;       // 0xA8
    comandBuff[1] = 0x3f;
    sendCommand(comandBuff,2);
}

bool displaySetCursorXPos(uint8_t posX)
{
    uint8_t comandBuff[4];
    comandBuff[0] = SSD1306_ROW_START_LO | (posX & 0xF);
    sendCommand(comandBuff,1);

    comandBuff[0] = SSD1306_ROW_START_HI | ((posX & 0xF0) >> 4);
    return sendCommand(comandBuff,1);
}

bool displaySetYArea(Ssd1306YPos startY, Ssd1306YPos stopY)
{
    uint8_t comandBuff[4];
    comandBuff[0] = SSD1306_PAGE_START_END;
    comandBuff[1] = startY; // first page
    comandBuff[2] = stopY; // last page
    return sendCommand(comandBuff,3);
}


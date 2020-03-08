#include "stdint.h"
#include "string.h"

#include "frameHall.h"
#include "displaySsd1306HAL.h"

#include "symbolsArial11pts.h"
#include "symbolsArial8pts.h"
//#include "symbolsArial16pts.h"
//#include "symbolsSegoePrint14pts.h"

/*fontDescriptor fields */
#define WIDTH_POS   0
#define HEIGHT_POS  1
#define ADDRESS_POS 2

/*fontInfo fields */
#define FIRST_SYMBOL_POS      1
#define LAST_SYMBOL_POS       2
#define FONT_DESCRIPTIONS_POS 4
#define FONT_BITMAP_POS       5

static const uint32_t *fontInfoList[] =
{
    [ARIAL_8PTS]  = (uint32_t*)arial_8ptFontInfo,
    [ARIAL_11PTS] = (uint32_t*)arial_11ptFontInfo,
    //[ARIAL_16PTS] = (uint32_t*)arial_16ptFontInfo,
    //[SEGOEPRINT_14PTS] = (uint32_t*)segoePrint_14ptFontInfo,
};

static const uint8_t* getSymbol(const uint8_t *symbol,
                                          uint8_t *height,
                                          uint8_t *width,
                                          SymbolType symbolType)
{
    const uint32_t *fontInfo            = fontInfoList[symbolType];
    const uint32_t (*fontDescriptor)[3] = (uint32_t(*)[3])fontInfo[FONT_DESCRIPTIONS_POS];
    const uint8_t *fontBitmap           = (uint8_t*)fontInfo[FONT_BITMAP_POS];

    if((*symbol < fontInfo[FIRST_SYMBOL_POS]) || (*symbol > fontInfo[LAST_SYMBOL_POS])) {
        return NULL;  // symbol out of range
    }

    uint8_t pos = *symbol - fontInfo[FIRST_SYMBOL_POS];

    *height = fontDescriptor[pos][HEIGHT_POS];
    *width  = fontDescriptor[pos][WIDTH_POS];
    return fontBitmap + fontDescriptor[pos][ADDRESS_POS];
}

static bool addImage(FrameDescr *inFrame, const uint8_t *image, uint8_t heigh, uint8_t width, bool leaveExisting, bool invers)
{
    uint8_t  (*imageArray)[width] = (uint8_t(*)[width])image;
    uint8_t  (*frameArray)[inFrame->width] = (uint8_t(*)[inFrame->width])inFrame->buff;
    uint32_t frameX    = 0;
    uint32_t imageY    = 0;
    uint32_t frameByte = inFrame->y >> 3;
    uint32_t frameBit  = inFrame->y % 8;
    uint32_t imageByte = 0;
    uint32_t imageBit  = 0;
    uint8_t  frameMaska = 0;
    uint8_t  imageMaska = 0;
    uint8_t  frameMaskaRev = 0;

    while(imageY < heigh) {
        frameMaska = 1 << frameBit;
        imageMaska = 1 << imageBit;
        frameMaskaRev = ~frameMaska;
        frameX = inFrame->x;
        for(uint32_t imageX = 0; imageX < width; imageX++, frameX++) {
            if(invers) {
                if(imageArray[imageByte][imageX] & imageMaska) {
                    if(!leaveExisting) {
                        frameArray[frameByte][frameX] &= frameMaskaRev;
                    }
                } else {
                    frameArray[frameByte][frameX] |= frameMaska;
                }
            } else {
                if(imageArray[imageByte][imageX] & imageMaska) {
                    frameArray[frameByte][frameX] |= frameMaska;
                } else {
                    if(!leaveExisting) {
                        frameArray[frameByte][frameX] &= frameMaskaRev;
                    }
                }
            }
        }
        if(++imageBit == 8) {
            imageBit = 0;
            imageByte++;
        }
        if(++frameBit == 8) {
            frameBit = 0;
            frameByte++;
        }
        imageY++;
    }
}

void frameInit(FrameHandl inFrame, uint8_t *frameBuff, uint16_t heigh, uint16_t width)
{
    inFrame->buff  = frameBuff;
    inFrame->heigh = heigh;
    inFrame->width = width;
}

bool frameAddString(FrameHandl inFrame, const uint8_t *str, SymbolType symbolType, bool leaveExisting)
{
    #define SPACE_BITS  4
    const uint8_t *symbol;
    uint8_t symbolHeigh;
    uint8_t symbolWidth;
    while(*str) {
        symbol = getSymbol(str, &symbolHeigh, &symbolWidth, symbolType);
        if((*str == ' ') || (symbol == NULL)) {
            inFrame->x += SPACE_BITS;
            str++;
            continue;
        }

        if((inFrame->x + symbolWidth) >= inFrame->width) {
           if((inFrame->y + symbolHeigh) >= inFrame->heigh) {
               return false;
           }
           inFrame->y += symbolHeigh;
           inFrame->x = 0;
        }

        addImage(inFrame, symbol, symbolHeigh, symbolWidth, leaveExisting, false);
        inFrame->x += symbolWidth + SPACE_BITS;
        str++;
    }
    return true;
}

bool frameAddImage(FrameHandl inFrame, const uint8_t *image, uint8_t imageHeigh, uint8_t imageWidth, bool leaveExisting, bool invers)
{
    return addImage(inFrame, image, imageHeigh, imageWidth, leaveExisting, invers);
}

void frameAddArea(FrameHandl inFrame, Point beginPoint, uint8_t height, uint8_t width)
{
    uint8_t  (*screenBuff)[inFrame->width] = (uint8_t(*)[inFrame->width])inFrame->buff;
    uint8_t y = beginPoint.y >> 3;
    uint8_t fb = beginPoint.y - y * 8; // first bit in mask
    uint8_t lb; // last bit in mask
    uint8_t mb; // mask
    uint8_t xl = beginPoint.x + width;
    do {
        lb = ((lb = fb + height - 1) > 7) ? (7) : (lb);
        mb = (~(0xFF << (lb - fb + 1))) << fb;
        height -= (lb + 1 - fb);
        fb = 0;
        for(uint8_t x = beginPoint.x; x < xl; x++) {
            screenBuff[y][x] |= mb;
        }
        y++;
    } while(height);
}

void frameAddRectangle(FrameHandl inFrame, Point upLeft, Point downRight, uint8_t width)
{
    //up horizontal line
    frameAddArea(inFrame, upLeft, width, downRight.x - upLeft.x);
    //down horizontal line
    frameAddArea(inFrame, (Point){upLeft.x, downRight.y - width + 1}, width, downRight.x - upLeft.x + 1);
    //left vertical line
    frameAddArea(inFrame, upLeft, downRight.y - upLeft.y  , width);
    //left vertical line
    frameAddArea(inFrame, (Point){downRight.x - width + 1, upLeft.y}, downRight.y - upLeft.y + 1 , width);
}

void frameAddHorizontalLine(FrameHandl inFrame, Point startPoint, uint8_t width, uint8_t length)
{
    frameAddArea(inFrame, startPoint, width, length);
}

void framePutPixe(FrameHandl inFrame, Point pixel)
{
    uint8_t  (*screenBuff)[inFrame->width] = (uint8_t(*)[inFrame->width])inFrame->buff;
    uint8_t y = pixel.y >> 3;
    uint8_t fb = pixel.y - y * 8;
    screenBuff[y][pixel.x] |= (1 << fb);
}

bool frameClear(FrameHandl inFrame)
{
   memset(inFrame->buff, 0, inFrame->heigh * inFrame->width /8);
   return true;
}

bool frameSetPosition(FrameHandl inFrame, uint8_t inX, uint8_t inY)
{
    inFrame->x = inX;
    inFrame->y = inY;
    return true;
}

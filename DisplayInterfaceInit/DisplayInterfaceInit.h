#ifndef __DISPLAY_INTERFACE_INIT_H__
#define __DISPLAY_INTERFACE_INIT_H__

#include "stdint.h"

#define FRAME_WIDTH  128
#define FRAME_HEIGHT 64

void     displayInterfaceInit(void);
uint8_t* displayInterfaceGetFrameBuffer(void);
bool     displayInterfaceSendFrame(void);
bool     displayInterfaceSetCursorXPos(uint8_t posX);

#endif

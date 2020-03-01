#ifndef __BUTTONS_HALL_H__
#define __BUTTONS_HALL_H__

#include "stdint.h"

#define BUTTON_CNT 3

typedef enum {
    PRESS_SHORT,
    PRESS_LONG,
    PRESS_LONG_LONG,
    PRESS_TYPE_CNT,
} PRESS_TYPE;

typedef void (*ButtonActionCb)(void);
typedef bool (*ReadButtonCb)(uint16_t buttonNumber, bool *rezRead);

typedef struct {
    ButtonActionCb buttonActionCb;
    uint32_t buttonNumber;
    PRESS_TYPE pressType;
} ButtonActionDescription;

void buttonsInitButton(const ButtonActionDescription *buttonDescription,
                       uint32_t size,
                       ReadButtonCb readButtonCb);
void vButtonsTask(void *pvParameters);

#endif

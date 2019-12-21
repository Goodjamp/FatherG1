#include <stdio.h>
#include "../../buttons.h"


void buttonPresShortCB_0(void)
{
    printf("button 0 press SHORT \n");
}

void buttonPresLongCB_0(void)
{
    printf("button 0 press LONG \n");
}

void buttonPresLongLongCB_0(void)
{
    printf("button 0 press LONG LONG \n");
}

void buttonPresShortCB_1(void)
{
    printf("button 1 press SHORT \n");
}

void buttonPresLongCB_1(void)
{
    printf("button 1 press LONG \n");
}

void buttonPresLongLongCB_1(void)
{
    printf("button 1 press LONG LONG \n");
}


const ButtonActionDescription buttonDescription[] = {
    [0] = {.buttonActionCb = buttonPresShortCB_0,
           .buttonNumber   = 0,
           .pressType      = PRESS_SHORT,
    },
    [1] = {.buttonActionCb = buttonPresLongCB_0,
           .buttonNumber   = 0,
           .pressType      = PRESS_LONG,
    },
    [2] = {.buttonActionCb = buttonPresLongLongCB_0,
           .buttonNumber   = 0,
           .pressType      = PRESS_LONG_LONG,
    },
    [3] = {.buttonActionCb = buttonPresShortCB_1,
           .buttonNumber   = 1,
           .pressType      = PRESS_SHORT,
    },
    [4] = {.buttonActionCb = buttonPresLongCB_1,
           .buttonNumber   = 1,
           .pressType      = PRESS_LONG,
    },
    [5] = {.buttonActionCb = buttonPresLongLongCB_1,
           .buttonNumber   = 1,
           .pressType      = PRESS_LONG_LONG,
    },
};


int main()
{
    buttonsInitButton(buttonDescription, sizeof(buttonDescription) / sizeof(buttonDescription[0]));
    printf("Hello World!\n");
    vButtonsTask((void *)NULL);
    return 0;
}

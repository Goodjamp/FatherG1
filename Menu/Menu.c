#include <stdint.h>
#include <stdio.h>

#include "Menu.h"

void menuProcessing(MenuItem **menuItem, MenuEvent event) {
    uint32_t item;
    switch((*(*menuItem)).itemF(event, &item)) {
    case MENU_STATUS_IDLE:
        return;
    case MENU_STATUS_ENTER:
        if((*menuItem)->childrenCnt == 0
           || item > (*menuItem)->childrenCnt) {
            return;
        }
        *menuItem = (MenuItem*)(*menuItem)->childrenList[item];
        (*menuItem)->itemF(MENU_EVENT_ENTER, &item);
        break;
    case MENU_STATUS_EXIT:
        if((MenuItem*)(*menuItem)->parent == NULL) {
            return;
        }
        *menuItem = (MenuItem*)(*menuItem)->parent;
        (*menuItem)->itemF(MENU_EVENT_ENTER, &item);
        break;
    };
}

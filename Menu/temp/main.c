#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"


#include "../Menu.h"

MenuStatus menuRoot(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Menu Root, ENTER subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu Root: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu Root: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu Root: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuStatus menuF1(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Entr menu menuF1, ENTR subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu menuF1: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu menuF1: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu menuF1: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuStatus menuF2(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Entr menu menuF2, ENTR subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu menuF2: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu menuF2: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu menuF2: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuStatus menuF1_1(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Entr menu menuF1_1, ENTR subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu menuF1_1: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu menuF1_1: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu menuF1_1: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuStatus menuF1_2(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Entr menu menuF1_2, ENTR subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu menuF1_2: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu menuF1_2: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu menuF1_2: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuStatus menuF2_1(MenuEvent event, uint32_t *children)
{
    static uint32_t subMenCnt;
    switch(event) {
    case MENU_EVENT_ENTER: // entr to menu
        subMenCnt = 0;
        printf("Entr menu menuF2_1, ENTR subMenuCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BS_0: // switch children
        if(++subMenCnt > 4) {
            subMenCnt = 0;
        }
        printf("Menu menuF2_1: SELECT CHILDREN subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_IDLE;
    case MENU_EVENT_BL_0:
        printf("Menu menuF2_1: EXIT subMenCnt = %u \n", subMenCnt);
        return MENU_STATUS_EXIT;
    case MENU_EVENT_BS_1:
        printf("Menu menuF2_1: NEXT subMenCnt = %u \n", subMenCnt);
        *children = subMenCnt;
        return MENU_STATUS_ENTER;
    default:
        return MENU_STATUS_IDLE;
    }
}

MenuItem menuItemF1;
MenuItem menuItemF1_1;
MenuItem menuItemF1_2;
MenuItem menuItemF2;
MenuItem menuItemF2_1;
MenuItem menuItemRoot;
MenuItem *menu = &menuItemRoot;

void initMenu(void)
{
    ADD_MENU_ITEM(menuItemRoot, menuRoot, NULL,          &menuItemF1,   &menuItemF2);
    ADD_MENU_ITEM(menuItemF1,   menuF1,   &menuItemRoot, &menuItemF1_1, &menuItemF1_2);
    ADD_MENU_ITEM(menuItemF2,   menuF2,   &menuItemRoot, &menuItemF2_1);
    ADD_MENU_ITEM(menuItemF1_1, menuF1_1, &menuItemF1);
    ADD_MENU_ITEM(menuItemF1_2, menuF1_2, &menuItemF1);
    ADD_MENU_ITEM(menuItemF2_1, menuF2_1, &menuItemF2);
}

int main(int argCnt, char **vArg) {
    char event;
    initMenu();
    while(1) {
        event = getchar();
        switch(event) {
        case 'q':
            menuProcessing(&menu, MENU_EVENT_BS_0);
            break;
        case 'w':
            menuProcessing(&menu, MENU_EVENT_BL_0);
            break;
        case 'e':
            menuProcessing(&menu, MENU_EVENT_BS_1);
            break;
        default: continue;
        }
    }
}

#include <stdint.h>

#define ADD_MENU_ITEM(MENU_ITEM, MENU_FUNCTION, PARENT, ...)            \
    static const MenuItem *MENU_ITEM##_CHILDREN_LIST[] = {__VA_ARGS__}; \
    MENU_ITEM.parent       = PARENT;                                   \
    MENU_ITEM.childrenList = MENU_ITEM##_CHILDREN_LIST;                \
    MENU_ITEM.childrenCnt  = sizeof(MENU_ITEM##_CHILDREN_LIST) /        \
                             sizeof(MENU_ITEM##_CHILDREN_LIST[0]);      \
    MENU_ITEM.itemF        = MENU_FUNCTION;

typedef enum MenuEvent {
    MENU_EVENT_ENTER,   // Enter to menu item
    MENU_EVENT_BS_0,    // Button 0 short press
    MENU_EVENT_BL_0,    // Button 0 long press
    MENU_EVENT_BS_1,    // Button 1 short press
    MENU_EVENT_BL_1,    // Button 1 long press
    MENU_EVENT_TIMER,   // Timer
} MenuEvent;

typedef enum MenuStatus {
    MENU_STATUS_IDLE,    // Leav in current menu
    MENU_STATUS_ENTER,   // Enter to children
    MENU_STATUS_EXIT,    // Exit to parent
} MenuStatus;

typedef MenuStatus (*MenuItemF)(MenuEvent event, uint32_t *children);

typedef struct MenuItem {
    struct MenuItem const *parent;
    struct MenuItem const **childrenList;
    MenuItemF             itemF;
    uint32_t              childrenCnt;
} MenuItem;

void menuProcessing(MenuItem **menuItem, MenuEvent event);

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

#include "misc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#include "userMenu.h"
#include "buttons.h"
#include "Measurement.h"
#include "usbProto.h"

#define MEASUREMENT_TASK_STACK_SIZE   256
#define MEASUREMENT_TASK_PRIORITY     2

#define USER_MENU_TASK_STACK_SIZE     256
#define USER_MENU_TASK_PRIORITY       2

#define BUTTON_TASK_STACK_SIZE        256
#define BUTTON_TASK_PRIORITY          2

#define USBPROTO_TASK_STACK_SIZE      256
#define USBPROTO_TASK_PRIORITY        2

int main(void)
{
    xTaskCreate(vUsbProtoTask,
               "USBPROTO_TASK",
               USBPROTO_TASK_STACK_SIZE,
               NULL,
               USBPROTO_TASK_PRIORITY,
               NULL);

    xTaskCreate(vMeasTask,
                "MEASUREMENT_TASK",
                MEASUREMENT_TASK_STACK_SIZE,
                NULL,
                MEASUREMENT_TASK_PRIORITY,
                NULL);
     xTaskCreate(vUserMenuTask,
                "USER_MENU_TASK",
                USER_MENU_TASK_STACK_SIZE,
                NULL,
                USER_MENU_TASK_PRIORITY,
                NULL);
     xTaskCreate(vButtonsTask,
                "BUTTON_TASK",
                BUTTON_TASK_STACK_SIZE,
                NULL,
                BUTTON_TASK_PRIORITY,
                NULL);
   vTaskStartScheduler();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
}

void vApplicationIdleHook( void )
{
}

void vApplicationMallocFailedHook( void )
{
}

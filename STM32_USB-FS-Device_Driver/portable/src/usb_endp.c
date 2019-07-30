/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  */


/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_user_setings.h"
#include "usb_pwr.h"

#include "usbHIDInterface.h"


extern __IO uint32_t bDeviceState;

uint8_t Receive_Buffer[EP_COUNT+1];
__IO uint8_t PrevXferComplete[EP_NUM_MAX] = {[0 ... EP_NUM_MAX -1 ] = 1};

/* user callback function*/
static usbHIDRxHandlCB         rxCB;
static usbHIDTxCompleteHandlCB txCompleteCB;

/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/


void EP1_OUT_Callback(void)
{
	uint32_t numRx = USB_SIL_Read(EP1_OUT, Receive_Buffer);

	SetEPRxStatus(ENDP1, EP_RX_VALID);
	//user call back function
	if(rxCB == NULL)
	{
		return;
	}
	rxCB( ENDP1, numRx, Receive_Buffer);
	// Call user processing function
}


/*******************************************************************************
* Function Name  : EP1_IN_Callback.
* Description    : EP1 IN Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
    PrevXferComplete[ENDP1] = 1;
    //user call back function
    if(txCompleteCB == NULL)
    {
       return;
    }
    txCompleteCB();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

void usbHIDInit(void){
	USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
}



void usbHIDAddRxCB(usbHIDRxHandlCB inRxCB)
{
	rxCB = inRxCB;
}


void usbHIDAddTxCompleteCB(usbHIDTxCompleteHandlCB inTxCompleteCB)
{
	txCompleteCB = inTxCompleteCB;
}


bool usbHIDTx(uint8_t endPointNum, uint8_t *data, uint8_t dataSize)
{
	if((PrevXferComplete[endPointNum] != 1) || (bDeviceState != CONFIGURED))
	{
		return false;
	}
	PrevXferComplete[endPointNum]=0;

	USB_SIL_Write( (0b10000000 | endPointNum) , data, dataSize);
	SetEPTxValid(endPointNum);

	return true;
}


bool usbHIDEPIsReadyToTx(uint8_t numEP)
{
	return (PrevXferComplete[numEP] == 1) ? (true) : (false);
}


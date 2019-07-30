/**
  ******************************************************************************
  * @file    stm32_it.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  */



#include "stm32_it.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "hw_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t Send_Buffer[2];
extern __IO uint8_t PrevXferComplete;
extern __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/******************************************************************************/
/*            STM32 Peripherals Interrupt Handlers                        */
/******************************************************************************/

/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void USB_LP_CAN1_RX0_IRQHandler(void)
{

  USB_Istr();
}

#if defined(STM32L1XX_MD)
/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
  /* Check on the RIGHT button */
  if(EXTI_GetITStatus(RIGHT_BUTTON_EXTI_LINE) != RESET)
  {  
    if ((PrevXferComplete) && (bDeviceState == CONFIGURED))
    {
      Send_Buffer[0] = 0x05; 
      
      if (STM_EVAL_PBGetState(Button_RIGHT) == Bit_RESET)
      {
        Send_Buffer[1] = 0x01;
      }
      else 
      {
        Send_Buffer[1] = 0x00;
      }  
      
      /* Write the descriptor through the endpoint */
      USB_SIL_Write(EP1_IN, (uint8_t*) Send_Buffer, 2);  
      
      SetEPTxValid(ENDP1);
      
      PrevXferComplete = 0;
    }
    /* Clear the EXTI line  pending bit */
    EXTI_ClearITPendingBit(RIGHT_BUTTON_EXTI_LINE);
  }  

  /* Check on the LEFT button */
  if(EXTI_GetITStatus(LEFT_BUTTON_EXTI_LINE) != RESET)
  {  
    if ((PrevXferComplete) && (bDeviceState == CONFIGURED))
    {
      Send_Buffer[0] = 0x06; 
      
      if (STM_EVAL_PBGetState(Button_LEFT) == Bit_RESET)
      {
        Send_Buffer[1] = 0x01;
      }
      else 
      {
        Send_Buffer[1] = 0x00;
      }  
      
      /* Write the descriptor through the endpoint */
      USB_SIL_Write(EP1_IN, (uint8_t*) Send_Buffer, 2);  
      
      SetEPTxValid(ENDP1);
      
      PrevXferComplete = 0;
    }
    /* Clear the EXTI line  pending bit */
    EXTI_ClearITPendingBit(LEFT_BUTTON_EXTI_LINE);
  }  
}
#endif


/*******************************************************************************
* Function Name  : EXTI_IRQHandler
* Description    : This function handles External lines interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void EXTI9_5_IRQHandler(void)
{
	/*
  if(EXTI_GetITStatus(KEY_BUTTON_EXTI_LINE) != RESET)
  {  
    if ((PrevXferComplete) && (bDeviceState == CONFIGURED))
    {
      Send_Buffer[0] = 0x05; 
#if defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
      if (!STM_EVAL_PBGetState(Button_KEY) == Bit_RESET)
#else
      if (STM_EVAL_PBGetState(Button_KEY) == Bit_RESET)       
#endif      
      {
        Send_Buffer[1] = 0x01;
      }
      else 
      {
        Send_Buffer[1] = 0x00;
      }  
      */
      /* Write the descriptor through the endpoint */
	/*
      USB_SIL_Write(EP1_IN, (uint8_t*) Send_Buffer, 2);  
      SetEPTxValid(ENDP1);
      PrevXferComplete = 0;
      */
      /*
    }

    EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
  }
*/
}

/*******************************************************************************
* Function Name  : USB_FS_WKUP_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line18);
}
/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32xxx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * @file    usb_user_setings.h
  * @author  Gerasimchuk A.
  * @version V1.0.0
  * @date    24-January-2017
  * @brief   Main setings of USB
  * */
#ifndef USB_USER_SETINGS_H_
#define USB_USER_SETINGS_H_

/* EP_NUM */
#define EP_NUM     (2)
#define EP_COUNT    (uint8_t)(0x40)


/* Selected EP definition  (uncoment selected)*/
/*8n - EPn_IN  endpoint (host IN)
 *0n - EPn_OUT endpoint (host OUT)
 */
/*----EP 1----*/
#define EP_01 0x1
#define EP_81 0x81
/*----EP 2----*/
//#define EP_02
//#define EP_82
/*----EP 3----*/
//#define EP_03
//#define EP_83
/*----EP 4----*/
//#define EP_04
//#define EP_84
/*----EP 5----*/
//#define EP_05
//#define EP_85
/*----EP 6----*/
//#define EP_06
//#define EP_86
/*----EP 7----*/
//#define EP_07
//#define EP_87

/* EP1  */
/* tx buffer base address */
#define ENDP1_TXADDR        (0x100)
#define ENDP1_RXADDR        (0x140)


#endif

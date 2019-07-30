/* file:   HIDIntarface.h
 * Author: Gerasimchuk
 * Date:   13 jun 2017
 * Version: 1.0
 *
 * Inarface between USB FW and application
 */

#ifndef HIDINTARFACE_H_
#define HIDINTARFACE_H_

#include "stdint.h"

typedef void (*usbHIDRxHandlCB)        (uint8_t epNumber, uint8_t numRx, uint8_t* rxData);
typedef void (*usbHIDTxCompleteHandlCB)(void);


void usbHIDInit(void);
void usbHIDAddRxCB(usbHIDRxHandlCB inRxCB);
void usbHIDAddTxCompleteCB(usbHIDTxCompleteHandlCB inTxCompleteCB);
bool usbHIDTx(uint8_t endPointNum, uint8_t *data, uint8_t dataSize);
bool usbHIDEPIsReadyToTx(uint8_t numEP);

#endif

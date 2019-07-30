/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptor Header for Custom HID Demo
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H
#include "usb_user_setings.h"

// Descriptor definition
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define HID_DESCRIPTOR_TYPE                     0x21

#define CUSTOMHID_OFF_HID_DESC                  0x12

#define CUSTOMHID_SIZ_STRING_LANGID             4
#define CUSTOMHID_SIZ_STRING_VENDOR             38
#define CUSTOMHID_SIZ_STRING_PRODUCT            32
#define CUSTOMHID_SIZ_STRING_SERIAL             30


#pragma pack(1)

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
}sDeviceDescriptor;


typedef struct{
	uint8_t reserved:5;
	uint8_t remoteWakeup:1;
	uint8_t selfPowered:1;
	uint8_t Reserved:1;
}bmAttributes;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
}sConfigurationDescriptor;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
}sInetrfaceDescriptor;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorType1;
	uint16_t wDescriptorLength;
}sHIDDescriptor;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
}sEndPointDescriptor;



//typedef sEndPointDescriptor sEndPointArrayDescr[EP_NUM];
typedef struct {
	sInetrfaceDescriptor sInetrfaceDesc;
	sHIDDescriptor       HIDDesc;
	sEndPointDescriptor  EndPointDesc[EP_NUM];
}sInetrfaceDescUser;

typedef struct{
	sConfigurationDescriptor  ConfigurationDescriptor;
	sInetrfaceDescUser        userDesc;
}sUSBHIDDescriptor;

#pragma pack()


extern const sDeviceDescriptor CustomHID_DeviceDescriptor;
extern const sUSBHIDDescriptor CustomHID_ConfigDescriptor;
extern const uint8_t CustomHID_ReportDescriptor[];
extern const uint8_t CustomHID_StringLangID[CUSTOMHID_SIZ_STRING_LANGID];
extern const uint8_t CustomHID_StringVendor[CUSTOMHID_SIZ_STRING_VENDOR];
extern const uint8_t CustomHID_StringProduct[CUSTOMHID_SIZ_STRING_PRODUCT];
extern uint8_t CustomHID_StringSerial[CUSTOMHID_SIZ_STRING_SERIAL];
extern const uint8_t CustomHID_StringInterface[CUSTOMHID_SIZ_STRING_PRODUCT];

//
uint16_t getSizeReportDescriptor(void);

#endif /* __USB_DESC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

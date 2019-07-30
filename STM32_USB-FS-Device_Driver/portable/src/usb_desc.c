/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  Gerasimchuk A.
  * @version V41.0.0
  * @date    14-January-2017
  * @brief   Descriptors for Custom HID
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "usb_desc.h"
#include "usb_user_setings.h"

/*********************************************************************************/
/*******************          REPORT DESCRIPTOR          *************************/
/*********************************************************************************/
const uint8_t CustomHID_ReportDescriptor[] ={
		  0x06, 0xFF, 0x00,      /* USAGE_PAGE (Vendor Page: 0xFF00) */
		    0x09, 0x01,            /* USAGE                          */
		    0xa1, 0x01,            /* COLLECTION (Application)       */
		    /* 6 */

		    /* Out */
//		    0x85, 0x01,            /*     REPORT_ID (1)              */
//		    0x09, 0x01,            /*     USAGE (Out)	             */
		    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
		    0x26, 0xff, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
		    0x75, 0x08,            /*     REPORT_SIZE (8)            */
		    0x95, EP_COUNT,            /*     REPORT_COUNT               */
//		    0x91, 0x82,            /*     OUTPUT (Data,Var,Abs,Vol)  */
		    0x09, 0x01,            /* USAGE */

            0x81, 0b10,            /*     INPUT (Data,Var,Abs,Vol)   */
		    0x95, EP_COUNT,            /*     REPORT_COUNT               */
		    0x09, 0x01,            /* USAGE */
		    0x91, 0b10,            /*     OUTPUT (Data,Var,Abs,Vol)  */
		    /* In */
//		    0x85, 0x07,            /*     REPORT_ID (7)              */
//		    0x09, 0x07,            /*     USAGE (IN)             */
//		    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
//		    0x26, 0xff, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
//		    0x75, 0x08,            /*     REPORT_SIZE (8)            */
//		    0x95, EP_SIZE,            /*     REPORT_COUNT              */
//		    0x81, 0x82,            /*     INPUT (Data,Var,Abs,Vol)   */
		    /* 161 */

		    0xc0 	          /*     END_COLLECTION	             */
};

/*********************************************************************************/
/*******************          DEVICE DESCRIPTOR          *************************/
/*********************************************************************************/

const sDeviceDescriptor CustomHID_DeviceDescriptor = {
	.bLength = sizeof(sDeviceDescriptor),
	.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize = 0x40,
	.idVendor = 0x0483,
	.idProduct = 0x5711,
	.bcdDevice = 0x0200,   // diff
	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,
	.bNumConfigurations = 0x01
};



/*********************************************************************************/
/*******************       CONFIGURATION DESCRIPTOR       ************************/
/*******************         INTERFACE DESCRIPTOR         ************************/
/*******************            HID DESCRIPTOR            ************************/
/*******************          ENDPOIN DESCRIPTOR          ************************/
/*********************************************************************************/

const sUSBHIDDescriptor CustomHID_ConfigDescriptor = {
		/********Configuration descriptor**************/
		.ConfigurationDescriptor = {
									.bLength = sizeof(sConfigurationDescriptor),
									.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,
									.wTotalLength = sizeof(sUSBHIDDescriptor),
									.bNumInterfaces = 0x01,
									.bConfigurationValue = 0x01,
									.iConfiguration = 0x00,
									.bmAttributes = (uint8_t)0xC0,
									.bMaxPower = 0x32,  /* MaxPower 100 mA: this current is used for detecting Vbus */
		},
		/********Interface #1 descriptor**************/
		.userDesc.sInetrfaceDesc = {
									.bLength = sizeof(sInetrfaceDescriptor),
									.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
									.bInterfaceNumber = 0x00,
									.bAlternateSetting = 0x00,
									.bNumEndpoints = EP_NUM,
									.bInterfaceClass = 0x03,    /* bInterfaceClass: HID */
									.bInterfaceSubClass = 0x00,
									.bInterfaceProtocol = 0x00, // ?
									.iInterface = 0x04,
		},
		/********USB HID descriptor***************/
		.userDesc.HIDDesc = {
									.bLength = sizeof(sHIDDescriptor),
									.bDescriptorType = HID_DESCRIPTOR_TYPE,
									.bcdHID = 0x0111,
									.bCountryCode = 0x00,
									.bNumDescriptors = 0x01,   /* bNumDescriptors: Number of HID class descriptors to follow */
									.bDescriptorType1 = 0x22, // ?
									.wDescriptorLength = sizeof(CustomHID_ReportDescriptor), /*  Total length of Report descriptor */
		},
		/********USB EndPoint descriptor***************/
		.userDesc.EndPointDesc[0] = {
									.bLength = sizeof(sEndPointDescriptor),          /* bLength: Endpoint Descriptor size */
									.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE,
									.bEndpointAddress = 0x81,   /* bEndpointAddress: Endpoint Address (IN) */
									.bmAttributes = 0x03,       /* bmAttributes: Interrupt endpoint */
									.wMaxPacketSize = EP_COUNT,   /* wMaxPacketSize: size Bytes max */
									.bInterval = 1,           /* bInterval: Polling Interval, ms */
		},
		.userDesc.EndPointDesc[1] = {
									.bLength = sizeof(sEndPointDescriptor),          /* bLength: Endpoint Descriptor size */
									.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE,
									.bEndpointAddress = 0x01,   /* bEndpointAddress: Endpoint Address (OUT) */
									.bmAttributes = 0x03,       /* bmAttributes: Interrupt endpoint */
									.wMaxPacketSize = EP_COUNT,   /* wMaxPacketSize: size Bytes max */
									.bInterval = 1,           /* bInterval: Polling Interval, ms */
		}
};



/*********************************************************************************/
/*******************          STRING DESCRIPTOR          *************************/
/*********************************************************************************/

const uint8_t CustomHID_StringLangID[CUSTOMHID_SIZ_STRING_LANGID] =
  {
    CUSTOMHID_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  }; /* LangID = 0x0409: U.S. English */

const uint8_t CustomHID_StringVendor[CUSTOMHID_SIZ_STRING_VENDOR] =
  {
    CUSTOMHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'N', 0, 'T', 0, 'C', 0, ' ', 0, 'E', 0, 'N', 0, 'E', 0, 'R', 0,
    'G', 0, 'O', 0, 'S', 0, 'V', 0, ' ', 0, 'L', 0, 'I', 0, 'M', 0,
    '.', 0, ' ', 0
  };

const uint8_t CustomHID_StringProduct[CUSTOMHID_SIZ_STRING_PRODUCT] =
  {
    CUSTOMHID_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'I', 0, 'N', 0, 'F', 0, 'O', 0, 'R', 0, 'M', 0, 'P', 0,
    ' ', 0, '0', 0, '0', 0, '1', 0, ' ', 0, ' ', 0, ' ', 0,
    ' ', 0
  };
uint8_t CustomHID_StringSerial[CUSTOMHID_SIZ_STRING_SERIAL] =
  {
    CUSTOMHID_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0,
  };

const uint8_t CustomHID_StringInterface[CUSTOMHID_SIZ_STRING_PRODUCT] =
  {
    CUSTOMHID_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'B', 0, 'l', 0, 'u', 0, 'm', 0, ' ', 0, 'B', 0, 'l', 0,
    'u', 0, 'm', 0, ' ', 0, 'C', 0, ' ', 0, 'H', 0, 'I', 0,
    'D', 0
  };


uint16_t getSizeReportDescriptor(void){

	return sizeof(CustomHID_ReportDescriptor);
}

/*****************************END OF FILE****/


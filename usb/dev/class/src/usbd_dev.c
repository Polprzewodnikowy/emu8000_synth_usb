/*
 * usbd_dev.c
 *
 *  Created on: 31-03-2013
 *      Author: Polprzewodnikowy
 */

#include <string.h>
#include "usbd_dev.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "event.h"
#include "misc/led.h"
#include "misc/midiuart.h"
#include "misc/usbcontrol.h"
#include "sound/emu8000/emu8000_midi_bridge.h"

extern USB_OTG_CORE_HANDLE USB_Dev;
static uint32_t usb_altsetting = 0;
static uint8_t data_buf[64];
static uint8_t midi_buf[64];
const uint8_t cin_size[16] =
{0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};

static uint8_t usbd_Init(void *pdev, uint8_t cfgidx);
static uint8_t usbd_DeInit(void *pdev, uint8_t cfgidx);
static uint8_t usbd_Setup(void *pdev, USB_SETUP_REQ *req);
static uint8_t usbd_EP0_TxSent(void *pdev);
static uint8_t usbd_EP0_RxReady(void *pdev);
static uint8_t usbd_DataIn(void *pdev, uint8_t epnum);
static uint8_t usbd_DataOut(void *pdev, uint8_t epnum);
static uint8_t usbd_SOF(void *pdev);
static uint8_t usbd_IsoINIncomplete(void *pdev);
static uint8_t usbd_IsoOUTIncomplete(void *pdev);
static uint8_t *usbd_GetConfigDescriptor(uint8_t speed, uint16_t *length);
static uint8_t *usbd_GetUsrStrDescriptor(uint8_t speed, uint8_t index, uint16_t *length);

USBD_Class_cb_TypeDef USBD_cb =
{
	usbd_Init,
	usbd_DeInit,
	usbd_Setup,
	usbd_EP0_TxSent,
	usbd_EP0_RxReady,
	usbd_DataIn,
	usbd_DataOut,
	usbd_SOF,
	usbd_IsoINIncomplete,
	usbd_IsoOUTIncomplete,
	usbd_GetConfigDescriptor,
	usbd_GetUsrStrDescriptor, };

static uint8_t usbd_ConfigDescriptor[133 + 23] =
{
//Configuration descriptor
	0x09,//bLength
	0x02,	//bDescriptorType
	LOBYTE(sizeof(usbd_ConfigDescriptor)),	//wTotalLength[0]
	HIBYTE(sizeof(usbd_ConfigDescriptor)),	//wTotalLength[1]
	0x03,	//bNumInterfaces
	0x01,	//bNumConfigurationValue
	0x06,	//iConfiguration
	0xE0,	//bmAttributes
	0x32,	//MaxPower

	//--------------------------------------------------

	//--------------------   MIDI   --------------------

	//--------------------------------------------------

	//Standard AC interface descriptor
	0x09,//bLength
	0x04,	//bDescriptorType
	0x00,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x00,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x01,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x07,	//iInterface

	//Class-specific AC interface descriptor
	0x09,//bLength
	0x24,	//bDescriptorType
	0x01,	//bDescriptorSubType
	LOBYTE(0x0100),	//bcdADC[0]
	HIBYTE(0x0100),	//bcdADC[1]
	LOBYTE(0x0009),	//wTotalLength[0]
	HIBYTE(0x0009),	//wTotalLength[1]
	0x01,	//blnCollection
	0x01,	//baInterfaceNumber

	//--------------------------------------------------

	//Standard MS interface descriptor
	0x09,//bLength
	0x04,	//bDescriptorType
	0x01,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x02,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x03,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x08,	//iInterface

	//Class-specific MS interface descriptor
	0x07,//bLength
	0x24,	//bDescriptorType
	0x01,	//bDescriptorSubType
	LOBYTE(0x0100),	//bcdADC[0]
	HIBYTE(0x0100),	//bcdADC[1]
	LOBYTE(0x0061),	//wTotalLength[0]
	HIBYTE(0x0061),	//wTotalLength[1]

	//--------------------------------------------------

	//MIDI IN jack descriptor (embedded)
	0x06,//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x01,	//bJackType
	0x01,	//bJackID
	0x08,	//iJack

	//MIDI OUT jack descriptor (external)
	0x09,//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x02,	//bJackType
	0x02,	//bJackID
	0x01,	//bNumberInputPins
	0x01,	//baSourceID
	0x01,	//baSourcePin
	0x08,	//iJack

	//--------------------------------------------------

	//MIDI IN jack descriptor (embedded)
	0x06,//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x01,	//bJackType
	0x03,	//bJackID
	0x08,	//iJack

	//MIDI OUT jack descriptor (external)
	0x09,//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x02,	//bJackType
	0x04,	//bJackID
	0x01,	//bNumberInputPins
	0x03,	//baSourceID
	0x01,	//baSourcePin
	0x08,	//iJack

	//--------------------------------------------------

	//MIDI OUT jack descriptor (embedded)
	0x09,//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x01,	//bJackType
	0x05,	//bJackID
	0x01,	//bNumberInputPins
	0x06,	//baSourceID
	0x01,	//baSourcePin
	0x08,	//iJack

	//MIDI IN jack descriptor (external)
	0x06,//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x02,	//bJackType
	0x06,	//bJackID
	0x08,	//iJack

	//--------------------------------------------------

	//MIDI OUT jack descriptor (embedded)
	0x09,//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x01,	//bJackType
	0x07,	//bJackID
	0x01,	//bNumberInputPins
	0x08,	//baSourceID
	0x01,	//baSourcePin
	0x08,	//iJack

	//MIDI IN jack descriptor (external)
	0x06,//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x02,	//bJackType
	0x08,	//bJackID
	0x08,	//iJack

	//--------------------------------------------------

	//Standard bulk OUT endpoint descriptor
	0x09,//bLength
	0x05,	//bDescriptorType
	0x02,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk OUT endpoint descriptor
	0x06,//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x02,	//bNumberEmbeddedMIDIJack
	0x01,	//baAssocJackID[0]
	0x03,	//baAssocJackID[1]

	//--------------------------------------------------

	//Standard bulk IN endpoint descriptor
	0x09,//bLength
	0x05,	//bDescriptorType
	0x82,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk IN endpoint descriptor
	0x06,//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x02,	//bNumberEmbeddedMIDIJack
	0x05,	//baAssocJackID[0]
	0x07,	//baAssocJackID[1]

	//--------------------------------------------------

	//--------------------  WinUSB  --------------------

	//--------------------------------------------------

	//Interface descriptor
	0x09,	//bLength
	0x04,	//bDescriptorType
	0x02,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x02,	//bNumEndpoints
	0xFF,	//bInterfaceClass
	0xFF,	//bInterfaceSubClass
	0xFF,	//bInterfaceProtocol
	0x09,	//iInterface

	//OUT endpoint descriptor
	0x07,	//bLength
	0x05,	//bDescriptorType
	0x01,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval

	//IN endpoint descriptor
	0x07,	//bLength
	0x05,	//bDescriptorType
	0x81,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval
};

static uint8_t usbd_Init(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Open(pdev, 0x02, 64, USB_OTG_EP_BULK);
	DCD_EP_Open(pdev, 0x82, 64, USB_OTG_EP_BULK);
	DCD_EP_Open(pdev, 0x01, 64, USB_OTG_EP_BULK);
	DCD_EP_Open(pdev, 0x81, 64, USB_OTG_EP_BULK);
	DCD_EP_PrepareRx(pdev, 0x02, midi_buf, 64);
	DCD_EP_PrepareRx(pdev, 0x01, data_buf, 64);
	return USBD_OK;
}

static uint8_t usbd_DeInit(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Close(pdev, 0x02);
	DCD_EP_Close(pdev, 0x82);
	DCD_EP_Close(pdev, 0x01);
	DCD_EP_Close(pdev, 0x81);
	return USBD_OK;
}

static uint8_t usbd_Setup(void *pdev, USB_SETUP_REQ *req)
{
	uint16_t len = 0;
	uint8_t *pbuf = NULL;

	switch(req->bmRequest & USB_REQ_TYPE_MASK)
	{
		case USB_REQ_TYPE_CLASS:
			switch(req->bRequest)
			{
				default:
					USBD_CtlError(pdev, req);
					return USBD_FAIL;
			}
			break;
		case USB_REQ_TYPE_STANDARD:
			switch(req->bRequest)
			{
				case USB_REQ_GET_DESCRIPTOR:
					if(req->wValue >> 8 == 0x24)		//MIDI_DESCRIPTOR_TYPE
					{
						pbuf = usbd_ConfigDescriptor + 36;
						len = MIN(sizeof(usbd_ConfigDescriptor), req->wLength);
					}
					USBD_CtlSendData(pdev, pbuf, len);
					break;
				case USB_REQ_SET_INTERFACE:
					usb_altsetting = (uint8_t)(req->wValue);
					break;
				case USB_REQ_GET_INTERFACE:
					USBD_CtlSendData(pdev, (uint8_t *)&usb_altsetting, 1);
					break;
				default:
					USBD_CtlError(pdev, req);
					return USBD_FAIL;
			}
	}
	return USBD_OK;
}

static uint8_t usbd_EP0_TxSent(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_EP0_RxReady(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_DataIn(void *pdev, uint8_t epnum)
{
	pdev = pdev;
	epnum = epnum;
	return USBD_OK;
}

static uint8_t usbd_DataOut(void *pdev, uint8_t epnum)
{
	event_t ev;

	if(epnum == 0x01)
	{
		ev.type = EVENT_TYPE_USB;
		memcpy(ev.buf, data_buf, 64);
		event_put(ev);
		DCD_EP_PrepareRx(pdev, 0x01, data_buf, 64);
	}
	if(epnum == 0x02)
	{
		for(int i = 0; midi_buf[i] != 0; i += 4)
		{
			switch(midi_buf[i] >> 4)
			{
				case 0:
					ev.type = EVENT_TYPE_MIDI;
					memcpy(ev.buf, &midi_buf[i + 1], 3);
					event_put(ev);
					break;
				case 1:
					usart_midi_TxPacket(&midi_buf[i + 1], cin_size[midi_buf[i] & 0x0F]);
					break;
				default:
					break;
			}
		}
		memset(midi_buf, 0, 64);
		DCD_EP_PrepareRx(pdev, 0x02, midi_buf, 64);
	}
	return USBD_OK;
}

static uint8_t usbd_SOF(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_IsoINIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_IsoOUTIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t *usbd_GetConfigDescriptor(uint8_t speed, uint16_t *length)
{
	speed = speed;
	*length = sizeof(usbd_ConfigDescriptor);
	return usbd_ConfigDescriptor;
}

static uint8_t *usbd_GetUsrStrDescriptor(uint8_t speed, uint8_t index, uint16_t *length)
{
	speed = speed;
	switch(index)
	{
		case 0x06:
			USBD_GetString((uint8_t *)"EMU8000 USB Synth", USBD_StrDesc, length);
			break;
		case 0x07:
			USBD_GetString((uint8_t *)"EMU8000", USBD_StrDesc, length);
			break;
		case 0x08:
			USBD_GetString((uint8_t *)"EMU8000 MIDI", USBD_StrDesc, length);
			break;
		case 0x09:
			USBD_GetString((uint8_t *)"EMU8000 Control & Debug", USBD_StrDesc, length);
			break;
	}
	return USBD_StrDesc;
}

void usbd_TxPacket(uint8_t epnum, uint8_t *buf, int len)
{
	if(USB_Dev.dev.device_status == USB_OTG_CONFIGURED)
		DCD_EP_Tx(&USB_Dev, epnum, buf, len);
}

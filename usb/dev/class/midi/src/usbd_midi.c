/*
 * usbd_midi.c
 *
 *  Created on: 31-03-2013
 *      Author: Polprzewodnikowy
 */

#include <string.h>
#include "usbd_midi.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "misc/led.h"
#include "sound/emu8000/emu8000_midi_bridge.h"

static volatile uint32_t usbd_midi_AlternateSetting = 0;
static uint8_t MIDI_Buf[64];

const uint8_t MIDI_CIN_Size[16] = {
	0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
};

static uint8_t usbd_midi_Init(void *pdev, uint8_t cfgidx);
static uint8_t usbd_midi_DeInit(void *pdev, uint8_t cfgidx);
static uint8_t usbd_midi_Setup(void *pdev, USB_SETUP_REQ *req);
static uint8_t usbd_midi_EP0_TxSent(void *pdev);
static uint8_t usbd_midi_EP0_RxReady(void *pdev);
static uint8_t usbd_midi_DataIn(void *pdev, uint8_t epnum);
static uint8_t usbd_midi_DataOut(void *pdev, uint8_t epnum);
static uint8_t usbd_midi_SOF(void *pdev);
static uint8_t usbd_midi_IsoINIncomplete(void *pdev);
static uint8_t usbd_midi_IsoOUTIncomplete(void *pdev);
static uint8_t *usbd_midi_GetConfigDescriptor(uint8_t speed, uint16_t *length);

void usbd_midi_Parse(uint8_t *buf);

USBD_Class_cb_TypeDef MIDI_cb =
{
	usbd_midi_Init,
	usbd_midi_DeInit,
	usbd_midi_Setup,
	usbd_midi_EP0_TxSent,
	usbd_midi_EP0_RxReady,
	usbd_midi_DataIn,
	usbd_midi_DataOut,
	usbd_midi_SOF,
	usbd_midi_IsoINIncomplete,
	usbd_midi_IsoOUTIncomplete,
	usbd_midi_GetConfigDescriptor,
};

static uint8_t usbd_midi_ConfigDescriptor[101] =
{
	//Configuration descriptor
	0x09,	//bLength
	0x02,	//bDescriptorType
	LOBYTE(sizeof(usbd_midi_ConfigDescriptor)),	//wTotalLength[0]
	HIBYTE(sizeof(usbd_midi_ConfigDescriptor)),	//wTotalLength[1]
	0x02,	//bNumInterfaces
	0x01,	//bNumConfigurationValue
	0x00,	//iConfiguration
	0x80,	//bmAttributes
	0x32,	//MaxPower

	//Standard AC interface descriptor
	0x09,	//bLength
	0x04,	//bDescriptorType
	0x00,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x00,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x01,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x00,	//iInterface

	//Class-specific AC interface descriptor
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x01,	//bDescriptorSubType
	LOBYTE(0x0100),	//bcdADC[0]
	HIBYTE(0x0100),	//bcdADC[1]
	LOBYTE(0x0009),	//wTotalLength[0]
	HIBYTE(0x0009),	//wTotalLength[1]
	0x01,	//blnCollection
	0x01,	//baInterfaceNumber

	//Standard MS interface descriptor
	0x09,	//bLength
	0x04,	//bDescriptorType
	0x01,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x02,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x03,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x00,	//iInterface

	//Class-specific MS interface descriptor
	0x07,	//bLength
	0x24,	//bDescriptorType
	0x01,	//bDescriptorSubType
	LOBYTE(0x0100),	//bcdADC[0]
	HIBYTE(0x0100),	//bcdADC[1]
	LOBYTE(0x0041),	//wTotalLength[0]
	HIBYTE(0x0041),	//wTotalLength[1]

	//MIDI IN jack descriptor (embedded)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x01,	//bJackType
	0x01,	//bJackID
	0x00,	//iJack

	//MIDI IN jack descriptor (external)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x02,	//bJackType
	0x02,	//bJackID
	0x00,	//iJack

	//MIDI OUT jack descriptor (embedded)
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x01,	//bJackType
	0x03,	//bJackID
	0x01,	//bNumberInputPins
	0x02,	//baSourceID
	0x01,	//baSourcePin
	0x00,	//iJack

	//MIDI OUT jack descriptor (external)
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x02,	//bJackType
	0x04,	//bJackID
	0x01,	//bNumberInputPins
	0x01,	//baSourceID
	0x01,	//baSourcePin
	0x00,	//iJack

	//Standard bulk OUT endpoint descriptor
	0x09,	//bLength
	0x05,	//bDescriptorType
	0x01,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk OUT endpoint descriptor
	0x05,	//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x01,	//bNumberEmbeddedMIDIJack
	0x01,	//baAssocJackID

	//Standard bulk IN endpoint descriptor
	0x09,	//bLength
	0x05,	//bDescriptorType
	0x81,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0040),	//wMaxPacketSize[0]
	HIBYTE(0x0040),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk IN endpoint descriptor
	0x05,	//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x01,	//bNumberEmbeddedMIDIJack
	0x03,	//baAssocJackID
};

static uint8_t usbd_midi_Init(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Open(pdev, 0x81, 64, USB_OTG_EP_BULK);
	DCD_EP_Open(pdev, 0x01, 64, USB_OTG_EP_BULK);
	DCD_EP_PrepareRx(pdev, 0x01, MIDI_Buf, 64);
	bridge_init();
	return USBD_OK;
}

static uint8_t usbd_midi_DeInit(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Close(pdev, 0x81);
	DCD_EP_Close(pdev, 0x01);
	bridge_init();
	return USBD_OK;
}

static uint8_t usbd_midi_Setup(void *pdev, USB_SETUP_REQ *req)
{
	uint16_t len = sizeof(usbd_midi_ConfigDescriptor);
	uint8_t *pbuf = usbd_midi_ConfigDescriptor + 9;

	switch(req->bmRequest & USB_REQ_TYPE_MASK)
	{
		case USB_REQ_TYPE_STANDARD:
			switch(req->bRequest)
			{
				case USB_REQ_GET_DESCRIPTOR:
					if((req->wValue >> 8) == 0x24)
						len = MIN(len , req->wLength);
					USBD_CtlSendData(pdev, pbuf, len);
					break;
				case USB_REQ_GET_INTERFACE:
					USBD_CtlSendData(pdev, (uint8_t *)&usbd_midi_AlternateSetting, 1);
					break;
				case USB_REQ_SET_INTERFACE:
					if((uint8_t)(req->wValue) < 2)
						usbd_midi_AlternateSetting = (uint8_t)(req->wValue);
					else
						USBD_CtlError(pdev, req);
					break;
			}
			break;
		case USB_REQ_TYPE_CLASS:
			switch(req->bRequest)
			{
				default:
					USBD_CtlError(pdev, req);
			}
			break;
	}
	return USBD_OK;
}

static uint8_t usbd_midi_EP0_TxSent(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_midi_EP0_RxReady(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_midi_DataIn(void *pdev, uint8_t epnum)
{
	pdev = pdev;
	epnum = epnum;
	return USBD_OK;
}

static uint8_t usbd_midi_DataOut(void *pdev, uint8_t epnum)
{
	if(epnum == 0x01)
	{
		for(int i = 0; MIDI_Buf[i] != 0; i += 4)
			usbd_midi_Parse(&MIDI_Buf[1 + i]);
		memset(MIDI_Buf, 0, 64);
		DCD_EP_PrepareRx(pdev, 0x01, MIDI_Buf, 64);
	}
	return USBD_OK;
}

static uint8_t usbd_midi_SOF(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_midi_IsoINIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_midi_IsoOUTIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t *usbd_midi_GetConfigDescriptor(uint8_t speed, uint16_t *length)
{
	speed = speed;
	*length = sizeof(usbd_midi_ConfigDescriptor);
	return usbd_midi_ConfigDescriptor;
}

void usbd_midi_Parse(uint8_t *buf)
{
	switch(buf[0] & 0xF0)
	{
		case MIDI_NOTE_OFF:
			note_off(buf[0] & 0x0F, buf[1], buf[2]);
			break;
		case MIDI_NOTE_ON:
			note_on(buf[0] & 0x0F, buf[1], buf[2]);
			break;
		case MIDI_AFTERTOUCH_POLY:
			aftertouch_poly(buf[0] & 0x0F, buf[1], buf[2]);
			break;
		case MIDI_CONTROL_CHANGE:
			control_change(buf[0] & 0x0F, buf[1], buf[2]);
			break;
		case MIDI_PROGRAM_CHANGE:
			program_change(buf[0] & 0x0F, buf[1]);
			break;
		case MIDI_AFTERTOUCH_CHAN:
			aftertouch_chan(buf[0] & 0x0F, buf[1]);
			break;
		case MIDI_PITCH_BEND:
			pitch_bend(buf[0] & 0x0F, buf[1], buf[2]);
			break;
		default:
			break;
	}
	if((buf[0] & 0xF0) == MIDI_NOTE_ON)
		led_blink(LED_G, 2);
	else if((buf[0] & 0xF0) == MIDI_NOTE_OFF)
		led_blink(LED_R, 2);
	else
		led_blink(LED_B, 2);
}

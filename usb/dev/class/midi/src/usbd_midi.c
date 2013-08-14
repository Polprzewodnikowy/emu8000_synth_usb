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
#include "misc/midiuart.h"
#include "sound/emu8000/emu8000_midi_bridge.h"

extern USB_OTG_CORE_HANDLE USB_Dev;
static uint8_t MIDI_Buf[4];
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
static uint8_t *usbd_midi_GetUsrStrDescriptor(uint8_t speed, uint8_t index, uint16_t *length);

void usbd_midi_Parse(uint8_t *buf);

USBD_Class_cb_TypeDef MIDI_cb = {
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
	usbd_midi_GetUsrStrDescriptor,
};

static uint8_t usbd_midi_ConfigDescriptor[133] = {
	//Configuration descriptor
	0x09,	//bLength
	0x02,	//bDescriptorType
	LOBYTE(sizeof(usbd_midi_ConfigDescriptor)),	//wTotalLength[0]
	HIBYTE(sizeof(usbd_midi_ConfigDescriptor)),	//wTotalLength[1]
	0x02,	//bNumInterfaces
	0x01,	//bNumConfigurationValue
	0x04,	//iConfiguration
	0x80,	//bmAttributes
	0x32,	//MaxPower

	//--------------------------------------------------

	//Standard AC interface descriptor
	0x09,	//bLength
	0x04,	//bDescriptorType
	0x00,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x00,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x01,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x06,	//iInterface

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

	//--------------------------------------------------

	//Standard MS interface descriptor
	0x09,	//bLength
	0x04,	//bDescriptorType
	0x01,	//bInterfaceNumber
	0x00,	//bAlternateSetting
	0x02,	//bNumEndpoints
	0x01,	//bInterfaceClass
	0x03,	//bInterfaceSubClass
	0x00,	//bInterfaceProtocol
	0x05,	//iInterface

	//Class-specific MS interface descriptor
	0x07,	//bLength
	0x24,	//bDescriptorType
	0x01,	//bDescriptorSubType
	LOBYTE(0x0100),	//bcdADC[0]
	HIBYTE(0x0100),	//bcdADC[1]
	LOBYTE(0x0061),	//wTotalLength[0]
	HIBYTE(0x0061),	//wTotalLength[1]

	//--------------------------------------------------

	//MIDI IN jack descriptor (embedded)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x01,	//bJackType
	0x01,	//bJackID
	0x07,	//iJack

	//MIDI OUT jack descriptor (external)
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x02,	//bJackType
	0x02,	//bJackID
	0x01,	//bNumberInputPins
	0x01,	//baSourceID
	0x01,	//baSourcePin
	0x07,	//iJack

	//--------------------------------------------------

	//MIDI IN jack descriptor (embedded)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x01,	//bJackType
	0x03,	//bJackID
	0x08,	//iJack

	//MIDI OUT jack descriptor (external)
	0x09,	//bLength
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
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x01,	//bJackType
	0x05,	//bJackID
	0x01,	//bNumberInputPins
	0x06,	//baSourceID
	0x01,	//baSourcePin
	0x07,	//iJack

	//MIDI IN jack descriptor (external)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x02,	//bJackType
	0x06,	//bJackID
	0x07,	//iJack

	//--------------------------------------------------

	//MIDI OUT jack descriptor (embedded)
	0x09,	//bLength
	0x24,	//bDescriptorType
	0x03,	//bDescriptorSubType
	0x01,	//bJackType
	0x07,	//bJackID
	0x01,	//bNumberInputPins
	0x08,	//baSourceID
	0x01,	//baSourcePin
	0x08,	//iJack

	//MIDI IN jack descriptor (external)
	0x06,	//bLength
	0x24,	//bDescriptorType
	0x02,	//bDescriptorSubType
	0x02,	//bJackType
	0x08,	//bJackID
	0x08,	//iJack

	//--------------------------------------------------

	//Standard bulk OUT endpoint descriptor
	0x09,	//bLength
	0x05,	//bDescriptorType
	0x01,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0004),	//wMaxPacketSize[0]
	HIBYTE(0x0004),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk OUT endpoint descriptor
	0x06,	//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x02,	//bNumberEmbeddedMIDIJack
	0x01,	//baAssocJackID[0]
	0x03,	//baAssocJackID[1]

	//--------------------------------------------------

	//Standard bulk IN endpoint descriptor
	0x09,	//bLength
	0x05,	//bDescriptorType
	0x81,	//bEndpointAddress
	0x02,	//bmAttributes
	LOBYTE(0x0004),	//wMaxPacketSize[0]
	HIBYTE(0x0004),	//wMaxPacketSize[1]
	0x00,	//bInterval
	0x00,	//bRefresh
	0x00,	//bSynchAddress

	//Class-specific MS bulk IN endpoint descriptor
	0x06,	//bLength
	0x25,	//bDescriptorType
	0x01,	//bDescriptorSubType
	0x02,	//bNumberEmbeddedMIDIJack
	0x05,	//baAssocJackID[0]
	0x07,	//baAssocJackID[1]
};

static uint8_t usbd_midi_Init(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Open(pdev, 0x81, 4, USB_OTG_EP_BULK);
	DCD_EP_Open(pdev, 0x01, 4, USB_OTG_EP_BULK);
	DCD_EP_PrepareRx(pdev, 0x01, MIDI_Buf, 4);
	bridge_init();
	usart_midi_Init(31250);
	return USBD_OK;
}

static uint8_t usbd_midi_DeInit(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Close(pdev, 0x81);
	DCD_EP_Close(pdev, 0x01);
	bridge_init();
	usart_midi_DeInit();
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
				default:
					USBD_CtlError(pdev, req);
					break;
			}
			break;
		case USB_REQ_TYPE_CLASS:
			switch(req->bRequest)
			{
				default:
					USBD_CtlError(pdev, req);
					break;
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
		switch(MIDI_Buf[0]>>4)
		{
			case 0:
				usbd_midi_Parse(&MIDI_Buf[1]);
				break;
			case 1:
				usart_midi_TxPacket(&MIDI_Buf[1], MIDI_CIN_Size[MIDI_Buf[0] & 0x0F]);
				break;
			default:
				break;
		}
		memset(MIDI_Buf, 0, 4);
		DCD_EP_PrepareRx(pdev, 0x01, MIDI_Buf, 4);
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

static uint8_t *usbd_midi_GetUsrStrDescriptor(uint8_t speed, uint8_t index, uint16_t *length)
{
	speed = speed;
	switch(index)
	{
		case 6:
			USBD_GetString((uint8_t *)"EMU8000", USBD_StrDesc, length);
			break;
		case 7:
			USBD_GetString((uint8_t *)"EMU8000 MIDI", USBD_StrDesc, length);
			break;
		case 8:
			USBD_GetString((uint8_t *)"USB - MIDI", USBD_StrDesc, length);
			break;
		default:
			USBD_GetString((uint8_t *)"Error", USBD_StrDesc, length);
	}
	return USBD_StrDesc;
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

void usbd_midi_TxPacket(uint8_t interface, uint8_t cmd, uint8_t *data)
{
	uint8_t buf[4];
	buf[0] = (interface<<4) | (cmd>>4);
	buf[1] = cmd;
	buf[2] = data[0];
	buf[3] = data[1];
	DCD_EP_Tx(&USB_Dev, 0x81, buf, 4);
}

/*
 * usbcontrol.c
 *
 *  Created on: 21-08-2013
 *      Author: Polprzewodnikowy
 */

#include <usbd_dev.h>
#include "misc/led.h"
#include "misc/usbcontrol.h"

uint8_t response[64];
uint8_t current_settings[16] = {
	USBCONTROL_RESPONSE,
	23<<3, 23<<3,	//Master
	31<<3, 31<<3,	//Voice
	31<<3, 31<<3,	//MIDI
	8, 8,			//Mixer Bass & Treble
	0,				//3DSE
	8, 9,			//Synth Bass & Treble
	5,				//Reverb
	2,				//Chorus
	0, 0			//Unused
};

void usbcontrol_process_data(uint8_t *data)
{
	switch(data[0])
	{
		case USBCONTROL_CONNECTED:
			led_blink(LED_B, 50);
			break;
		case USBCONTROL_DISCONNECTED:
			led_blink(LED_O, 50);
			break;
		case USBCONTROL_SET_MIXER_FEATURE:
			switch(data[1])
			{
				case USBCONTROL_MIXER_MASTER:
					if(data[2] == USBCONTROL_MIXER_L)
					{
						sbmix_set_vol(SBMIX_MASTER_L, data[3]);
						current_settings[1] = data[3];
					}
					else if(data[2] == USBCONTROL_MIXER_R)
					{
						sbmix_set_vol(SBMIX_MASTER_R, data[3]);
						current_settings[2] = data[3];
					}
					break;
				case USBCONTROL_MIXER_VOICE:
					if(data[2] == USBCONTROL_MIXER_L)
					{
						sbmix_set_vol(SBMIX_VOICE_L, data[3]);
						current_settings[3] = data[3];
					}
					else if(data[2] == USBCONTROL_MIXER_R)
					{
						sbmix_set_vol(SBMIX_VOICE_R, data[3]);
						current_settings[4] = data[3];
					}
					break;
				case USBCONTROL_MIXER_MIDI:
					if(data[2] == USBCONTROL_MIXER_L)
					{
						sbmix_set_vol(SBMIX_MIDI_L, data[3]);
						current_settings[5] = data[3];
					}
					else if(data[2] == USBCONTROL_MIXER_R)
					{
						sbmix_set_vol(SBMIX_MIDI_R, data[3]);
						current_settings[6] = data[3];
					}
					break;
				case USBCONTROL_MIXER_BASS:
					if(data[2] == USBCONTROL_MIXER_L)
						sbmix_set_eq(SBMIX_BASS, SBMIX_LEFT, data[3]);
					else if(data[2] == USBCONTROL_MIXER_R)
						sbmix_set_eq(SBMIX_BASS, SBMIX_RIGHT, data[3]);
					current_settings[7] = data[3];
					break;
				case USBCONTROL_MIXER_TREBLE:
					if(data[2] == USBCONTROL_MIXER_L)
						sbmix_set_eq(SBMIX_TREBLE, SBMIX_LEFT, data[3]);
					else if(data[2] == USBCONTROL_MIXER_R)
						sbmix_set_eq(SBMIX_TREBLE, SBMIX_RIGHT, data[3]);
					current_settings[8] = data[3];
					break;
				case USBCONTROL_MIXER_3DSE:
					sbmix_write(SBMIX_3DSE, data[2]);
					current_settings[9] = data[2];
					break;
			}
			break;
		case USBCONTROL_SET_EMU8000_FEATURE:
			switch(data[1])
			{
				case USBCONTROL_EMU8000_EQ:
					emu8000_update_equalizer(data[2], data[3]);
					current_settings[10] = data[2];
					current_settings[11] = data[3];
					break;
				case USBCONTROL_EMU8000_REVERB:
					emu8000_update_reverb_mode(data[2]);
					current_settings[12] = data[2];
					break;
				case USBCONTROL_EMU8000_CHORUS:
					emu8000_update_chorus_mode(data[2]);
					current_settings[13] = data[2];
					break;
			}
			break;
		case USBCONTROL_GET_SETTINGS:
			usbd_TxPacket(0x81, current_settings, 16);
			return;
			break;

	}
	response[0] = USBCONTROL_RESPONSE;
	usbd_TxPacket(0x81, response, 1);
}

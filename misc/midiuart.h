/*
 * midiuart.h
 *
 *  Created on: 13-08-2013
 *      Author: Polprzewodnikowy
 */

#ifndef MIDIUART_H_
#define MIDIUART_H_

#define MIDI_STATUS_MASK		(1<<7)

#define MIDI_NOTE_OFF			0x80
#define MIDI_NOTE_ON			0x90
#define MIDI_AFTERTOUCH_POLY	0xA0
#define MIDI_CONTROL_CHANGE		0xB0
#define MIDI_PROGRAM_CHANGE		0xC0
#define MIDI_AFTERTOUCH_CHAN	0xD0
#define MIDI_PITCH_BEND			0xE0
#define MIDI_SYSEX				0xF0
#define MIDI_SYSEX_END			0xF7
#define MIDI_CLOCK				0xF8
#define MIDI_ACTIVE_SENSING		0xFE

void usart_midi_Init(uint32_t speed);
void usart_midi_DeInit(void);
void usart_midi_TxPacket(uint8_t *buf, uint8_t size);
void usart_midi_TxByte(uint8_t data);
void midiuart_process(uint8_t data);

#endif /* MIDIUART_H_ */

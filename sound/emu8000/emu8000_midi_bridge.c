/*
 * emu8000_midi_bridge.c
 *
 *  Created on: 02-08-2013
 *      Author: Polprzewodnikowy
 */

#include <math.h>
#include "sound/emu8000/emu8000.h"

SFInfo *sf;
emu8000_voice_t voice[EMU8000_DRAM_VOICES];
midi_ch_t channel[16];

void reset_enumerators(enumerators_t *s)
{
	for(int i = 0; i < End_Oper; i++)
	{
		s->set[i] = 0;
		s->ovr[i] = 0;
	}
}

void link_enumerators(enumerators_t *s, enumerators_t *t)
{
	for(int i = 0; i < End_Oper; i++)
		if(s->ovr[i])
		{
			t->set[i] = s->set[i];
			t->ovr[i] = 1;
		}
}

void parse_enumerators(enumerators_t *s, emu8000_reg_t *t)
{
	for(int i = 0; i < End_Oper; i++)
	{
		if(s->ovr[i])
		{
			switch(i)
			{
//				case Start_Address:
//				case End_Address:
//				case Start_Loop_Address:
//				case End_Loop_Address:
//				case Start_Address_Coarse:
				case LFO1_To_Pitch:
				{
					int16_t vibrato = (int16_t)s->set[i];
					if(vibrato > 1200)
						vibrato = 1200;
					if(vibrato < -1200)
						vibrato = -1200;

					t->lfo1_vibrato = (vibrato * 127) / 1200;
					break;
				}
				case LFO2_To_Pitch:
				{
					int16_t vibrato = (int16_t)s->set[i];
					if(vibrato > 1200)
						vibrato = 1200;
					if(vibrato < -1200)
						vibrato = -1200;

					t->lfo2_vibrato = (vibrato * 127) / 1200;
					break;
				}
				case Env1_To_Pitch:
				{
					int16_t pitch_height = (int16_t)s->set[i];
					if(pitch_height > 1200)
						pitch_height = 1200;
					if(pitch_height < -1200)
						pitch_height = -1200;

					t->pitch_height = (pitch_height * 127) / 1200;
					break;
				}
				case Initial_Filter_Cutoff:
				{
					int16_t cutoff = (int16_t)s->set[i];
					if(cutoff == 0) /* no cutoff */
					{
						cutoff = 0xFF;
					}else{
						//cutoff = abscents / 25 - 189;
						cutoff = (cutoff - 4721) / 25;	//4721=125Hz
						if(cutoff < 0)
							cutoff = 0;
						if(cutoff > 255)
							cutoff = 255;
					}

					t->initial_cutoff = cutoff;
					break;
				}
				case Initial_Filter_Q:
				{
					int16_t q = (int16_t)s->set[i];
					if(q > 240)
						q = 240;
					if(q < 0)
						q = 0;

					t->filter_q = q / 16;
					break;
				}
				case LFO1_To_Filter_Cutoff:
				{
					int16_t shift = (int16_t)s->set[i];
					if(shift > 3600)
						shift = 3600;
					if(shift < -3600)
						shift = -3600;

					t->lfo1_cutoff = (shift * 127) / 3600;
					break;
				}
				case Env1_To_Filter_Cutoff:
				{
					int16_t shift = (int16_t)s->set[i];
					if(shift > 7200)
						shift = 7200;
					if(shift < -7200)
						shift = -7200;

					t->cutoff_height = (shift * 127) / 7200;
					break;
				}
//				case End_Address_Coarse:
				case LFO1_To_Volume:
				{
					int16_t tremolo = (int16_t)s->set[i];
					if(tremolo > 120)
						tremolo = 120;
					if(tremolo < -120)
						tremolo = -120;

					t->lfo1_tremolo = (tremolo * 127) / 120;
					break;
				}
//				case Env2_To_Volume:
				case Chorus_Send:
				{
					int16_t chorus = (int16_t)s->set[i];
					if(chorus > 1000)
						chorus = 1000;
					if(chorus < 0)
						chorus = 0;

					t->chorus = (chorus * 255) / 1000;
					break;
				}
				case Reverb_Send:
				{
					int16_t reverb = (int16_t)s->set[i];
					if(reverb > 1000)
						reverb = 1000;
					if(reverb < 0)
						reverb = 0;

					t->reverb = (reverb * 255) / 1000;
					break;
				}
				case Pan_Send:
				{
					int16_t pan = (int16_t)s->set[i];
					if(pan > 500)
						pan = 500;
					if(pan < -500)
						pan = -500;

					t->pan = 0xFF - (((pan + 500) * 255) / 1000);
					break;
				}
//				case Aux_Send:
//				case Sample_Volume:
				case LFO1_Delay:
				{
					int delay = (int16_t)s->set[i];
					delay = 1000 * pow(2.0, (double)delay / 1200.0);

					t->lfo1_delay = 0x8000 - ((delay * 1000) / 725);
					break;
				}
				case LFO1_Freq:
				{
					int freq = (int16_t)s->set[i];
					freq = (8176.0 * pow(2.0, (double)freq / 1200.0));
					freq /= 42;
					if(freq > 255)
						freq = 255;
					if(freq < 0)
						freq = 0;

					t->lfo1_frequency = freq;
					break;
				}
				case LFO2_Delay:
				{
					int delay = (int16_t)s->set[i];
					delay = 1000 * pow(2.0, (double)delay / 1200.0);

					t->lfo2_delay = 0x8000 - ((delay * 1000) / 725);
					break;
				}
				case LFO2_Freq:
				{
					int freq = (int16_t)s->set[i];
					freq = (8176.0 * pow(2.0, (double)freq / 1200.0));
					freq /= 42;
					if(freq > 255)
						freq = 255;
					if(freq < 0)
						freq = 0;

					t->lfo2_frequency = freq;
					break;
				}
				case Env1_Delay:
				{
					int delay = (int16_t)s->set[i];
					delay = 1000 * pow(2.0, (double)delay / 1200.0);

					t->modenv_delay = 0x8000 - ((delay * 1000) / 725);
					break;
				}
				case Env1_Attack:
				{
					int attack = (int16_t)s->set[i];
					attack = 1000 * pow(2.0, (double)attack / 1200.0);
					if(attack == 0)
						attack = 0x7F;
					else if(attack >= 360)
						attack = (11878 / attack);
					else if(attack < 360)
						attack = 32.0 + 53.15085 * log10(360.0 / attack);
					else
						attack = 0x7F;

					if(attack < 1)
						attack = 1;
					if(attack > 127)
						attack = 127;

					t->modenv_attack = attack;
					break;
				}
				case Env1_Hold:
				{
					int hold = (int16_t)s->set[i];
					hold = 1000 * pow(2.0, (double)hold / 1200.0);
					hold = (0x7F * 92 - hold) / 92;
					if(hold < 1)
						hold = 1;
					if(hold > 127)
						hold = 127;

					t->modenv_hold = hold;
					break;
				}
				case Env1_Decay:
				{
					int decay = (int16_t)s->set[i];
					decay =  1000 * pow(2.0, (double)decay / 1200.0);
					if(decay == 0)
					{
						decay = 127;
					}else{
						decay = 0x7F - 50 * log10(decay / 23.04);
					}
					if(decay < 1)
						decay = 1;
					if(decay > 127)
						decay = 127;

					t->modenv_decay = decay;
					break;
				}
				case Env1_Sustain:
				{
					int sustain = (int16_t)s->set[i];
					if(sustain < 0)
						sustain = 0;
					else if(sustain > 1000)
						sustain = 1000;

					if(sustain == 1000)
					{
						sustain = 1;
					}else{
						sustain = (0x7F * 40 - sustain * 3) / 40;
						if(sustain < 1)
							sustain = 1;
					}

					t->modenv_sustain = sustain;
					break;
				}
				case Env1_Release:
				{
					int release = (int16_t)s->set[i];
					release =  1000 * pow(2.0, (double)release / 1200.0);
					if(release == 0)
					{
						release = 127;
					}else{
						release = 0x7F - 50 * log10(release / 23.04);
					}
					if(release < 1)
						release = 1;
					if(release > 127)
						release = 127;

					t->modenv_release = release;
					break;
				}
				//case Env1_Keynum_To_Hold:
				//case Env1_Keynum_To_Decay:
				case Env2_Delay:
				{
					int delay = (int16_t)s->set[i];
					delay = 1000 * pow(2.0, (double)delay / 1200.0);

					t->volenv_delay = 0x8000 - ((delay * 1000) / 725);
					break;
				}
				case Env2_Attack:
				{
					int attack = (int16_t)s->set[i];
					attack = 1000 * pow(2.0, (double)attack / 1200.0);
					if(attack == 0)
						attack = 0x7F;
					else if(attack >= 360)
						attack = (11878 / attack);
					else if(attack < 360)
						attack = 32.0 + 53.15085 * log10(360.0 / attack);
					else
						attack = 0x7F;

					if(attack < 1)
						attack = 1;
					if(attack > 127)
						attack = 127;

					t->volenv_attack = attack;
					break;
				}
				case Env2_Hold:
				{
					int hold = (int16_t)s->set[i];
					hold = 1000 * pow(2.0, (double)hold / 1200.0);
					hold = (0x7F * 92 - hold) / 92;
					if(hold < 1)
						hold = 1;
					if(hold > 127)
						hold = 127;

					t->volenv_hold = hold;
					break;
				}
				case Env2_Decay:
				{
					int decay = (int16_t)s->set[i];
					decay =  1000 * pow(2.0, (double)decay / 1200.0);
					if(decay == 0)
					{
						decay = 127;
					}else{
						decay = 0x7F - 50 * log10(decay / 23.04);
					}
					if(decay < 1)
						decay = 1;
					if(decay > 127)
						decay = 127;

					t->volenv_decay = decay;
					break;
				}
				case Env2_Sustain:
				{
					int sustain = (int16_t)s->set[i];
					if(sustain <= 0)
					{
						sustain = 0x7F;
						t->volenv_sustain = sustain;
						break;
					}
					else if(sustain >= 1000)
					{
						sustain = 1;
						t->volenv_sustain = sustain;
						break;
					}
					sustain = (0x7F * 40 - sustain * 3) / 40;
					if(sustain < 1)
						sustain = 1;

					t->volenv_sustain = sustain;
					break;
				}
				case Env2_Release:
				{
					int release = (int16_t)s->set[i];
					release =  1000 * pow(2.0, (double)release / 1200.0);
					if(release == 0)
					{
						release = 127;
					}else{
						release = 0x7F - 50 * log10(release / 23.04);
					}
					if(release < 1)
						release = 1;
					if(release > 127)
						release = 127;

					t->volenv_release = release;
					break;
				}
				//case Env2_Keynum_To_Hold:
				//case Env2_Keynum_To_Decay:
//				case Instrument_ID:
//				case Key_Range:
//				case Vel_Range:
//				case Start_Loop_Address_Coarse:
//				case Keynum:
//				case Velocity:
//				case Initial_Attenuation:
//				case Key_Tuning:
//				case End_Loop_Address_Coarse:
//				case Coarse_Tune:
//				case Fine_Tune:
//				case Sample_ID:
//				case Sample_Flags:
//				case Sample_Pitch:
//				case Scale_Tuning:
//				case Exclusive_Class:
//				case Root_Key:
				default:
					break;
			}
		}
	}
}

void play_notes(uint8_t ch, uint8_t note, uint8_t vel)
{
	enumerators_t prst_g;
	enumerators_t prst_l;
	enumerators_t inst_g;
	enumerators_t inst_l;
	_Bool prst_gl = 0;
	_Bool inst_gl = 0;

	reset_enumerators(&prst_g);
	reset_enumerators(&prst_l);
	reset_enumerators(&inst_g);
	reset_enumerators(&inst_l);

	for(int p = sf->presethdr[channel[ch].presetnr].bagNdx; p < sf->presethdr[channel[ch].presetnr + 1].bagNdx; p++)
	{
		for(int pb = sf->presetbag[p]; pb < sf->presetbag[p + 1]; pb++)
		{
			if(sf->presetgen[pb].oper == Instrument_ID)
			{
				if((note <= ((prst_l.set[Key_Range]>>8) & 0xFF)) || (!prst_l.ovr[Key_Range]))
				{
					if((note >= (prst_l.set[Key_Range] & 0xFF)) || (!prst_l.ovr[Key_Range]))
					{
						for(int i = sf->insthdr[sf->presetgen[pb].amount].bagNdx; i < sf->insthdr[sf->presetgen[pb].amount + 1].bagNdx; i++)
						{
							for(int ib = sf->instbag[i]; ib < sf->instbag[i + 1]; ib++)
							{
								if(sf->instgen[ib].oper == Sample_ID)
								{
									if((note <= ((inst_l.set[Key_Range]>>8) & 0xFF)) || (!inst_l.ovr[Key_Range]))
									{
										if((note >= (inst_l.set[Key_Range] & 0xFF)) || (!inst_l.ovr[Key_Range]))
										{
											uint16_t sample = sf->instgen[ib].amount;
											enumerators_t settings;
											emu8000_reg_t reg;
											uint8_t vc = 0;
											uint32_t offset = 0;
											int16_t finetune = 0;
											int16_t coarsetune = 0;
											uint8_t rootkey = 60;
											double pitch = 0xE000;

											reset_enumerators(&settings);
											reset_regs(&reg);
											link_enumerators(&prst_g, &settings);
											link_enumerators(&prst_l, &settings);
											link_enumerators(&inst_g, &settings);
											link_enumerators(&inst_l, &settings);
											parse_enumerators(&settings, &reg);

											if(sf->sampleinfo[sample].sampletype & 0x8000)
												offset = 0;
											else
												offset = EMU8000_DRAM_OFFSET;

											reg.start_address = sf->sampleinfo[sample].startsample + offset;

											if(settings.set[Sample_Flags])
											{
												reg.loop_start = sf->sampleinfo[sample].startloop + offset;
												reg.loop_end = sf->sampleinfo[sample].endloop + offset;
											}else{
												reg.loop_start = sf->sampleinfo[sample].endsample + offset;
												reg.loop_end = sf->sampleinfo[sample].endsample + 4 + offset;
											}

											finetune = (int8_t)sf->sampleinfo[sample].pitchCorrection + (int16_t)settings.set[Fine_Tune];
											coarsetune = (int16_t)settings.set[Coarse_Tune];
											if(settings.ovr[Root_Key])
												rootkey = settings.set[Root_Key];
											else
												rootkey = sf->sampleinfo[sample].originalPitch;

											if(channel[ch].cc[1])
												reg.lfo2_vibrato = channel[ch].cc[1] / 12.7;
											if(channel[ch].cc[10] != 64)
												reg.pan = 0xFF - (channel[ch].cc[10] * 2);
											if(channel[ch].cc[91])
												reg.reverb = (channel[ch].cc[91]) + 1;
											if(channel[ch].cc[93])
												reg.chorus = (channel[ch].cc[93]) + 1;

											if((settings.ovr[Exclusive_Class]) && (settings.set[Exclusive_Class] != 0))
												for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
													if(voice[i].exclusive_class == settings.set[Exclusive_Class])
													{
														emu8000_terminate_voice(i);
														voice[i].state = STATE_END;
														voice[i].ch = voice[i].note = 0xFF;
														voice[i].exclusive_class = 0;
													}

											pitch += (log(sf->sampleinfo[sample].samplerate / 44100.0) / log(2.0)) * 4096.0;
											pitch += finetune * (4096.0 / 1200.0);	//0x1000 - 1 octave
											pitch += coarsetune * (4096.0 / 12.0);
											pitch += (note - rootkey) * (4096.0 / 12.0);
											uint16_t absolutepitch = round(pitch);
											pitch += ((channel[ch].pitch - 0x2000) / 8192.0) * 4096.0;
											pitch = round(pitch);
											reg.initial_pitch = (uint16_t)pitch;

											uint8_t vol;
											if(settings.ovr[Initial_Attenuation])
												vol = (settings.set[Initial_Attenuation] / 10.0) / 0.375;
											else
												vol = 0;

											double vvel = vel / (127.0 / (10.0 - 1.0));
											vvel = log10(1.0 + vvel);
											double vcc = channel[ch].cc[7] / (127.0 / (10.0 - 1.0));
											vcc = log10(1.0 + vcc);
											reg.attenuation = (0xFF - (uint8_t)((((vvel * vcc) / 2.0) + 0.5) * 255.0)) + vol;

											vc = emu8000_find_voice(voice);
											voice[vc].reg = reg;
											voice[vc].ch = ch;
											voice[vc].note = note;
											voice[vc].vel = vel;
											voice[vc].exclusive_class = settings.set[Exclusive_Class];
											if(settings.set[Sample_Flags])
												voice[vc].loop = 1;
											else
												voice[vc].loop = 0;
											voice[vc].state = STATE_PLAYING;
											emu8000_start_voice(vc, voice[vc].reg);
											voice[vc].reg.initial_pitch = absolutepitch;
											voice[vc].vol = vol;
										}
									}
									reset_enumerators(&inst_l);
									inst_gl = 1;
								}else{
									if(sf->instgen[ib].oper == Key_Range)
										inst_gl = 1;

									if(inst_gl == 0)
									{
										inst_g.set[sf->instgen[ib].oper] = sf->instgen[ib].amount;
										inst_g.ovr[sf->instgen[ib].oper] = 1;
									}else{
										inst_l.set[sf->instgen[ib].oper] = sf->instgen[ib].amount;
										inst_l.ovr[sf->instgen[ib].oper] = 1;
									}
								}
							}
							reset_enumerators(&inst_l);
							inst_gl = 1;
						}
					}
				}
				reset_enumerators(&prst_l);
				prst_gl = 1;
			}else{
				if(sf->presetgen[pb].oper == Key_Range)
					prst_gl = 1;

				if(prst_gl == 0)
				{
					prst_g.set[sf->presetgen[pb].oper] = sf->presetgen[pb].amount;
					prst_g.ovr[sf->presetgen[pb].oper] = 1;
				}else{
					prst_l.set[sf->presetgen[pb].oper] = sf->presetgen[pb].amount;
					prst_l.ovr[sf->presetgen[pb].oper] = 1;
				}
			}
		}
		reset_enumerators(&prst_l);
		prst_gl = 1;
	}
}

void reg_soundfont(SFInfo *ptr)
{
	sf = ptr;
}

void bridge_init(void)
{
	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
	{
		reset_regs(&voice[i].reg);
		emu8000_release_voice(i, voice[i].reg);
		voice[i].state = STATE_END;
		voice[i].ch = voice[i].note = 0xFF;
		voice[i].exclusive_class = 0;
		voice[i].loop = 0;
	}
	for(int i = 0; i < 16; i++)
	{
		all_notes_off(i);
		reset_all_controllers(i);
	}
}

void all_notes_off(uint8_t ch)
{
	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
	{
		if(voice[i].ch == ch)
		{
			emu8000_release_voice(i, voice[i].reg);
			voice[i].state = STATE_END;
			voice[i].ch = voice[i].note = 0xFF;
			voice[i].exclusive_class = 0;
		}
	}
	reset_all_controllers(ch);
}

void reset_all_controllers(uint8_t ch)
{
	for(int cc = 0; cc < 128; cc++)
	{
		channel[ch].cc[cc] = 0;
	}
	channel[ch].cc[7] = 127;
	channel[ch].cc[10] = 64;
	channel[ch].pitch = 0x2000;

	program_change(ch, 0);
}

void note_off(uint8_t ch, uint8_t note, uint8_t vel)
{
	vel = vel;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
	{
		if(voice[i].ch == ch)
		{
			if(voice[i].note == note)
			{
				emu8000_release_voice(i, voice[i].reg);
				voice[i].state = STATE_RELEASED;
				voice[i].ch = voice[i].note = 0xFF;
			}
		}
	}
}

void note_on(uint8_t ch, uint8_t note, uint8_t vel)
{
	if(vel)
	{
		play_notes(ch, note, vel);
	}else{
		note_off(ch, note, vel);
		return;
	}
}

void aftertouch_poly(uint8_t ch, uint8_t note, uint8_t pressure)
{
	ch = ch;
	note = note;
	pressure = pressure;
}

void control_change(uint8_t ch, uint8_t cc, uint8_t val)
{
	channel[ch].cc[cc] = val;
	switch(cc)
	{
		case 0:	//Bank select MSB
			program_change(ch, channel[ch].program);
			break;
		case 1: //Modulation wheel
			for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
			{
				if(voice[i].ch == ch)
				{
					voice[i].reg.lfo2_vibrato = channel[ch].cc[1] / 12.7;
					emu8000_update_voice(i, voice[i].reg, SET_FM2FRQ2);
				}
			}
			break;
		case 7: //Channel volume
			for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
			{
				if(voice[i].ch == ch)
				{
					double vvel = voice[i].vel / (127.0 / (10.0 - 1.0));
					vvel = log10(1.0 + vvel);
					double vcc = channel[ch].cc[7] / (127.0 / (10.0 - 1.0));
					vcc = log10(1.0 + vcc);
					voice[i].reg.attenuation = (0xFF - ((((vvel * vcc) / 2.0) + 0.5) * 255.0)) + voice[i].vol;
					emu8000_update_voice(i, voice[i].reg, SET_VOLUME);
				}
			}
			break;
		case 10: //Channel pan
			for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
			{
				if(voice[i].ch == ch)
				{
					voice[i].reg.pan = 0xFF - (channel[ch].cc[10] * 2);
					emu8000_update_voice(i, voice[i].reg, SET_PAN);
				}
			}
			break;
		case 121: //Reset all controllers
			reset_all_controllers(ch);
			break;
		case 123: //All notes off
			all_notes_off(ch);
			break;
	}
}

void program_change(uint8_t ch, uint8_t program)
{
	uint8_t bank = 0;
	uint8_t preset = 0;

	channel[ch].program = program;

	if(ch != 9)
	{
		bank = channel[ch].cc[0];
	}else{
		bank = 128;
	}

	for(int prst = 0; prst < sf->nrpresets - 1; prst++)
	{
		if(sf->presethdr[prst].bank == bank)
		{
			if(sf->presethdr[prst].preset == channel[ch].program)
			{
				channel[ch].presetnr = prst;
				return;
			}
		}
	}

	if(ch != 9)
	{
		bank = 0;
		preset = channel[ch].program;
	}else{
		bank = 128;
		preset = 0;
	}

	for(int prst = 0; prst < sf->nrpresets - 1; prst++)
	{
		if(sf->presethdr[prst].bank == bank)
		{
			if(sf->presethdr[prst].preset == preset)
			{
				channel[ch].presetnr = prst;
				return;
			}
		}
	}

	channel[ch].presetnr = 0;
}

void aftertouch_chan(uint8_t ch, uint8_t pressure)
{
	ch = ch;
	pressure = pressure;
}

void pitch_bend(uint8_t ch, uint8_t lsb, uint8_t msb)
{
	channel[ch].pitch = ((msb & 0x7F)<<7) | (lsb & 0x7F);
	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
	{
		if(voice[i].ch == ch)
		{
			uint16_t prevpitch = voice[i].reg.initial_pitch;
			double pitch = prevpitch;
			pitch += ((channel[ch].pitch - 0x2000) / 8192.0) * 4096.0;
			voice[i].reg.initial_pitch = round(pitch);
			emu8000_update_voice(i, voice[i].reg, SET_PITCH);
			voice[i].reg.initial_pitch = prevpitch;
		}
	}
}

void sysex(uint8_t *buf, int len, uint8_t divided)
{
	buf = buf;
	len = len;
	divided = divided;
}

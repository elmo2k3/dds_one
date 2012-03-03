/*
 * Copyright (C) 2012 Bjoern Biesenbach <bjoern at bjoern-b.de>
 *               2012 homerj00			<homerj00@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include "page_single_tone.h"
#include "lcd-routines.h"
#include "buttons.h"
#include "DDS.h"

static enum _selectionState
{
	state_select_nothing,
	state_select_gain,
	state_select_3_mhz,
	state_select_2_mhz,
	state_select_1_mhz,
	state_select_6_khz,
	state_select_5_khz,
	state_select_4_khz,
	state_select_3_khz,
	state_select_2_khz,
	state_select_1_khz,
	state_select_last
}selectionState;

static uint32_t frequency_eeprom EEMEM;
static uint8_t gain_eeprom EEMEM;

static uint32_t frequency; // frequency in single tone mode
static uint8_t gain; // gain


/* Menu
 * 		1234567890123456
 * 	1	  Single  Tone
 * 	2	f=100.0MHz  v=15
 *
 * 		1234567890123456
 * 	1	Single Tone v=15
 * 	2	f=100.000000MHz
*/

static void print_gain(uint8_t blink)
{
	char digits[3];
	uint8_t g;
	
	g = gain;
	digits[0] = g/1e1; g -= digits[0]*1e1;
	digits[1] = g;

	digits[0] += 48;
	digits[1] += 48;
	digits[2] = 0;

	lcd_setcursor(12,1);
	lcd_string("v=");
	if(blink && selectionState == state_select_gain)
		lcd_string("  ");
	else
		lcd_string(digits);
}

static void print_frequency(uint8_t blink)
{
	char digits[11];
	uint8_t i;
	uint32_t f;

	lcd_setcursor(0,2); // second line

	f = frequency;
	digits[0] = f/1e8; f -= digits[0]*1e8;
	digits[1] = f/1e7; f -= digits[1]*1e7;
	digits[2] = f/1e6; f -= digits[2]*1e6;
	digits[3] = '.';
	digits[4] = f/1e5; f -= digits[4]*1e5;
	digits[5] = f/1e4; f -= digits[5]*1e4;
	digits[6] = f/1e3; f -= digits[6]*1e3;
	digits[7] = f/1e2; f -= digits[7]*1e2;
	digits[8] = f/1e1; f -= digits[8]*1e1;
	digits[9] = f/1e0; f -= digits[9]*1e0;
	digits[10] = 0;

	for(i=0;i<10;i++)
		digits[i] += 48;
	digits[3] = '.';
	if(blink)
	{
		if(selectionState == state_select_3_mhz)
			digits[0] = ' ';
		if(selectionState == state_select_2_mhz)
			digits[1] = ' ';
		if(selectionState == state_select_1_mhz)
			digits[2] = ' ';
		if(selectionState == state_select_6_khz)
			digits[4] = ' ';
		if(selectionState == state_select_5_khz)
			digits[5] = ' ';
		if(selectionState == state_select_4_khz)
			digits[6] = ' ';
		if(selectionState == state_select_3_khz)
			digits[7] = ' ';
		if(selectionState == state_select_2_khz)
			digits[8] = ' ';
		if(selectionState == state_select_1_khz)
			digits[9] = ' ';
	}

	lcd_string(digits);
	lcd_string(" MHz");
}

void page_single_tone_draw(struct menuitem *self)
{
	selectionState = state_select_nothing;
	lcd_clear();
	lcd_string(self->line1);

	dds_set_single_tone_frequency(frequency);
	dds_vga_set_gain(gain);

	print_gain(0);
	print_frequency(0);
}

uint8_t page_single_tone_bt(struct menuitem *self, uint8_t button, uint8_t rpt)
{
	static uint8_t num_rpt = 0;
	uint32_t increment;

	increment = 1000; // 1MHz

	if(button == BT_UP)
	{
		if(selectionState == state_select_gain)
			gain++;
		else if(selectionState == state_select_3_mhz)
			frequency += 1e8;
		else if(selectionState == state_select_2_mhz)
			frequency += 1e7;
		else if(selectionState == state_select_1_mhz)
			frequency += 1e6;
		else if(selectionState == state_select_6_khz)
			frequency += 1e5;
		else if(selectionState == state_select_5_khz)
			frequency += 1e4;
		else if(selectionState == state_select_4_khz)
			frequency += 1e3;
		else if(selectionState == state_select_3_khz)
			frequency += 1e2;
		else if(selectionState == state_select_2_khz)
			frequency += 1e1;
		else if(selectionState == state_select_1_khz)
			frequency += 1e0;
	}
	else if(button == BT_DOWN)
	{
		if(selectionState == state_select_gain)
			gain--;
		else if(selectionState == state_select_3_mhz)
			frequency -= 1e8;
		else if(selectionState == state_select_2_mhz)
			frequency -= 1e7;
		else if(selectionState == state_select_1_mhz)
			frequency -= 1e6;
		else if(selectionState == state_select_6_khz)
			frequency -= 1e5;
		else if(selectionState == state_select_5_khz)
			frequency -= 1e4;
		else if(selectionState == state_select_4_khz)
			frequency -= 1e3;
		else if(selectionState == state_select_3_khz)
			frequency -= 1e2;
		else if(selectionState == state_select_2_khz)
			frequency -= 1e1;
		else if(selectionState == state_select_1_khz)
			frequency -= 1e0;
	}
	else if(button == BT_CM)
	{
		if(++selectionState == state_select_last)
		{
			selectionState = 0;
			print_frequency(0);
			return 1; // focus not here any more
		}
		else
		{
			print_gain(1);
			print_frequency(1);
			return 0; // focus still here
		}
	}

	if(frequency > 200000000)
		frequency = 0;
	if(gain == 16)
		gain = 0x0;
	if(gain == 0xFF)
		gain = 0x0F;

	dds_set_single_tone_frequency(frequency);
	dds_vga_set_gain(gain);
	print_frequency(0);
	print_gain(0);

	return 0; // still has focus
}

void page_single_tone_periodic(struct menuitem *self)
{
	static uint8_t toggler;

	toggler ^= 1;
	print_gain(toggler);
	print_frequency(toggler);
}

void page_single_tone_load_parameters(struct menuitem *self)
{
	frequency = eeprom_read_dword(&frequency_eeprom);
	if(frequency == 0xFFFFFFFF) // cell after erase cycle
		frequency = 0;
	gain = eeprom_read_byte(&gain_eeprom);
	if(gain == 0xFF)
		gain = 0;
}

void page_single_tone_write_parameters(struct menuitem *self)
{
	eeprom_write_dword(&frequency_eeprom, frequency);
	eeprom_write_byte(&gain_eeprom, gain);
}


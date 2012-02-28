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

#include "page_single_tone.h"
#include "lcd-routines.h"
#include "buttons.h"
#include "DDS.h"

/* Menu
 * 		1234567890123456
 * 	1	  Single  Tone
 * 	2	f=100.0MHz  v=15
*/
void page_single_tone_draw(struct menuitem *self, uint8_t blink_f, uint8_t blink_g)
{
	char numstring[10];
	char s[20];
	uint8_t i;
	uint8_t num_length = 0;
	uint32_t mhz;
	uint32_t khz;
	
	mhz = frequency/1000000;
	khz = (frequency - mhz*1000000)/1000;

	lcd_clear();
	lcd_string(self->line1);
	lcd_setcursor(0,2); // next line

	if(blink_f)
		strcpy(s," =");
	else
		strcpy(s,"f=");
	if(mhz == 0) // draw khz only
	{
		ultoa(frequency/1000,numstring,10);
		num_length = strlen(numstring);
		strcat(s,numstring);
		strcat(s,"kHz");
	}
	else // MHz
	{

		ultoa(mhz,numstring,10);
		num_length = strlen(numstring);
		strcat(s,numstring);
		strcat(s,".");
		num_length++;
		if(khz < 100) // add leading zeroes
		{
			strcat(s,"0");
			num_length++;
		}
		if(khz < 10)
		{
			strcat(s,"0");
			num_length++;
		}
		ultoa(khz,numstring,10);
		num_length += strlen(numstring);
		strcat(s,numstring);
		if(mhz > 100)
			strcat(s,"M  ");
		else
			strcat(s,"MHz");
	}
	// fill with whitespaces
	for(i=0;i<7-num_length;i++)
	{
		strcat(s," ");
	}
	if(blink_g)
		strcat(s," =");
	else
		strcat(s,"v=");
	ultoa(gain,numstring,10);
	strcat(s,numstring);
	lcd_string(s);
}

void page_single_tone_bt_f(struct menuitem *self, uint8_t button, uint8_t rpt)
{
	static uint8_t num_rpt = 0;
	uint32_t increment;
	
	if(rpt)
		num_rpt++;
	else
		num_rpt = 0;

	if(num_rpt > 40)
	{
		num_rpt = 40;
		increment = 10000000; // 10MHz
	}
	else if(num_rpt > 30)
	{
		increment = 1000000; // 1MHz
	}
	else if(num_rpt > 20)
	{
		increment = 100000; // 100kHz
	}
	else if(num_rpt > 10)
		increment = 10000; // 10kHz
	else
		increment = 1000; // 1kHz

	// clear everything below increment step
	// example: f=101.111MHz
	// increment step 1MHz
	// next value: f= 102.000MHz
	frequency = frequency/increment;
	frequency = frequency*increment;

	if(button == BT_UP)
	{
		frequency += increment;
	}
	else if(button == BT_DOWN)
		frequency -= increment;

	if(frequency > 200000000)
		frequency = 0;

	dds_set_single_tone_frequency(frequency);
	self->draw_func(self,0,0);
}
void page_single_tone_bt_g(struct menuitem *self, uint8_t button, uint8_t rpt)
{
	if(button == BT_UP)
	{
		if(++gain > 0x0F)
			gain = 0;
	}
	else if(button == BT_DOWN)
	{
		if(--gain > 0x0F) // because of unsigned
			gain = 0x0F;
	}
	dds_vga_set_gain(gain);
	self->draw_func(self,0,0);
}

void page_single_tone_blink_f(struct menuitem *self)
{
	static uint8_t blink_f;
	blink_f ^= 1;
	self->draw_func(self,blink_f,0);
}
void page_single_tone_blink_g(struct menuitem *self)
{
	static uint8_t blink_g;
	blink_g ^= 1;
	self->draw_func(self,0,blink_g);
}

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

#include "page_linear_sweep.h"
#include "lcd-routines.h"
#include "buttons.h"
#include "DDS.h"

static uint32_t f1_eeprom EEMEM;
static uint32_t f2_eeprom EEMEM;
static uint32_t time_eeprom EEMEM;

static uint32_t f1;
static uint32_t f2;
static uint32_t time; // in ms

static enum _selectionState
{
	state_select_nothing,
	state_select_f1_3_mhz,
	state_select_f1_2_mhz,
	state_select_f1_1_mhz,
	state_select_f1_3_khz,
	state_select_f1_2_khz,
	state_select_f2_3_mhz,
	state_select_f2_2_mhz,
	state_select_f2_1_mhz,
	state_select_f2_3_khz,
	state_select_f2_2_khz,
	state_select_time_4,
	state_select_time_3,
	state_select_time_2,
	state_select_time_1,
	state_select_last
}selectionState;

// 400MHz SYSCLK -> 100MHz SYNC_CLK
// t = ((f2-f1)/fstep)*(Ramp_rate/SYNC_CLK)
void set_sweep()
{
	uint32_t rate;
	uint32_t f_step;
	uint32_t t;

	t = time;
	rate = (t*100e3)/(f2-f1)+ 1;
	if(rate > 255)
		rate = 255;

	f_step = ((f2-f1)*rate)/(t*100e3) + 1;
	if(f_step == 0)
		f_step = 1;

    dds_set_linear_sweep(f1,f2,f_step,f_step,rate);
}

static void print_f1(uint8_t blink)
{
	char digits[7];
	uint8_t i;
	uint32_t f;

	lcd_setcursor(0,1);
	lcd_string("LSweep f1=");

	f = f1;
	digits[0] = f/1e8; f -= digits[0]*1e8;
	digits[1] = f/1e7; f -= digits[1]*1e7;
	digits[2] = f/1e6; f -= digits[2]*1e6;
	digits[3] = '.';
	digits[4] = f/1e5; f -= digits[4]*1e5;
	digits[5] = f/1e4; f -= digits[5]*1e4;
	digits[6] = 0;

	for(i=0;i<6;i++)
		digits[i] += 48;
	digits[3] = '.';
	if(blink)
	{
		if(selectionState == state_select_f1_3_mhz)
			digits[0] = ' ';
		if(selectionState == state_select_f1_2_mhz)
			digits[1] = ' ';
		if(selectionState == state_select_f1_1_mhz)
			digits[2] = ' ';
		if(selectionState == state_select_f1_3_khz)
			digits[4] = ' ';
		if(selectionState == state_select_f1_2_khz)
			digits[5] = ' ';
	}
	lcd_string(digits);
}

static void print_f2(uint8_t blink)
{
	char digits[7];
	uint8_t i;
	uint32_t f;

	lcd_setcursor(7,2);
	lcd_string("f2=");

	f = f2;
	digits[0] = f/1e8; f -= digits[0]*1e8;
	digits[1] = f/1e7; f -= digits[1]*1e7;
	digits[2] = f/1e6; f -= digits[2]*1e6;
	digits[3] = '.';
	digits[4] = f/1e5; f -= digits[4]*1e5;
	digits[5] = f/1e4; f -= digits[5]*1e4;
	digits[6] = 0;

	for(i=0;i<6;i++)
		digits[i] += 48;
	digits[3] = '.';
	if(blink)
	{
		if(selectionState == state_select_f2_3_mhz)
			digits[0] = ' ';
		if(selectionState == state_select_f2_2_mhz)
			digits[1] = ' ';
		if(selectionState == state_select_f2_1_mhz)
			digits[2] = ' ';
		if(selectionState == state_select_f2_3_khz)
			digits[4] = ' ';
		if(selectionState == state_select_f2_2_khz)
			digits[5] = ' ';
	}
	lcd_string(digits);
}

static void print_time(uint8_t blink)
{
	char digits[6];
	uint8_t i;
	uint32_t f;

	lcd_setcursor(0,2);
	lcd_string("t=");

	f = time;

	digits[0] = f/1e3; f -= digits[0]*1e3;
	digits[1] = f/1e2; f -= digits[1]*1e2;
	digits[2] = f/1e1; f -= digits[2]*1e1;
	digits[3] = f/1e0; f -= digits[3]*1e0;
	digits[4] = 0;

	for(i=0;i<4;i++)
		digits[i] += 48;

	if(blink)
	{
		if(selectionState == state_select_time_4)
			digits[0] = ' ';
		if(selectionState == state_select_time_3)
			digits[1] = ' ';
		if(selectionState == state_select_time_2)
			digits[2] = ' ';
		if(selectionState == state_select_time_1)
			digits[3] = ' ';
	}
	lcd_string(digits);
}

/* Menu
 * 		1234567890123456
 * 	1	LSweep f1=123.12
 * 	2	t=123u f2=100.00
*/
void page_linear_sweep_draw(struct menuitem *self, uint8_t blink_f, uint8_t blink_g)
{
	lcd_clear();
	lcd_string(self->line1);
	set_sweep();
}

uint8_t page_linear_sweep_bt(struct menuitem *self, uint8_t button, uint8_t rpt)
{
	static uint8_t num_rpt = 0;
	uint32_t increment;

	if(button == BT_UP)
	{
		if(selectionState == state_select_f1_3_mhz)
			f1 += 1e8;
		else if(selectionState == state_select_f1_2_mhz)
			f1 += 1e7;
		else if(selectionState == state_select_f1_1_mhz)
			f1 += 1e6;
		else if(selectionState == state_select_f1_3_khz)
			f1 += 1e5;
		else if(selectionState == state_select_f1_2_khz)
			f1 += 1e4;
		else if(selectionState == state_select_f2_3_mhz)
			f2 += 1e8;
		else if(selectionState == state_select_f2_2_mhz)
			f2 += 1e7;
		else if(selectionState == state_select_f2_1_mhz)
			f2 += 1e6;
		else if(selectionState == state_select_f2_3_khz)
			f2 += 1e5;
		else if(selectionState == state_select_f2_2_khz)
			f2 += 1e4;
		else if(selectionState == state_select_time_4)
			time += 1e3;
		else if(selectionState == state_select_time_3)
			time += 1e2;
		else if(selectionState == state_select_time_2)
			time += 1e1;
		else if(selectionState == state_select_time_1)
			time += 1e0;
			
	}
	else if(button == BT_DOWN)
	{
		if(selectionState == state_select_f1_3_mhz)
			f1 -= 1e8;
		else if(selectionState == state_select_f1_2_mhz)
			f1 -= 1e7;
		else if(selectionState == state_select_f1_1_mhz)
			f1 -= 1e6;
		else if(selectionState == state_select_f1_3_khz)
			f1 -= 1e5;
		else if(selectionState == state_select_f1_2_khz)
			f1 -= 1e4;
		else if(selectionState == state_select_f2_3_mhz)
			f2 -= 1e8;
		else if(selectionState == state_select_f2_2_mhz)
			f2 -= 1e7;
		else if(selectionState == state_select_f2_1_mhz)
			f2 -= 1e6;
		else if(selectionState == state_select_f2_3_khz)
			f2 -= 1e5;
		else if(selectionState == state_select_f2_2_khz)
			f2 -= 1e4;
		else if(selectionState == state_select_time_4)
			time -= 1e3;
		else if(selectionState == state_select_time_3)
			time -= 1e2;
		else if(selectionState == state_select_time_2)
			time -= 1e1;
		else if(selectionState == state_select_time_1)
			time -= 1e0;
	}
	else if(button == BT_CM)
	{
		if(++selectionState == state_select_last)
		{
			selectionState = 0;
			print_f1(0);
			print_time(0);
			print_f2(0);
			return 1; // focus not here any more
		}
		else
		{
			print_f1(1);
			print_time(1);
			print_f2(1);
			return 0; // focus still here
		}
	}

	set_sweep();
	print_f1(0);
	print_time(0);
	print_f2(0);

	return 0; // still has focus
}

void page_linear_sweep_periodic(struct menuitem *self)
{
	static uint16_t prescaler = 1;
	static uint8_t prescaler2 = 10;
	static uint8_t toggler;

	if(--prescaler == 0)
	{
		prescaler = time/50;
	    PORTA ^= (1<<PA0);
	}

	if(--prescaler2 == 0)
	{
		prescaler2 = 10;
		toggler ^= 1;
		print_f1(toggler);
		print_time(toggler);
		print_f2(toggler);
	}
}
void page_linear_sweep_load_parameters(struct menuitem *self)
{
	f1 = eeprom_read_dword(&f1_eeprom);
	if(f1 == 0xFFFFFFFF) // cell after erase cycle
		f1 = 0;
	f2 = eeprom_read_dword(&f2_eeprom);
	if(f2 == 0xFFFFFFFF) // cell after erase cycle
		f2 = 0;
	time = eeprom_read_dword(&time_eeprom);
	if(time == 0xFFFFFFFF) // cell after erase cycle
		time = 50;
}
void page_linear_sweep_write_parameters(struct menuitem *self)
{
	eeprom_write_dword(&f1_eeprom, f1);
	eeprom_write_dword(&f2_eeprom, f2);
	eeprom_write_dword(&time_eeprom, time);
}


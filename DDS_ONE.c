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

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "DDS_ONE.h"
#include "uart.h"
#include "adc.h"
#include "DDS.h"
#include "buttons.h"
#include "lcd-routines.h"
#include "page_single_tone.h"
#include "page_linear_sweep.h"
#include "version.h"

#define FLAG_500MS 1
#define FLAG_1S 2

#define SECONDS_TO_EEPROM_SAVE 10

volatile uint8_t refreshFlags;

void init(void);
void wait(uint8_t k);
void timer_init(void);

void load_parameters_from_eeprom(void);
void save_parameters_to_eeprom(void);

// Line 1 , Line 2 , draw func , button func , periodic 500ms func, eeprom load, eeprom save
struct menuitem menu [] = {
	{"Single Tone ", "", page_single_tone_draw, page_single_tone_bt,
						 page_single_tone_periodic, page_single_tone_load_parameters,
						 page_single_tone_write_parameters},
	{"  Linear Sweep  ", "", page_linear_sweep_draw, page_linear_sweep_bt,
						 page_linear_sweep_periodic, page_linear_sweep_load_parameters,
						 page_linear_sweep_write_parameters}
};
static const unsigned NUM_PAGES = sizeof(menu) / sizeof(menu[0]);

void save_parameters_to_eeprom()
{
	uint8_t i;

	for(i=0;i<NUM_PAGES;i++)
	{
		if(menu[i].parameter_save_func)
			menu[i].parameter_save_func();
	}
	lcd_toggle_backlight();
	wait(1);
	lcd_toggle_backlight();
}

void load_parameters_from_eeprom()
{
	uint8_t i;

	for(i=0;i<NUM_PAGES;i++)
	{
		if(menu[i].parameter_load_func)
			menu[i].parameter_load_func();
	}
}

int main(void)
{
	uint8_t menu_position;
	uint8_t seconds_since_last_button_press;
	uint32_t seconds_since_startup;
	uint8_t focus_here;

	init();
	
	seconds_since_last_button_press = SECONDS_TO_EEPROM_SAVE+1;
	seconds_since_startup = 0;
	menu_position = 0;
	focus_here = 1;

	load_parameters_from_eeprom();
	menu[menu_position].draw_func(&menu[menu_position]);

    while(1)
    {
		if(buttons_get_rpt(1<<BT_UP))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				menu[menu_position].button_func(&menu[menu_position],BT_UP,1);
		}
		if(buttons_get_press(1<<BT_UP))
		{
			if(focus_here)
			{
				menu_position++;
				if(menu_position == NUM_PAGES)
					menu_position = 0;
				menu[menu_position].draw_func(&menu[menu_position]);
			}
			else
			{
				seconds_since_last_button_press = 0;
				if(menu[menu_position].button_func)
					menu[menu_position].button_func(&menu[menu_position],BT_UP,0);
			}
		}
		if(buttons_get_rpt(1<<BT_DOWN))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				menu[menu_position].button_func(&menu[menu_position],BT_DOWN,1);
		}
		if(buttons_get_press(1<<BT_DOWN))
		{
			if(focus_here)
			{
				menu_position--;
				if(menu_position == NUM_PAGES)
					menu_position = 0;
				if(menu_position > NUM_PAGES)
					menu_position = NUM_PAGES-1;
				menu[menu_position].draw_func(&menu[menu_position]);
			}
			else
			{
				seconds_since_last_button_press = 0;
				if(menu[menu_position].button_func)
					menu[menu_position].button_func(&menu[menu_position],BT_DOWN,0);
			}
		}
		if(buttons_get_rpt(1<<BT_CM))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				focus_here = menu[menu_position].button_func(&menu[menu_position],BT_CM,1);
		}
		if(buttons_get_press(1<<BT_CM))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				focus_here = menu[menu_position].button_func(&menu[menu_position],BT_CM,0);
		}
		if(refreshFlags & (1<<FLAG_500MS)){
			refreshFlags &= ~(1<<FLAG_500MS);
			if(menu[menu_position].periodic_500ms_func)
				menu[menu_position].periodic_500ms_func(&menu[menu_position]);
    	}
		if(refreshFlags & (1<<FLAG_1S))
		{
			refreshFlags &= ~(1<<FLAG_1S);
			seconds_since_startup++;
			if(seconds_since_last_button_press == SECONDS_TO_EEPROM_SAVE)
			{
				// one last increment so we wont get here twice
				seconds_since_last_button_press++;
				save_parameters_to_eeprom();
			}
			else if(seconds_since_last_button_press < SECONDS_TO_EEPROM_SAVE)
				seconds_since_last_button_press++;
		}
	}
}

void init(void)
{
	uint32_t i;

	lcd_init();
		
	//DDS Initialisierung
	dds_init();
	
	//ADC Initialisieren
	ADC_Init();

	// Taster initialisieren
	buttons_init();

	timer_init();
	
	//lcd_toggle_backlight();
	lcd_string("    DDS ONE");
	lcd_setcursor( 0, 2 );
	lcd_string(GIT_VERSION);
	wait(10);
}

void timer_init()
{
	// timer 0
	TCCR0 = (1<<WGM01)|(1<<CS02);     // CTC, XTAL / 64
    OCR0 = (uint8_t)(F_CPU / 64.0 * 1e-3 - 0.5);   // 1ms
    TIMSK |= 1<<OCIE0;
}

void wait(uint8_t k)
{
	//Wait for k*100ms
	for (uint8_t i=0; i<k; i++)
	{
		_delay_ms(100);
	}
}

ISR(SIG_OUTPUT_COMPARE0) // 1ms
{
	cli();
	static uint16_t prescaler = 1000;
	if(--prescaler == 0){
		refreshFlags |= (1<<FLAG_1S);
		prescaler = 1000;
	}else if(!(prescaler % 10)){ // 10ms
		buttons_every_10_ms();
	}
	if(!(prescaler % 500)){ // 500ms
		refreshFlags |= (1<<FLAG_500MS);
	}
	sei();
}

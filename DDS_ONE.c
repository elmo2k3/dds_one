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
#include <avr/eeprom.h>
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
#include "version.h"

#define FLAG_500MS 1
#define FLAG_1S 2

#define SECONDS_TO_EEPROM_SAVE 10

uint32_t frequency_eeprom EEMEM;
uint8_t gain_eeprom EEMEM;

// frequency and gain are global to all files!
uint32_t frequency; // frequency in single tone mode
uint8_t gain; // gain

volatile uint8_t refreshFlags;

void init(void);
void wait(uint8_t k);
void timer_init(void);

void save_parameters_to_eeprom(void);

// Line 1 , Line 2 , draw func , button func , periodic 500ms func
struct menuitem menu [] = {
	{"  Single  Tone  ", "", page_single_tone_draw, NULL, NULL},
	{"  Single  Tone  ", "", page_single_tone_draw, page_single_tone_bt_f, page_single_tone_blink_f},
	{"  Single  Tone  ", "", page_single_tone_draw, page_single_tone_bt_g, page_single_tone_blink_g}
};
static const unsigned NUM_PAGES = sizeof(menu) / sizeof(menu[0]);

int main(void)
{
	uint8_t menu_position;
	uint8_t seconds_since_last_button_press;
	uint32_t seconds_since_startup;

	init();
	
	dds_set_frequency(frequency);
	dds_vga_set_gain(gain);

	seconds_since_last_button_press = SECONDS_TO_EEPROM_SAVE+1;
	seconds_since_startup = 0;
	menu_position = 0;
	menu[menu_position].draw_func(&menu[menu_position],0,0);

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
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				menu[menu_position].button_func(&menu[menu_position],BT_UP,0);
		}
		if(buttons_get_rpt(1<<BT_DOWN))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				menu[menu_position].button_func(&menu[menu_position],BT_DOWN,1);
		}
		if(buttons_get_press(1<<BT_DOWN))
		{
			seconds_since_last_button_press = 0;
			if(menu[menu_position].button_func)
				menu[menu_position].button_func(&menu[menu_position],BT_DOWN,0);
		}
		if(buttons_get_press(1<<BT_CM))
		{
			menu_position++;
			if(menu_position == NUM_PAGES)
				menu_position = 0;
			menu[menu_position].draw_func(&menu[menu_position],0,0);
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

void save_parameters_to_eeprom()
{
	eeprom_write_dword(&frequency_eeprom, frequency);
	eeprom_write_byte(&gain_eeprom, gain);
	lcd_toggle_backlight();
	wait(1);
	lcd_toggle_backlight();
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
	
	frequency = eeprom_read_dword(&frequency_eeprom);
	if(frequency == 0xFFFFFFFF) // cell after erase cycle
		frequency = 0;
	gain = eeprom_read_byte(&gain_eeprom);
	if(gain == 0xFF)
		gain = 0;
}

void timer_init()
{
	// timer 0
	TCCR0 = 1<<WGM01^1<<CS01^1<<CS00;     // CTC, XTAL / 64
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

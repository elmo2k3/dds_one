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

#include "uart.h"
#include "adc.h"
#include "DDS.h"
#include "buttons.h"
#include "lcd-routines.h"
#include "version.h"

#define FLAG_500MS 1
#define FLAG_1S 2

#define SECONDS_TO_EEPROM_SAVE 10

uint32_t frequency_eeprom EEMEM;
uint8_t gain_eeprom EEMEM;

uint32_t frequency; // frequency in single tone mode
uint8_t gain; // gain
volatile uint8_t refreshFlags;

struct menuitem
{
    char line1[30];
	char line2[30];
    void (*draw_func)(struct menuitem *self, uint8_t opt1, uint8_t opt2);
    void (*button_func)(struct menuitem *self, uint8_t button, uint8_t rpt);
    void (*periodic_500ms_func)(struct menuitem *self);
};

void init(void);
void wait(uint8_t k);
void timer_init(void);

void save_parameters_to_eeprom(void);

void page_single_tone_draw(struct menuitem *self, uint8_t blink_f, uint8_t blink_g);
void page_single_tone_bt_f(struct menuitem *self, uint8_t button, uint8_t rpt);
void page_single_tone_bt_g(struct menuitem *self, uint8_t button, uint8_t rpt);
void page_single_tone_blink_f(struct menuitem *self);
void page_single_tone_blink_g(struct menuitem *self);
void page_single_tone_periodic(struct menuitem *self);

//#define NUM_PAGES 3
struct menuitem menu [] = {
	{"  Single  Tone  ", "", page_single_tone_draw, NULL, NULL},
	{"  Single  Tone  ", "", page_single_tone_draw, page_single_tone_bt_f, page_single_tone_blink_f},
	{"  Single  Tone  ", "", page_single_tone_draw, page_single_tone_bt_g, page_single_tone_blink_g}
};
static const unsigned NUM_PAGES = sizeof(menu) / sizeof(menu[0]);

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
	else //if(mhz < 100) // 1-100MHz
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
/*	else // >100MHz
	{

		ultoa(mhz,numstring,10);
		num_length = strlen(numstring);
		strcat(s,numstring);
		strcat(s,".");
		num_length++;
		khz = khz/10; // dont draw single khz any more now
		if(khz < 10) // add leading zeroes
		{
			strcat(s,"0");
			num_length++;
		}
		ultoa(khz,numstring,10);
		num_length += strlen(numstring);
		strcat(s,numstring);
		strcat(s,"MHz");
	}*/
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

	dds_set_frequency(frequency);
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

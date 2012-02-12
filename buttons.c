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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "buttons.h"

//code taken from http://www.mikrocontroller.net/articles/Entprellung
//
static volatile uint8_t key_state;
static volatile uint8_t key_press;
static volatile uint8_t key_rpt;

void buttons_init()
{
    // inputs
    BT_DDR &= ~((1<<BT_UP) | (1<<BT_DOWN) | (1<<BT_CM));
    // pullups on
    BT_PORT |= (1<<BT_UP) | (1<<BT_DOWN) | (1<<BT_CM);
}

void buttons_every_10_ms(){
    static uint8_t ct0=0, ct1=0, rpt=0;
    uint8_t i;

    i = key_state ^ ~(BT_PIN & ((1<<BT_UP) | (1<<BT_DOWN) | (1<<BT_CM)));
    ct0 = ~(ct0 & i);
    ct1 = ct0 ^(ct1 & i);
    i &= ct0 & ct1;
    key_state ^= i;
    key_press |= key_state & i;

    if((key_state & REPEAT_MASK) == 0){
        rpt = REPEAT_START;
    }
    if(--rpt == 0){
        rpt = REPEAT_NEXT;
        key_rpt |= key_state & REPEAT_MASK;
    }
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t buttons_get_press( uint8_t key_mask )
{
  cli();                                          // read and clear atomic !
  key_mask &= key_press;                          // read key(s)
  key_press ^= key_mask;                          // clear key(s)
  sei();
  return key_mask;
}
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed long enough such that the
// key repeat functionality kicks in. After a small setup delay
// the key is reported beeing pressed in subsequent calls
// to this function. This simulates the user repeatedly
// pressing and releasing the key.
//
uint8_t buttons_get_rpt( uint8_t key_mask )
{
  cli();                                          // read and clear atomic !
  key_mask &= key_rpt;                            // read key(s)
  key_rpt ^= key_mask;                            // clear key(s)
  sei();
  return key_mask;
}

uint8_t buttons_get_short( uint8_t key_mask )
{
      cli();                                          // read key state and key press atomic !
      return buttons_get_press( ~key_state & key_mask );
}

 

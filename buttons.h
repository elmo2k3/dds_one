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
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdint.h>

//code taken from http://www.mikrocontroller.net/articles/Entprellung

#define BT_PORT PORTD
#define BT_PIN  PIND
#define BT_DDR  DDRD

#define BT_UP    PD5
#define BT_DOWN   PD6
#define BT_CM    PD7

#define REPEAT_MASK (1<<BT_UP | 1<<BT_DOWN)
#define REPEAT_START 50 // 500ms
#define REPEAT_NEXT 40 // 200ms

void buttons_init(void);
void buttons_every_10_ms(void);
uint8_t buttons_get_press(uint8_t key_mask);
uint8_t buttons_get_rpt( uint8_t key_mask );

#endif


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

#include "page_linear_sweep.h"
#include "lcd-routines.h"
#include "buttons.h"
#include "DDS.h"

/* Menu
 * 		1234567890123456
 * 	1	  Linear Sweep
 * 	2	f=100.0MHz  v=15
*/
void page_linear_sweep_draw(struct menuitem *self, uint8_t blink_f, uint8_t blink_g)
{
	lcd_clear();
	lcd_string(self->line1);
    dds_set_linear_sweep(1e6,200e6,1e2,1e2);
}

uint8_t page_linear_sweep_bt(struct menuitem *self, uint8_t button, uint8_t rpt)
{
    PORTA ^= (1<<PA0);
}

void page_linear_sweep_periodic(struct menuitem *self)
{
}
void page_linear_sweep_load_parameters(struct menuitem *self)
{
}
void page_linear_sweep_write_parameters(struct menuitem *self)
{
}


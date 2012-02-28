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

#include "DDS_ONE.h"

#ifndef __PAGE_LINEAR_SWEEP_H__
#define __PAGE_LINEAR_SWEEP_H__

void page_linear_sweep_draw(struct menuitem *self, uint8_t blink_f, uint8_t blink_g);
void page_linear_sweep_bt_f(struct menuitem *self, uint8_t button, uint8_t rpt);
//void page_single_tone_bt_g(struct menuitem *self, uint8_t button, uint8_t rpt);
//void page_single_tone_blink_f(struct menuitem *self);
//void page_single_tone_blink_g(struct menuitem *self);
//void page_single_tone_periodic(struct menuitem *self);

#endif


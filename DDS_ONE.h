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

#include <stdint.h>

#ifndef __DDS_ONE_H__
#define __DDS_ONE_H__

struct menuitem
{
    char line1[30];
	char line2[30];
    void (*draw_func)(struct menuitem *self);
    uint8_t (*button_func)(struct menuitem *self, uint8_t button, uint8_t rpt);
    void (*periodic_500ms_func)(struct menuitem *self);
    void (*parameter_load_func)();
    void (*parameter_save_func)();
};

#endif


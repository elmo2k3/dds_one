/*
 * Copyright (C) 2012 homerj00	<homerj00@web.de>
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
#include "fifo.h"

void fifo_init (fifo_t *f, uint8_t *buffer, const uint8_t size)
{
	f->count = 0;
	f->pread = f->pwrite = buffer;
	f->read2end = f->write2end = f->size = size;
}

uint8_t fifo_put (fifo_t *f, const uint8_t data)
{
	return _inline_fifo_put (f, data);
}

uint8_t fifo_get_wait (fifo_t *f)
{
	while (!f->count);
	
	return _inline_fifo_get (f);	
}

int fifo_get_nowait (fifo_t *f)
{
	if (!f->count)		return -1;
		
	return (int) _inline_fifo_get (f);	
}

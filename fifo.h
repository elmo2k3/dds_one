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
#ifndef FIFO_H
#define FIFO_H

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct
{
	uint8_t volatile count;       // # Zeichen im Puffer
	uint8_t size;                 // Puffer-Gr��e
	uint8_t *pread;               // Lesezeiger
	uint8_t *pwrite;              // Schreibzeiger
	uint8_t read2end, write2end;  // # Zeichen bis zum �berlauf Lese-/Schreibzeiger
} fifo_t;

extern void fifo_init (fifo_t*, uint8_t* buf, const uint8_t size);
extern uint8_t fifo_put (fifo_t*, const uint8_t data);
extern uint8_t fifo_get_wait (fifo_t*);
extern int fifo_get_nowait (fifo_t*);

static inline uint8_t
_inline_fifo_put (fifo_t *f, const uint8_t data)
{
	if (f->count >= f->size)
		return 0;
		
	uint8_t * pwrite = f->pwrite;
	
	*(pwrite++) = data;
	
	uint8_t write2end = f->write2end;
	
	if (--write2end == 0)
	{
		write2end = f->size;
		pwrite -= write2end;
	}
	
	f->write2end = write2end;
	f->pwrite = pwrite;

	uint8_t sreg = SREG;
	cli();
	f->count++;
	SREG = sreg;
	
	return 1;
}

static inline uint8_t 
_inline_fifo_get (fifo_t *f)
{
	uint8_t *pread = f->pread;
	uint8_t data = *(pread++);
	uint8_t read2end = f->read2end;
	
	if (--read2end == 0)
	{
		read2end = f->size;
		pread -= read2end;
	}
	
	f->pread = pread;
	f->read2end = read2end;
	
	uint8_t sreg = SREG;
	cli();
	f->count--;
	SREG = sreg;
	
	return data;
}

#endif /* FIFO_H */

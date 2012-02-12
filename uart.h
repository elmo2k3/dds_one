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

#ifndef UART_H
#define UART_H

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000
#endif


#define BAUDRATE 1000000
/* mögliche Baudraten mit 16MHz:
U2X=0: 1M, 0.5M. 0.25M
U2X=1: 2400 */

extern void uart_init (void);
extern void commando(void);
extern void uart_puts (const char *s);


static inline void uart_flush (void)
{
	while (UCSR1B & (1 << UDRIE));
}

#endif

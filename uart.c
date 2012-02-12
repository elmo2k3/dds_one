/*
 * Copyright (C) 2012 Norrin
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
#include "uart.h"
#include "fifo.h"
#include "lcd-routines.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

#define memzero(s, n) memset((s), 0, (n))

/*#define BUFSIZE_IN  0xF0
uint8_t inbuf[BUFSIZE_IN];
fifo_t infifo;*/

#define BUFSIZE_OUT 0xF0
uint8_t outbuf[BUFSIZE_OUT];
fifo_t outfifo;

// Ein Zeilenumbruch, abhängig davon, was die Gegenstelle haben will 
// Windows: "rn" 
// Linux  : "n" 
// MacOS  : "r" 
#define CR "\r\n"

static char inbuffer[40];
static uint8_t StringLen = 0;
static uint8_t NextChar;
uint8_t channel=1;

uint8_t uart_getc(void);
void uart_gets( char* Buffer, uint8_t MaxLen );

void uart_init(void)
{
	uint8_t sreg = SREG;
	uint16_t ubrr = (uint16_t) ((uint32_t) F_CPU/(16UL*BAUDRATE) - 1);
	
	cli();
	UBRR1H = (uint8_t)(ubrr>>8);
    UBRR1L = (uint8_t)(ubrr);
	
	//Double Speed Mode for Baudrate 2400
	UCSR1A |= (0<<U2X);
	/* Set frame format: 8data, 1stop bit */
	UCSR1C |= (1<<UCSZ1)|(1<<UCSZ0); 
	/* Enable receiver and transmitter and Interrupt for Receive*/
	UCSR1B |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);

	do
    {
        //Dummy Read 
        UDR1;
    }
    while (UCSR1A & (1 << RXC));

    // Rücksetzen von Receive und Transmit Complete-Flags 
    UCSR1A = (1 << RXC) | (1 << TXC);
	
	// FIFOs für Ein- und Ausgabe initialisieren 
    //fifo_init (&infifo,   inbuf, BUFSIZE_IN);
    fifo_init (&outfifo, outbuf, BUFSIZE_OUT);

	// Global Interrupt-Flag wieder herstellen 
    SREG = sreg;	
}


ISR (USART_RXC_vect)
{
	uart_gets(inbuffer, sizeof(inbuffer));
}

void commando(void)
{
	//Befehlsauswertung und -verarbeitung
	cli();
	if(strstr(inbuffer, "toggleosc"))
	{

		StringLen=0;
		memzero(inbuffer, sizeof(inbuffer));
		if(PORTB&(1<<PB1))
		{
			UDR1=255;
		}			
		else
		{
			UDR1=0;
		}			
	}
	else if(strstr(inbuffer, "channel1"))
	{

			StringLen=0;
			memzero(inbuffer, sizeof(inbuffer));
			UDR1=1;

	}
	else if(strstr(inbuffer, "channel2"))
	{

		StringLen=0;
		memzero(inbuffer, sizeof(inbuffer));
		UDR1=2;

	}
	else if(strstr(inbuffer, "temperature"))
	{
		StringLen=0;
		memzero(inbuffer, sizeof(inbuffer));

	}
	else if((strstr(inbuffer, "help")) || (strstr(inbuffer, "?")))
	{
		StringLen=0;
		memzero(inbuffer, sizeof(inbuffer));
		uart_puts("Command\t\t\t\tEffect\n"
					"toggleosc\t\t\t100MHz Oscillator on/off\n"
					"channel1\t\t\t\tSelect Channel 1 for Measurement (LT5534)\n"
					"channel2\t\t\t\tSelect Channel 2 for Measurement (Passive Detector)\n"
					"temperature\tShow Temperature in °C\n"
					"help\t\t\t\t\t\t\t\tList Commands\n");
	}
	sei();
}	
 
//Daten aus FIFO senden
ISR (USART_UDRE_vect)
{	
    if (outfifo.count > 0)
       UDR1 = _inline_fifo_get (&outfifo);
    else
        UCSR1B &= ~(1 << UDRIE);
}

//Einzelnes Zeichen empfangen
uint8_t uart_getc(void)
{
    while (!(UCSR1A & (1<<RXC)))
        ;
    return UDR1;
}

//Zeichenkette empfangen und in Puffer speichern
void uart_gets( char* Buffer, uint8_t MaxLen )
{
	NextChar = uart_getc();
	inbuffer[StringLen] = NextChar;
	StringLen++;
	
	if(StringLen == MaxLen)
	{
		StringLen=0;
		memzero(inbuffer, sizeof(inbuffer));
	}		
}

//Einzelnes Zeichen senden
int uart_putc (const uint8_t c)
{
    int ret = fifo_put (&outfifo, c);
	
    UCSR1B |= (1 << UDRIE);
	 
    return ret;
}

// Einen 0-terminierten String übertragen. 
void uart_puts (const char *s)
{
    do
    {
        uart_putc (*s);
    }
    while (*s++);
}

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
#include <util/delay.h>
#include "DDS.h"

#define DDS_CS_SET() 	SPI_PORT |= (1<<DDS_CS);
#define DDS_CS_CLR() 	SPI_PORT &= ~(1<<DDS_CS);
#define DDS_DO_RESET()		DDS_PORT |= (1<<DDS_RESET); DDS_PORT &= ~(1<<DDS_RESET);
#define DDS_DO_IOUPDATE()	DDS_PORT |= (1<<DDS_IOUPDATE); DDS_PORT &= ~(1<<DDS_IOUPDATE);

#define FREQUENCY_FACTOR 10.73741824

static void dds_cmd(uint8_t address, uint8_t read_access, uint32_t value, uint8_t size);

void dds_init(void)
{
	//VGA Initialisierung
	VGA_DDR |= (1<<VGA_G3) | (1<<VGA_G2) | (1<<VGA_G1) | (1<<VGA_G0);
	VGA_PORT = 0;
	
	//DDS Initialisierung
	DDS_DDR |= (1<<DDS_PWRDWNCTL) | (1<<DDS_RESET) | (1<<DDS_IOSYNC) |
			(1<<DDS_CLKMODESEL) | (1<<DDS_IOUPDATE) | (1<<DDS_PS1) | (1<<DDS_PS1);
	DDS_PORT |= (1<<DDS_CLKMODESEL);
	
	
	//SPI Initialisierung
	SPI_DDR |= (1<<DDS_CS) | (1<<SPI_MOSI) | (1<<SPI_SCK);
	SPI_PORT |= (1<<DDS_CS) | (1<<PB0);
	/* Enable SPI, Master, set clock rate fck/128 */
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0)|(0<<CPOL);
	//Double Speed Mode
	SPSR |= (0<<SPI2X);

	DDS_DO_RESET();

	// set AutoClr Phase Accum
	dds_cmd(DDS_REGISTER_CFR1, 0, 0x00000200, DDS_REGISTER_CFR1_LENGTH);
	// set REFCLK Multiplier to 16 and activate high frequency VCO range
	// running at 400MHz now
	dds_cmd(DDS_REGISTER_CFR2, 0, 0x00000084, DDS_REGISTER_CFR2_LENGTH);
}

void dds_vga_set_gain(uint8_t gain)
{
	gain = (gain & 0x0F) | (VGA_PORT & 0xF0);
	VGA_PORT = gain;
}

void dds_set_frequency(uint32_t frequency)
{
	dds_cmd(DDS_REGISTER_FTW0,0, FREQUENCY_FACTOR*frequency, DDS_REGISTER_FTW0_LENGTH);
	DDS_DO_IOUPDATE();
}

static void dds_cmd(uint8_t address, uint8_t read_access, uint32_t value, uint8_t size)
{
	uint8_t i;
	uint32_t temp;

	DDS_CS_CLR(); // Start transfer

	SPDR = (address&0x1F) | (read_access<<7);
	while(!(SPSR & (1<<SPIF))); // wait until transmission finished
	
	if(size>4) size = 4; // not more than 4 bytes allowed
	for(i=0;i<size;i++)
	{
		// Start with upper byte (MSB first)
		temp = (value>>((size-i-1)*8));
		SPDR = temp & 0xFF;
    	while(!(SPSR & (1<<SPIF))); // wait until transmission finished
	}

	DDS_CS_SET(); // End of transfer
}


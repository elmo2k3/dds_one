/*
 * Copyright (C) 2012 Bjoern Biesenbach <bjoern at bjoern-b.de>
 *               2012 Norrin
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

#ifndef DDS_H_
#define DDS_H_

//Variable Gain Amplifier Pins
#define VGA_DDR DDRG
#define VGA_PORT PORTG
#define VGA_G0 0
#define VGA_G1 1
#define VGA_G2 2
#define VGA_G3 3

//DDS Pins
#define DDS_DDR DDRA
#define DDS_PORT PORTA
#define DDS_PS0 0
#define DDS_PS1 1
#define DDS_IOUPDATE 2
#define DDS_CLKMODESEL 3
#define DDS_IOSYNC 4
#define DDS_RESET 5
#define DDS_PWRDWNCTL 6

//DDS SPI Pins
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define DDS_CS 4
#define SPI_MOSI 2
#define SPI_MISO 3
#define SPI_SCK 1

#define DDS_REGISTER_CFR1 0x00
#define DDS_REGISTER_CFR1_LENGTH 4
#define DDS_REGISTER_CFR2 0x01
#define DDS_REGISTER_CFR2_LENGTH 3
#define DDS_REGISTER_FTW0 0x04
#define DDS_REGISTER_FTW0_LENGTH 4
#define DDS_REGISTER_FTW1 0x06
#define DDS_REGISTER_FTW1_LENGTH 4

void dds_init(void);
void dds_set_frequency(uint32_t frequency);
void dds_vga_set_gain(uint8_t gain);

#endif /* DDS_H_ */

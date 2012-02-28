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

#ifndef DDS_H_
#define DDS_H_

//Variable Gain Amplifier Pins
#define VGA_DDR DDRG
#define VGA_PORT PORTG
#define VGA_G0 PG0
#define VGA_G1 PG1
#define VGA_G2 PG2
#define VGA_G3 PG3

//DDS Pins
#define DDS_DDR DDRA
#define DDS_PORT PORTA
#define DDS_PS0 PA0
#define DDS_PS1 PA1
#define DDS_IOUPDATE PA2
#define DDS_CLKMODESEL PA3
#define DDS_IOSYNC PA4
#define DDS_RESET PA5
#define DDS_PWRDWNCTL PA6

//DDS SPI Pins
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define DDS_CS PB4
#define SPI_MOSI PB2
#define SPI_MISO PB3
#define SPI_SCK PB1

#define DDS_REGISTER_CFR1 0x00
#define DDS_REGISTER_CFR1_LENGTH 4
#define DDS_REGISTER_CFR2 0x01
#define DDS_REGISTER_CFR2_LENGTH 3
#define DDS_REGISTER_FTW0 0x04
#define DDS_REGISTER_FTW0_LENGTH 4
#define DDS_REGISTER_FTW1 0x06
#define DDS_REGISTER_FTW1_LENGTH 4
#define DDS_REGISTER_NLSCW 0x07
#define DDS_REGISTER_NLSCW_LENGTH 5
#define DDS_REGISTER_PLSCW 0x08
#define DDS_REGISTER_PLSCW_LENGTH 5

void dds_init(void);
void dds_set_single_tone_frequency(uint32_t frequency);
void dds_set_linear_sweep(uint32_t f_start, uint32_t f_stop, uint32_t step_up, uint32_t step_down);
void dds_vga_set_gain(uint8_t gain);

#endif /* DDS_H_ */

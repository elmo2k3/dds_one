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
#include "adc.h"
#include <avr/io.h>

/* ADC initialisieren */
void ADC_Init(void) {
 
  uint16_t result;
 
  // ADMUX = (0<<REFS1) | (1<<REFS0);      // AVcc als Referenz benutzen (2.56V)
  ADMUX = (1<<REFS1) | (1<<REFS0);      // interne Referenzspannung nutzen
  // Bit ADFR ("free running") in ADCSRA steht beim Einschalten
  // schon auf 0, also single conversion
  ADCSRA = (1<<ADPS1) | (1<<ADPS0);     // Frequenzvorteiler
  ADCSRA |= (1<<ADEN);                  // ADC aktivieren
 
  /* nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
     also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
 
  ADCSRA |= (1<<ADSC);                  // eine ADC-Wandlung 
  while (ADCSRA & (1<<ADSC) ) {}        // auf Abschluss der Konvertierung warten
  /* ADCW muss einmal gelesen werden, sonst wird Ergebnis der nächsten
     Wandlung nicht übernommen. */
  result = ADCW;
}
 
/* ADC Einzelmessung */
uint16_t ADC_Read( uint8_t channel )
{
  // Kanal waehlen, ohne andere Bits zu beeinflußen
  ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
  ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
  while (ADCSRA & (1<<ADSC) ) {}  // auf Abschluss der Konvertierung warten
  return ADCW;                    // ADC auslesen und zurückgeben
}

/* ADC Mehrfachmessung mit Mittelwertbbildung */
uint16_t ADC_Read_Avg( uint8_t channel, uint8_t average )
{
  uint32_t result = 0;
 
  for (uint8_t i = 0; i < average; ++i )
    result += ADC_Read( channel );
 
  return (uint16_t)( result / average );
}
 
 
/* Beispielaufrufe: 
int main()
{
  uint16_t adcval;
  ADC_Init();
 
  while( 1 ) {
    adcval = ADC_Read(0);  // Kanal 0
    // mach was mit adcval
 
    adcval = ADC_Read_Avg(2, 4);  // Kanal 2, Mittelwert aus 4 Messungen
    // mach was mit adcval
  }
}*/

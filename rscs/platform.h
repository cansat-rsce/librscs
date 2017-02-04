/*
 * platform.h
 *
 *  Created on: 17 нояб. 2016 г.
 *      Author: snork
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <avr/io.h>


#ifdef __AVR_ATmega328P__
#	define RSCS_SPI_PORTX PORTB
#	define RSCS_SPI_DDRX DDRB
#	define RSCS_SPI_MISO 4
#	define RSCS_SPI_MOSI 3
#	define RSCS_SPI_SCK 5
#	define RSCS_SPI_SS 2
#elif defined __AVR_ATmega128__
#	define RSCS_SPI_PORTX PORTB
#	define RSCS_SPI_DDRX DDRB
#	define RSCS_SPI_MISO 3
#	define RSCS_SPI_MOSI 2
#	define RSCS_SPI_SCK 1
#	define RSCS_SPI_SS 0
#endif





#endif /* PLATFORM_H_ */

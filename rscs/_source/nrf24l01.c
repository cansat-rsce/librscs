/*
 * nrf24l01.c
 *
 *  Created on: 10 февр. 2018 г.
 *      Author: developer
 */

//#include <util/atomic.h>
#include <stdio.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdint.h>

#include "error.h"

#include "nrf24l01.h"

#define R_REG			0x00
#define W_REG			0x20
#define R_RX_PAY	    0x61
#define W_TX_PAY        0xA0
#define FL_TX			0xE1
#define FL_RX			0xE2
#define RE_TX_PL		0xE3
#define ACT				0x50
#define R_RX_PL_WID		0x60
#define W_ACK_PAY		0xA8
#define W_TX_PAY_NACK   0x58
#define NOP				0xFF

#define STATUS			0x07
	#define RX_DR		6
	#define TX_DS		5
	#define MAX_RT		4
	#define RX_P_NO2	3
	#define RX_P_NO1	2
	#define RX_P_NO0	1
	#define TX_FULL		0

#define CONFIG			0x00
	#define MASK_RX_DR  6
	#define MASK_TX_DS  5
	#define MASK_MAX_RT 4
	#define EN_CRC      3
	#define CRCO        2
	#define PWR_UP      1
	#define PRIM_RX     0

#define DYNPD			0x1C
	#define DPL_P5		5
	#define DPL_P4		4
	#define DPL_P3		3
	#define DPL_P2		2
	#define DPL_P1		1
	#define DPL_P0		0

#define FEATURE			0x1D
	#define EN_DPL		2
	#define EN_ACK_PAY	1
	#define EN_DYN_ACK  0

#define RF_CH			0x05

#define RF_SETUP 		0x06

#define SETUP_RETR 		0x04

#define RX_PW_P0		0x11

#define spi_start(bus) (*bus->CSPORT &= ~bus->CSMASK)
#define spi_stop(bus) (*bus->CSPORT |= bus->CSMASK)
#define spi_ex(bus, val) (bus->exchange(val))


struct rscs_nrf24l01_bus_t{
	uint8_t (*exchange)(uint8_t byte); //SPI exchange function

	volatile uint8_t * CSPORT; //Chip Select SPI
	uint8_t CSMASK;

	volatile uint8_t * CEPORT; //Chip enable NRF
	uint8_t CEMASK;
};

static uint8_t _rreg(uint8_t reg, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	uint8_t retval = spi_ex(bus, reg | R_REG);

	if(reg == STATUS) {
		spi_stop(bus);
		return retval;
	}
	retval = spi_ex(bus, NOP);

	spi_stop(bus);
	return retval;
}

static void _wreg(uint8_t reg, uint8_t val, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	spi_ex(bus, reg | W_REG);
	spi_ex(bus, val);

	spi_stop(bus);
}

void rscs_nrf24l01_configure(rscs_nrf24l01_bus_t * bus, uint8_t config){
	_wreg(CONFIG, config | (1 << PWR_UP), bus);

	_delay_us(135);
}

void rscs_nrf24l01_feature(rscs_nrf24l01_bus_t * bus, uint8_t feature){
	_wreg(FEATURE, feature, bus);
}

void rscs_nrf24l01_en_aa(rscs_nrf24l01_bus_t * bus, ){

}

rscs_nrf24l01_bus_t * rscs_nrf24l01_init(uint8_t (*exchange)(uint8_t byte),
											volatile uint8_t * CSPORT, uint8_t cspin,
											volatile uint8_t * CEPORT, uint8_t cepin)
{
	rscs_nrf24l01_bus_t * retval = malloc(sizeof(rscs_nrf24l01_bus_t));

	if(!retval) return NULL;

	retval->exchange = exchange;

	retval->CSPORT = CSPORT;
	retval->CSMASK = (1 << cspin);

	retval->CEPORT = CEPORT;
	retval->CEMASK = (1 << cepin);

	return retval;
}

void info(rscs_nrf24l01_bus_t * bus){
	printf("\n");
	printf("CFG: %d\n", _rreg(CONFIG, bus));
	printf("STATUS: %d\n", _rreg(STATUS, bus));
	printf("RF_CH: %d\n", _rreg(RF_CH, bus));
	printf("RF_SETUP: %d\n", _rreg(RF_SETUP, bus));
	printf("FEATURE: %d\n", _rreg(FEATURE, bus));
	printf("SETUP_RETR: %d\n", _rreg(SETUP_RETR, bus));
	printf("P0: %d\n", _rreg(RX_PW_P0, bus));
	*bus->CSPORT &= ~bus->CSMASK;
	bus->exchange(R_REG | 0x0A);
	for(int i = 0; i < 5; i++) printf("%d ", bus->exchange(NOP));
	*bus->CSPORT |= bus->CSMASK;
	printf("\n");
	*bus->CSPORT &= ~bus->CSMASK;
	bus->exchange(R_REG | 0x10);
	for(int i = 0; i < 5; i++) printf("%d ", bus->exchange(NOP));
	*bus->CSPORT |= bus->CSMASK;
}

uint8_t test(rscs_nrf24l01_bus_t * nrf){
	rscs_nrf24l01_configure(nrf, RSCS_NRF24L01_PTX |
			RSCS_NRF24L01_CRC_2B | RSCS_NRF24L01_EN_CRC);

	info(nrf);

	return 0;
}











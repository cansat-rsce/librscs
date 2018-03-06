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


struct rscs_nrf24l01_bus_t{
	uint8_t (*exchange)(uint8_t byte); //SPI exchange function

	volatile uint8_t * CSPORT; //Chip Select SPI
	uint8_t CSMASK;

	volatile uint8_t * CEPORT; //Chip enable NRF
	uint8_t CEMASK;

	rscs_nrf24l01_mode_t mode;
};

static uint8_t _rreg(uint8_t reg, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	uint8_t retval = bus->exchange(reg | R_REG);

	if(reg == STATUS) {
		*bus->CSPORT |= bus->CSMASK;
		return retval;
	}

	retval = bus->exchange(NOP);

	spi_stop(bus);

	return retval;
}

static void _wreg(uint8_t reg, uint8_t val, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	bus->exchange(reg | W_REG);
	bus->exchange(val);

	spi_stop(bus);
}

static inline void _pwr_up(rscs_nrf24l01_bus_t * bus){
	_wreg(CONFIG, _rreg(CONFIG, bus) | (1 << PWR_UP), bus);

	_delay_us(200);
}

static inline void _pwr_down(rscs_nrf24l01_bus_t * bus){
	_wreg(CONFIG, _rreg(CONFIG, bus) & ~(1 << PWR_UP), bus);

	_delay_us(200);
}

static inline void _enable(rscs_nrf24l01_bus_t * bus){
	*bus->CEPORT |= bus->CEMASK;

	_delay_us(200);
}

static inline void _disable(rscs_nrf24l01_bus_t * bus){
	*bus->CEPORT &= ~bus->CEMASK;

	_delay_us(200);
}

static void _clock_data(void *data, size_t size, rscs_nrf24l01_bus_t *bus){
	uint8_t *buf = (uint8_t *)data;

	spi_start(bus);

	bus->exchange(W_TX_PAY);
	while(size--) bus->exchange(*buf++);

	spi_stop(bus);
}

static uint8_t _get_data(void *data, rscs_nrf24l01_bus_t *bus){
	uint8_t *buf = (uint8_t *)data;

	spi_start(bus);

	uint8_t len = bus->exchange(R_RX_PL_WID);
	bus->exchange(R_RX_PAY);
	for(int i = 0; i < len; i++) buf[i] = bus->exchange(NOP);
	bus->exchange(FL_RX);

	spi_stop(bus);

	return len;
}

static void _set_mode(rscs_nrf24l01_bus_t *bus, rscs_nrf24l01_mode_t mode){
	if(mode == RSCS_NRF24L01_MODE_PRX){
		_wreg(CONFIG, _rreg(CONFIG, bus) | (1 << PRIM_RX), bus);
	}
	else if(mode == RSCS_NRF24L01_MODE_PTX){
		_wreg(CONFIG, (_rreg(CONFIG, bus) & ~(1 << PRIM_RX)), bus);
	}
	else return;

	bus->mode = mode;

	_delay_us(200);
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

uint8_t test(rscs_nrf24l01_bus_t * PTX, rscs_nrf24l01_bus_t * PRX){
	_disable(PRX);
	_disable(PTX);

	_wreg(CONFIG, (1 << EN_CRC) | (1 << PWR_UP) | (1 << PRIM_RX), PRX);
	_delay_ms(100);
	_wreg(CONFIG, (1 << EN_CRC) | (1 << PWR_UP), PTX);
	_delay_ms(100);
	_wreg(RX_PW_P0, 1, PTX);
	_wreg(RX_PW_P0, 1, PRX);

	info(PTX);
	printf("\n");
	info(PRX);
	printf("\n-----------------------------------\n");

	*PTX->CSPORT &= ~PTX->CSMASK;
	PTX->exchange(W_TX_PAY);
	PTX->exchange(64);
	*PTX->CSPORT |= PTX->CSMASK;

	info(PTX);
	printf("\n");
	info(PRX);
	printf("\n-----------------------------------\n");

	_enable(PRX);
	_delay_ms(100);
	_enable(PTX);

	_delay_ms(100);

	info(PTX);
	printf("\n");
	info(PRX);
	printf("\n-----------------------------------\n");

	*PRX->CSPORT &= ~PRX->CSMASK;
	PRX->exchange(R_RX_PAY);
	uint8_t res = PRX->exchange(NOP);
	*PRX->CSPORT |= PRX->CSMASK;

	printf("%d", res);

	while(1){}
	/*_pwr_up(PTX);
	_pwr_up(PRX);

	_wreg(FEATURE, (1 << EN_DPL), PTX);
	_wreg(FEATURE, (1 << EN_DPL), PRX);

	_set_mode(PTX, RSCS_NRF24L01_MODE_PTX);
	_set_mode(PRX, RSCS_NRF24L01_MODE_PTX);

	char arr[] = "abcd";

	while(1){
		printf("writing\n");
		_clock_data(arr, sizeof(arr), PTX);
		_enable(PTX);

		while(1){
			uint8_t reg = _rreg(STATUS, PTX);

			if(reg & (1 << 4)){
				printf("max retransmit-%d\n", reg);
				break;
			}

			if(reg & (1 << 5)){
				printf("transmit-%d\n", reg);
				break;
			}

			printf("waiting transmit-%d\n", reg);
		}
		_disable(bus);

		_wreg(STATUS, _rreg(STATUS, PTX), PTX);
		printf("cleared\n--------------------\n");

		_delay_ms(1000);
	}*/


	return 0;
}











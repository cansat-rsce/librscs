/*
 * nrf24l01.c
 *
 *  Created on: 10 февр. 2018 г.
 *      Author: developer
 */

//#include <util/atomic.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdbool.h>

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

#define EN_RXADDR		0x02

#define SETUP_AW		0x03

#define RX_ADDR_P0		0x0A

#define TX_ADDR 		0x10

#define FIFO_STATUS		0x17

#define chip_en(bus) (*bus->CEPORT |= bus->CEMASK)
#define chip_dis(bus) (*bus->CEPORT &= ~bus->CEMASK)
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

void rscs_nrf24l01_set_config(rscs_nrf24l01_bus_t * bus, uint8_t config){
	_wreg(CONFIG, config | (1 << PWR_UP), bus);
	_delay_us(135);
	if(config & RSCS_NRF24L01_PRX) chip_en(bus);
	else chip_dis(bus);
}

void rscs_nrf24l01_set_feature(rscs_nrf24l01_bus_t * bus, uint8_t feature){
	_wreg(FEATURE, feature, bus);
}

void rscs_nrf24l01_set_pipe(rscs_nrf24l01_bus_t * bus, uint8_t pipe_num, bool enable){
	if(enable) _wreg(EN_RXADDR, _rreg(EN_RXADDR, bus) | (1 << pipe_num), bus);
	else _wreg(EN_RXADDR, _rreg(EN_RXADDR, bus) & ~(1 << pipe_num), bus);
}

void rscs_nrf24l01_set_pipe_bytes(rscs_nrf24l01_bus_t * bus, uint8_t pipe_num, uint8_t bytes){
	_wreg(RX_PW_P0 + pipe_num, bytes, bus);
}

void rscs_nrf24l01_set_pipe_dpl(rscs_nrf24l01_bus_t * bus, uint8_t pipe_num, bool enable){
	if(enable) _wreg(DYNPD, _rreg(DYNPD, bus) | (1 << pipe_num), bus);
	else _wreg(DYNPD, _rreg(DYNPD, bus) & ~(1 << pipe_num), bus);
}

void rscs_nrf24l01_set_rx_addr(rscs_nrf24l01_bus_t * bus, uint8_t pipe_num, uint64_t addr){
	uint8_t addr_len = (_rreg(SETUP_AW, bus) + 2);

	spi_start(bus);

	spi_ex(bus, W_REG | (RX_ADDR_P0 + pipe_num));
	for(int i = 0; i < addr_len; i++){
		spi_ex(bus, (uint8_t)((addr >> (i * 8)) & 0xFF));
	}

	spi_stop(bus);
}

void rscs_nrf24l01_set_tx_addr(rscs_nrf24l01_bus_t * bus, uint64_t addr){
	uint8_t addr_len = (_rreg(SETUP_AW, bus) + 2);

	spi_start(bus);

	spi_ex(bus, W_REG | TX_ADDR);
	for(int i = 0; i < addr_len; i++){
		spi_ex(bus, (uint8_t)((addr >> (i * 8)) & 0xFF));
	}

	spi_stop(bus);
}

void rscs_nrf24l01_set_retr(rscs_nrf24l01_bus_t * bus, uint8_t delay, uint8_t count){
	_wreg(SETUP_RETR, (delay << 4) | (count & 0xF), bus);
}

uint8_t rscs_nrf24l01_write(rscs_nrf24l01_bus_t * bus, void* data, size_t size){
	size = size > 32 ? 32 : size;
	uint8_t* buf = (uint8_t*)data;

	spi_start(bus);
	spi_ex(bus, FL_TX);
	spi_stop(bus);

	if(_rreg(CONFIG, bus) & (1 << PRIM_RX)){
		spi_start(bus);
		spi_ex(bus, W_ACK_PAY);
		for(int i = 0; i < size; i++) spi_ex(bus, *(buf + i));
		spi_stop(bus);
	}
	else{
		spi_start(bus);
		spi_ex(bus, W_TX_PAY);
		for(int i = 0; i < size; i++) spi_ex(bus, *(buf + i));
		spi_stop(bus);

		chip_en(bus);
		_delay_us(20);
		chip_dis(bus);
	}

	return size;
}

uint8_t rscs_nrf24l01_read(rscs_nrf24l01_bus_t * bus, void* data){
	spi_start(bus);

	spi_ex(bus, R_RX_PL_WID);
	uint8_t width = spi_ex(bus, NOP);

	spi_stop(bus);

	if(width == 0) return 0;
	uint8_t* buf = (uint8_t*)data;

	spi_start(bus);

	spi_ex(bus, R_RX_PAY);
	for(int i = 0; i < width; i++) *(buf + i) = spi_ex(bus, NOP);

	spi_stop(bus);

	return width;
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
	printf("\n---------------------\n");
	printf("CFG: %d\n", _rreg(CONFIG, bus));
	printf("STATUS: %d\n", _rreg(STATUS, bus));
	printf("RF_CH: %d\n", _rreg(RF_CH, bus));
	printf("RF_SETUP: %d\n", _rreg(RF_SETUP, bus));
	printf("FEATURE: %d\n", _rreg(FEATURE, bus));
	printf("SETUP_RETR: %d\n", _rreg(SETUP_RETR, bus));
	printf("DYNPD: %d\n", _rreg(DYNPD, bus));
	printf("EN_RXADDR: %d\n", _rreg(EN_RXADDR, bus));
	printf("SETUP_AW: %d\n", _rreg(SETUP_AW, bus));
	printf("FIFO_STATUS: %d\n", _rreg(FIFO_STATUS, bus));

	for(uint8_t i = 0; i < 6; i++) printf("P%d: %d\n", i, _rreg(RX_PW_P0 + i, bus));

	for(uint8_t i = 0; i < 6; i++){
		printf("RX_ADDR %d: ", i);
		*bus->CSPORT &= ~bus->CSMASK;
		bus->exchange(R_REG | (0x0A + i));
		for(int k = 0; k < 5; k++) printf("%d ", bus->exchange(NOP));
		*bus->CSPORT |= bus->CSMASK;
		printf("\n");
	}

	printf("TX_ADDR: ");
	*bus->CSPORT &= ~bus->CSMASK;
	bus->exchange(R_REG | 0x10);
	for(int i = 0; i < 5; i++) printf("%d ", bus->exchange(NOP));
	*bus->CSPORT |= bus->CSMASK;
	printf("\n---------------------\n");
}

uint8_t test(rscs_nrf24l01_bus_t * nrf1, rscs_nrf24l01_bus_t * nrf2, rscs_uart_bus_t* uart){
	rscs_nrf24l01_set_config(nrf1, RSCS_NRF24L01_EN_CRC | RSCS_NRF24L01_CRC_2B | RSCS_NRF24L01_PTX);
	rscs_nrf24l01_set_feature(nrf1, RSCS_NRF24L01_EN_DPL | RSCS_NRF24L01_EN_DYN_ACK | RSCS_NRF24L01_EN_ACK_PAY);
	rscs_nrf24l01_set_pipe(nrf1, 0, true);
	rscs_nrf24l01_set_pipe(nrf1, 1, false);
	//rscs_nrf24l01_set_pipe_bytes(nrf1, 0, 1);
	//rscs_nrf24l01_set_pipe_dpl(nrf1, 0, true);
	//rscs_nrf24l01_set_rx_addr(nrf1, 0, 0x1122334455);
	//rscs_nrf24l01_set_tx_addr(nrf1, 0x1122334455);
	//rscs_nrf24l01_set_retr(nrf1, 5, 5);

	rscs_nrf24l01_set_config(nrf2, RSCS_NRF24L01_EN_CRC | RSCS_NRF24L01_CRC_2B | RSCS_NRF24L01_PRX);
	rscs_nrf24l01_set_feature(nrf2, RSCS_NRF24L01_EN_DPL | RSCS_NRF24L01_EN_DYN_ACK | RSCS_NRF24L01_EN_ACK_PAY);
	rscs_nrf24l01_set_pipe(nrf2, 0, true);
	rscs_nrf24l01_set_pipe(nrf2, 1, false);
	//rscs_nrf24l01_set_pipe_bytes(nrf2, 0, 1);
	//rscs_nrf24l01_set_pipe_dpl(nrf2, 0, true);
	//rscs_nrf24l01_set_rx_addr(nrf2, 0, 0x1122334455);
	//rscs_nrf24l01_set_tx_addr(nrf2, 0x1122334455);
	//rscs_nrf24l01_set_retr(nrf2, 5, 5);

	char buf[33], data[33];
	size_t size, temp;

	while(1) {
		info(nrf1);
		info(nrf2);

		while(!(size = rscs_uart_read_some(uart, buf, sizeof(buf) - 1))) {}
		_delay_ms(10);
		while(size < 32 && (temp = rscs_uart_read_some(uart, buf + size, sizeof(buf) - size - 1))){
			size += temp;
			_delay_ms(10);
		}
		buf[size] = 0;
		printf("GOT FROM UART: %s\n", buf);

		rscs_nrf24l01_write(nrf1, buf, size);

		_delay_ms(2000);

		size = rscs_nrf24l01_read(nrf2, data);
		data[size] = 0;
		printf("GOT FROM NRF: %s\n", data);
	}

	return 0;
}











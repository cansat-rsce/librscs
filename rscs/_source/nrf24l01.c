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
	#define RX_P_NO		1
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
	#define PLL_LOCK	4
	#define RF_DR		3
	#define RF_PWR		1
	#define LNA_HCURR	0

#define SETUP_RETR 		0x04
	#define ARD			4
	#define ARC			0

#define RX_PW_P0		0x11

#define EN_RXADDR		0x02

#define SETUP_AW		0x03

#define RX_ADDR_P0		0x0A

#define TX_ADDR 		0x10

#define FIFO_STATUS		0x17

#define EN_AA			0x01

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

static void inline _command(uint8_t com, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	spi_ex(bus, com);

	spi_stop(bus);
}

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

static uint64_t _rreg5(uint8_t reg, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	uint64_t retval = 0;
	spi_ex(bus, reg | R_REG);
	for(int i = 0; i < 5; i++)
		retval |= (uint64_t)spi_ex(bus, NOP) << (i * 8);

	spi_stop(bus);

	return retval;
}

static void _wreg5(uint8_t reg, uint64_t val, rscs_nrf24l01_bus_t * bus){
	spi_start(bus);

	spi_ex(bus, reg | W_REG);
	for(int i = 0; i < 5; i++)
		spi_ex(bus, (uint8_t)((val >> (i * 8)) & 0xFF));

	spi_stop(bus);
}



rscs_nrf24l01_config_t* rscs_nrf24l01_get_config(rscs_nrf24l01_bus_t * bus){
	rscs_nrf24l01_config_t* retval = (rscs_nrf24l01_config_t*)malloc(sizeof(rscs_nrf24l01_config_t));
	if(retval == NULL) return NULL;

	uint8_t config 		= _rreg(CONFIG, bus);
	uint8_t setup_aw 	= _rreg(SETUP_AW, bus);
	uint8_t setup_retr 	= _rreg(SETUP_RETR, bus);
	uint8_t rf_ch 		= _rreg(RF_CH, bus);
	uint8_t rf_setup 	= _rreg(RF_SETUP, bus);
	uint8_t feature 	= _rreg(FEATURE, bus);
	uint64_t tx_addr 	= _rreg5(TX_ADDR, bus);

	retval->config.rx_dr 		= (config >> RX_DR) & 1;
	retval->config.tx_ds 		= (config >> TX_DS) & 1;
	retval->config.max_rt 		= (config >> MAX_RT) & 1;
	retval->config.en_crc 		= (config >> EN_CRC) & 1;
	retval->config.crc0 		= (config >> CRCO) & 1;
	retval->config.pwr_up 		= (config >> PWR_UP) & 1;
	retval->config.prim_rx 		= (config >> PRIM_RX) & 1;

	retval->setup_aw.aw 		= setup_aw;

	retval->setup_retr.ard 		= (setup_retr >> ARD) & 0b1111;
	retval->setup_retr.arc 		= (setup_retr >> ARC) & 0b1111;

	retval->rf_ch.rf_ch 		= rf_ch;

	retval->rf_setup.pll_lock 	= (rf_setup >> PLL_LOCK) & 1;
	retval->rf_setup.rf_dr 		= (rf_setup >> RF_DR) & 1;
	retval->rf_setup.rf_pwr 	= (rf_setup >> RF_PWR) & 0b11;
	retval->rf_setup.lna_hcurr 	= (rf_setup >> LNA_HCURR) & 1;

	retval->feature.en_dpl 		= (feature >> EN_DPL) & 1;
	retval->feature.en_ack_pay 	= (feature >> EN_ACK_PAY) & 1;
	retval->feature.en_dyn_ack 	= (feature >> EN_DYN_ACK) & 1;

	retval->tx.addr 			= tx_addr;

	return retval;
}

void rscs_nrf24l01_set_config(rscs_nrf24l01_config_t* set, rscs_nrf24l01_bus_t * bus){
	uint8_t config 		= (set->config.rx_dr << RX_DR) |
						  (set->config.tx_ds << TX_DS) |
						  (set->config.max_rt << MAX_RT) |
						  (set->config.en_crc << EN_CRC) |
						  (set->config.crc0 << CRCO) |
						  (set->config.pwr_up << PWR_UP) |
						  (set->config.prim_rx << PRIM_RX);

	uint8_t setup_retr 	= (set->setup_retr.arc << ARC) |
						  (set->setup_retr.ard << ARD);

	uint8_t rf_setup 	= (set->rf_setup.pll_lock << PLL_LOCK) |
					      (set->rf_setup.rf_dr << RF_DR) |
					      (set->rf_setup.rf_pwr << RF_PWR) |
					      (set->rf_setup.lna_hcurr << LNA_HCURR);

	uint8_t feature 	= (set->feature.en_ack_pay << EN_ACK_PAY) |
					      (set->feature.en_dpl << EN_DPL) |
					      (set->feature.en_dyn_ack << EN_DYN_ACK);

	_wreg(FEATURE, feature, bus);
	_wreg(RF_SETUP, rf_setup, bus);
	_wreg(SETUP_RETR, setup_retr, bus);
	_wreg(SETUP_AW, set->setup_aw.aw, bus);
	_wreg(RF_CH, set->rf_ch.rf_ch, bus);
	_wreg5(TX_ADDR, set->tx.addr, bus);
	_wreg(CONFIG, config, bus);

	if(set->config.pwr_up) _delay_us(135);
	if(set->config.prim_rx) chip_en(bus);
}


rscs_nrf24l01_pipe_config_t* rscs_nrf24l01_get_pipe_config(uint8_t num, rscs_nrf24l01_bus_t * bus){
	if(num > 5) return NULL;

	rscs_nrf24l01_pipe_config_t* retval = (rscs_nrf24l01_pipe_config_t*)malloc(sizeof(rscs_nrf24l01_pipe_config_t));
	if(retval == NULL) return NULL;

	retval->num 	= num;
	retval->en 		= (_rreg(EN_RXADDR, bus) >> retval->num) & 1;
	retval->en_aa 	= (_rreg(EN_AA, bus) >> retval->num) & 1;
	retval->en_dpl	= (_rreg(DYNPD, bus) >> retval->num) & 1;
	retval->pw 		= _rreg(RX_PW_P0 + retval->num, bus);
	retval->rx_addr = _rreg5(RX_ADDR_P0 + retval->num, bus);

	return retval;
}

void rscs_nrf24l01_set_pipe_config(rscs_nrf24l01_pipe_config_t* set, rscs_nrf24l01_bus_t * bus){
	uint8_t en_aa = _rreg(EN_AA, bus);
	uint8_t en = _rreg(EN_RXADDR, bus);
	uint8_t en_dpl = _rreg(DYNPD, bus);

	if(set->en) 	en |= (1 << set->num);
	else 			en |= ~(1 << set->num);

	if(set->en_dpl) en_dpl |= (1 << set->num);
	else 			en_dpl |= ~(1 << set->num);

	if(set->en_aa) 	en_aa |= (1 << set->num);
	else 		   	en_aa |= ~(1 << set->num);

	_wreg(EN_AA, en_aa, bus);
	_wreg(EN_RXADDR, en, bus);
	_wreg(DYNPD, en_dpl, bus);
	_wreg(RX_PW_P0 + set->num, set->pw, bus);
	_wreg5(RX_ADDR_P0 + set->num, set->rx_addr, bus);
}


uint8_t rscs_nrf24l01_write(rscs_nrf24l01_bus_t * bus, void* data, size_t size){
	size = size > 32 ? 32 : size;
	uint8_t* buf = (uint8_t*)data;

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


rscs_nrf24l01_status_t* rscs_nrf24l01_get_status(rscs_nrf24l01_bus_t * bus){
	rscs_nrf24l01_status_t* retval = (rscs_nrf24l01_status_t*)malloc(sizeof(rscs_nrf24l01_status_t));
	if(retval == NULL) return NULL;

	uint8_t status = _rreg(STATUS, bus);

	retval->max_rt 	= (status >> MAX_RT) & 1;
	retval->rx_dr 	= (status >> RX_DR) & 1;
	retval->tx_ds 	= (status >> TX_DS) & 1;
	retval->tx_full = (status >> TX_FULL) & 1;
	retval->rx_p_no = (status >> RX_P_NO) & 0b111;

	return retval;
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

void info_pipe(rscs_nrf24l01_pipe_config_t* retval){
	printf("\n#########   NRF24L01 PIPE CONFIG DUMP   #########\n");
	printf("num %d\n",retval->num);
	printf("en %d\n",retval->en);
	printf("en_aa %d\n",retval->en_aa);
	printf("en_dpl %d\n",retval->en_dpl);
	printf("pw %d\n",retval->pw);
	printf("######################################\n");
}

void info_nrf(rscs_nrf24l01_config_t* retval){
	printf("\n#########   NRF24L01 CONFIG DUMP   #########\n");
	printf("CONFIG\n");
	printf("	rx_dr %d \n", retval->config.rx_dr);
	printf("	tx_ds %d \n",retval->config.tx_ds);
	printf("	max_rt %d \n",retval->config.max_rt);
	printf("	en_crc %d \n",retval->config.en_crc);
	printf("	crc0 %d \n",retval->config.crc0);
	printf("	pwr_up %d \n",retval->config.pwr_up);
	printf("	prim_rx %d \n",retval->config.prim_rx);

	printf("--------------------------------\nSETUP_AW\n");
	printf("	aw %d \n",retval->setup_aw.aw);

	printf("--------------------------------\nSETUP_RETR\n");
	printf("	arc %d \n",retval->setup_retr.arc);
	printf("	ard %d \n",retval->setup_retr.ard);

	printf("--------------------------------\nRF_CH\n");
	printf("	rf_ch %d \n",retval->rf_ch.rf_ch);

	printf("--------------------------------\nRF_SETUP\n");
	printf("	pll_lock %d \n",retval->rf_setup.pll_lock);
	printf("	rf_dr %d \n",retval->rf_setup.rf_dr);
	printf("	rf_pwr %d \n",retval->rf_setup.rf_pwr);
	printf("	lna_hcurr %d \n",retval->rf_setup.lna_hcurr);

	printf("--------------------------------\nFEATURE\n");
	printf("	en_dpl %d \n",retval->feature.en_dpl);
	printf("	en_ack_pay %d \n",retval->feature.en_ack_pay);
	printf("	en_dyn_ack %d \n",retval->feature.en_dyn_ack);
	printf("######################################\n");
}

void info_st(rscs_nrf24l01_status_t* retval){
	printf("\n#########   NRF24L01 CONFIG DUMP  #########\n");
	printf("max_rt %d\n", retval->max_rt);
	printf("rx_dr %d\n", retval->rx_dr);
	printf("tx_ds %d\n", retval->tx_ds);
	printf("tx_full %d\n", retval->tx_full);
	printf("rx_p_no %d\n", retval->rx_p_no);
	printf("######################################\n");
}

#include <string.h>

uint8_t test(rscs_nrf24l01_bus_t * nrf1){
	rscs_nrf24l01_config_t * set = rscs_nrf24l01_get_config(nrf1);
	set->config.crc0 = 0;
	set->config.en_crc = 1;
	set->config.max_rt = 0;
	set->config.rx_dr = 0;
	set->config.tx_ds = 0;
	set->config.pwr_up = 1;
	set->config.prim_rx = 1;

	set->feature.en_ack_pay = 1;
	set->feature.en_dpl = 1;
	set->feature.en_dyn_ack = 1;

	set->rf_ch.rf_ch = 2;

	set->rf_setup.pll_lock = 0;
	set->rf_setup.rf_dr = 1;
	set->rf_setup.rf_pwr = 3;
	set->rf_setup.lna_hcurr = 1;

	set->setup_aw.aw = 3;

	set->setup_retr.arc = 0;
	set->setup_retr.ard = 0;

	set->tx.addr = 0x1122334455;

	info_nrf(set);
	rscs_nrf24l01_set_config(set, nrf1);
	info_nrf(rscs_nrf24l01_get_config(nrf1));

	rscs_nrf24l01_pipe_config_t* pipe = rscs_nrf24l01_get_pipe_config(0, nrf1);
	pipe->en = 1;
	pipe->en_aa = 1;
	pipe->en_dpl = 1;
	pipe->pw = 0;
	pipe->rx_addr = 0x1122334455;

	info_pipe(pipe);
	rscs_nrf24l01_set_pipe_config(pipe, nrf1);
	info_pipe(rscs_nrf24l01_get_pipe_config(0, nrf1));

	info_st(rscs_nrf24l01_get_status(nrf1));

	char f[] = "flash";
	char c[] = "config";
	char p[] = "pipe";
	char s[] = "status";
	char dr[] = "data_read";
	char dw[] = "data_write";

	while(1){
		printf("Command\n");
		char data[256];
		scanf("%s", data);
		if(!strcmp(data, c)) info_nrf(rscs_nrf24l01_get_config(nrf1));
		if(!strcmp(data, s)) info_st(rscs_nrf24l01_get_status(nrf1));
		if(!strcmp(data, p)){
			int num;
			scanf("%d", &num);
			info_pipe(rscs_nrf24l01_get_pipe_config(num, nrf1));
		}
		if(!strcmp(data, f)){
			_command(FL_RX, nrf1);
			_command(FL_TX, nrf1);
			_wreg(STATUS, _rreg(STATUS, nrf1), nrf1);
		}
		if(!strcmp(data, dr)){
			char get[33];
			uint8_t count = rscs_nrf24l01_read(nrf1, get);
			printf("Readed: %d\n", count);
			if(count){
				get[count - 1] = 0;
				printf("%s\n", get);
			}
		}
		if(!strcmp(data, dw)){
			char get[32];
			scanf("%s", get);
			int size = strnlen(get, 32);
			rscs_nrf24l01_write(nrf1, get, size);
			printf("Writed[%d]: %s\n", size, get);
		}
	}

	return 0;
}











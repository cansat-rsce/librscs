#ifndef RSCS_NRF24L01_H_
#define RSCS_NRF24L01_H_

#include <stdint.h>
#include "uart.h"

typedef struct rscs_nrf24l01_bus_t rscs_nrf24l01_bus_t;

typedef struct{
	struct{
		uint8_t rx_dr:1,
				tx_ds:1,
				max_rt:1,
				en_crc:1,
				crc0:1,
				pwr_up:1,
				prim_rx:1;
	} config;

	struct{
		uint8_t aw:2;
	} setup_aw;

	struct{
		uint8_t ard:4,
				arc:4;
	} setup_retr;

	struct{
		uint8_t rf_ch:7;
	} rf_ch;

	struct{
		uint8_t pll_lock:1,
				rf_dr:1,
				rf_pwr:2,
				lna_hcurr:1;
	} rf_setup;

	struct{
		uint8_t en_dpl:1,
				en_ack_pay:1,
				en_dyn_ack:1;
	} feature;

	struct{
		uint64_t addr:40;
	} tx;
} rscs_nrf24l01_config_t;


typedef struct{
	uint64_t rx_addr:40,
			 pw:6,
			 en_aa:1,
			 en_dpl:1,
			 en:1,
			 num:3;
} rscs_nrf24l01_pipe_config_t;

typedef struct{
	uint8_t rx_dr:1,
			tx_ds:1,
			max_rt:1,
			rx_p_no:3,
			tx_full:1;
} rscs_nrf24l01_status_t;

uint8_t rscs_nrf24l01_write(rscs_nrf24l01_bus_t * bus, void* data, size_t size);

uint8_t rscs_nrf24l01_read(rscs_nrf24l01_bus_t * bus, void* data);

void rscs_nrf24l01_get_config(rscs_nrf24l01_config_t* retval, rscs_nrf24l01_bus_t * bus);

void rscs_nrf24l01_set_config(rscs_nrf24l01_config_t set, rscs_nrf24l01_bus_t * bus);

void rscs_nrf24l01_get_pipe_config(rscs_nrf24l01_pipe_config_t* retval, rscs_nrf24l01_bus_t * bus);

void rscs_nrf24l01_set_pipe_config(rscs_nrf24l01_pipe_config_t set, rscs_nrf24l01_bus_t * bus);

void rscs_nrf24l01_get_status(rscs_nrf24l01_status_t* retval, rscs_nrf24l01_bus_t * bus);

rscs_nrf24l01_bus_t * rscs_nrf24l01_init(uint8_t (*exchange)(uint8_t byte),
											volatile uint8_t * CSPORT, uint8_t cspin,
											volatile uint8_t * CEPORT, uint8_t cepin);

uint8_t test(rscs_nrf24l01_bus_t * nrf1);

#endif

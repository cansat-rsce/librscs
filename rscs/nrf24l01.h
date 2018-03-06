#ifndef RSCS_NRF24L01_H_
#define RSCS_NRF24L01_H_

#include <stdint.h>

typedef struct rscs_nrf24l01_bus_t rscs_nrf24l01_bus_t;

enum{
	RSCS_NRF24L01_PTX = 0,
	RSCS_NRF24L01_PRX = 1,

	RSCS_NRF24L01_CRC_1B = 0,
	RSCS_NRF24L01_CRC_2B = (1 << 2),

	RSCS_NRF24L01_DIS_CRC = 0,
	RSCS_NRF24L01_EN_CRC = (1 << 3),

	RSCS_NRF24L01_REF_TX_DS_INT = 0,
	RSCS_NRF24L01_NREF_TX_DS_INT = (1 << 5),

	RSCS_NRF24L01_REF_RX_DS_INT = 0,
	RSCS_NRF24L01_NREF_RX_DS_INT = (1 << 6),

	RSCS_NRF24L01_REF_MAX_RT_INT = 0,
	RSCS_NRF24L01_NREF_MAX_RT_INT = (1 << 5),
}; //configure

enum{
	RSCS_NRF24L01_DIS_DPL = 0,
	RSCS_NRF24L01_EN_DPL = (1 << 2),

	RSCS_NRF24L01_DIS_ACK_PAY = 0,
	RSCS_NRF24L01_EN_ACK_PAY = (1 << 1),

	RSCS_NRF24L01_DIS_DYN_ACK = 0,
	RSCS_NRF24L01_EN_DYN_ACK = 1,
}; //feature

rscs_nrf24l01_bus_t * rscs_nrf24l01_init(uint8_t (*exchange)(uint8_t byte),
											volatile uint8_t * CSPORT, uint8_t cspin,
											volatile uint8_t * CEPORT, uint8_t cepin);

uint8_t test(rscs_nrf24l01_bus_t * nrf);

#endif

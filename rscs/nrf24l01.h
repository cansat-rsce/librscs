#ifndef RSCS_NRF24L01_H_
#define RSCS_NRF24L01_H_

typedef struct rscs_nrf24l01_bus_t rscs_nrf24l01_bus_t;

typedef enum{
	RSCS_NRF24L01_MODE_PTX = 1,
	RSCS_NRF24L01_MODE_PRX = 0
} rscs_nrf24l01_mode_t;

rscs_nrf24l01_bus_t * rscs_nrf24l01_init(uint8_t (*exchange)(uint8_t byte),
											volatile uint8_t * CSPORT, uint8_t cspin,
											volatile uint8_t * CEPORT, uint8_t cepin);

uint8_t test(rscs_nrf24l01_bus_t * PTX, rscs_nrf24l01_bus_t * PRX);

#endif

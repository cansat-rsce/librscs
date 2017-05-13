/*
 * tsl2561.h
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */

#ifndef TSL2561_H_
#define TSL2561_H_

#include "error.h"
#include <stdint.h>

struct rscs_tsl2561_t;
typedef struct rscs_tsl2561_t rscs_tsl2561_t;

typedef enum {
	RSCS_TSL2561_ADDR_LOW = 0b00101001,
	RSCS_TSL2561_ADDR_FLOATING = 0b00111001,
	RSCS_TSL2561_ADDR_HIGH = 0b01001001
} rscs_tsl2561_addr_t;

typedef enum {
	RSCS_TSL2561_GAIN_1X = 0,
	RSCS_TSL2561_GAIN_16X = 1
} rscs_tsl2561_gain_t;

typedef enum {
	RSCS_TSL2561_TYPE_T = 0,
	RSCS_TSL2561_TYPE_CS = 1
} rscs_tsl2561_type_t;

typedef enum {
	RSCS_TSL2561_INT_13MS = 0,
	RSCS_TSL2561_INT_100MS = 1,
	RSCS_TSL2561_INT_402MS = 2,
	RSCS_TSL2561_INT_MANUAL = 3
} rscs_tsl2561_int_t;

// Инициализация tsl2561
rscs_tsl2561_t * rscs_tsl2561_init(rscs_tsl2561_addr_t addr);

// первичная настройка и запуск измерений tsl2561
rscs_e rscs_tsl2561_setup(rscs_tsl2561_t * self);

// Деинициализация
void rscs_tsl2561_deinit(rscs_tsl2561_t * self);

// Получение данных с tsl2561
rscs_e rscs_tsl2561_read(rscs_tsl2561_t * self, uint16_t * sensor_data0, uint16_t * sensor_data1);

// Формирует полноценные люксы из отдельных показаний фотодиодов
uint16_t rscs_tsl2561_get_lux(rscs_tsl2561_t * self, rscs_tsl2561_gain_t iGain, rscs_tsl2561_type_t iType, rscs_tsl2561_int_t tInt, unsigned int sensor_data0, unsigned int sensor_data1);

#endif /* TSL2561_H_ */

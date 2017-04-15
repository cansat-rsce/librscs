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

// Инициализация tsl2561
rscs_e rscs_tsl2561_init(void);

// Получение данных с tsl2561
rscs_e rscs_tsl2561_read(uint16_t * sensor_data0, uint16_t * sensor_data1);

// Формирует полноценные люксы из отдельных показаний фотодиодов
uint16_t rscs_tsl2561_get_lux(unsigned int iGain, unsigned int tInt, unsigned int sensor_data0, unsigned int sensor_data1, int iType);

#endif /* TSL2561_H_ */

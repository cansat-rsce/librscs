/*
 * tsl2561.h
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */

#ifndef TSL2561_H_
#define TSL2561_H_

#include "error.h"

// Инициализация tsl2561
rscs_e rscs_tsl2561_init(void);

// Получение данных с tsl2561
uint16_t rscs_tsl2561_read(uint8_t *PD_ARRAY[4]);

#endif /* TSL2561_H_ */

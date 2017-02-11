/*
 * tsl2561.h
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */

#ifndef TSL2561_H_
#define TSL2561_H_

// Инициализация tsl2561
void rscs_tsl2561_init(void);

// Получение данных с tsl2561
int rscs_tsl2561_read(uint16_t luminosity);

#endif /* TSL2561_H_ */

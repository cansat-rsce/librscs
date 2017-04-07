/*
 * ina219.h
 *
 *  Created on: 18 марта 2017 г.
 *      Author: developer
 */

#ifndef INA219_H_
#define INA219_H_

#include <stdint.h>

#include "error.h"

struct rscs_ina219_t;
typedef struct rscs_ina219_t rscs_ina219_t;

typedef enum RSCS_INA219_MODE
{
	RSCS_INA219_CHANNEL_BUS,
	RSCS_INA219_CHANNEL_SHUNT,
	RSCS_INA219_CHANNEL_POWER,

} rscs_ina219_channel_t;


rscs_ina219_t * rscs_ina219_init(uint8_t i2c_addr);

void rscs_ina219_deinit(rscs_ina219_t * device);

rscs_e rscs_ina219_start_single(rscs_ina219_t * device, rscs_ina219_channel_t mode);

rscs_e rscs_ina219_start_continuous(rscs_ina219_t * device, rscs_ina219_channel_t mode);

rscs_e rscs_ina219_read(rscs_ina219_t * device, rscs_ina219_channel_t channel, uint16_t * rawvalue);

rscs_e rscs_ina219_wait_single (rscs_ina219_t * device);

rscs_e rscs_ina216_set_cal(rscs_ina219_t * device, uint8_t maxExpCur, uint8_t Rshunt);





#endif /* INA219_H_ */

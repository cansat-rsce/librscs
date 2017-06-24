/*
 * ina219.h
 *
 *  Created on: 18 марта 2017 г.
 *      Author: developer
 */

#ifndef INA219_H_
#define INA219_H_

#define INA_RST		15
#define INA_BRNG	13
#define INA_PG1		12
#define INA_PG0		11
#define INA_BADC4	10
#define INA_BADC3	9
#define INA_BADC2	8
#define INA_BADC1	7
#define INA_SADC4	6
#define INA_SADC3	5
#define INA_SADC2	4
#define INA_SADC1	3
#define INA_MODE3	2
#define INA_MODE2	1
#define INA_MODE1	0

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

rscs_e rscs_ina219_wait_single(rscs_ina219_t * device);

rscs_e rscs_ina219_set_cal(rscs_ina219_t * device, uint16_t calreg);

rscs_e rscs_ina219_set_cfg(rscs_ina219_t * device, uint16_t cfgreg);

//rscs_e rscs_ina_read_reg(rscs_ina219_t * device, uint8_t reg_addr, uint16_t * reg_value);





#endif /* INA219_H_ */

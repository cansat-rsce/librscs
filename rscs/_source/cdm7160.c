/*
 * cdm7160.c
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */
#include <stdlib.h>
#include <inttypes.h>
#include "stddef.h"

#include "../error.h"
#include "../i2c.h"

#include "../cdm7160.h"

#define RST		0x00
	#define REST	0

#define CTL		0x01
	#define CTL0	0
	#define CTL1	1
	#define CTL2 	2

#define ST1		0x02
	#define BUSY	7

#define DAL		0x03
#define DAH		0x04
#define HPA		0x09
#define HIT		0x0A

#define op(OP) error = OP; if(error != RSCS_E_NONE) goto end;
#define ch(OP) error = OP; if(error != RSCS_E_NONE) return error;

struct rscs_cdm7160_t {
	uint8_t addr;
}; // Объявление типа структуры для хранения дескриптора устройства


static rscs_e _wreg(rscs_cdm7160_t* sensor, uint8_t reg, uint8_t data){
	rscs_e error = RSCS_E_NONE;

	op(rscs_i2c_start());
	op(rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write));
	op(rscs_i2c_write_byte(reg));
	op(rscs_i2c_write_byte(data));

end:
	rscs_i2c_stop();
	return error;
}

static rscs_e _rreg(rscs_cdm7160_t* sensor, uint8_t reg, void* data, size_t datasize){
	uint8_t* buf = (uint8_t*)data;
	rscs_e error = RSCS_E_NONE;

	op(rscs_i2c_start());
	op(rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write));
	op(rscs_i2c_write_byte(reg));

	op(rscs_i2c_start());
	op(rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_read));
	op(rscs_i2c_read(buf, datasize, 1));

end:
	rscs_i2c_stop();
	return error;
}

rscs_cdm7160_t * rscs_cdm7160_init(rscs_cdm7160_address_t addr){
	rscs_cdm7160_t* sensor = (rscs_cdm7160_t*)malloc(sizeof(rscs_cdm7160_t));
	if(sensor == NULL) return NULL;

	if(addr == RSCS_CDM7160_ADDR_LOW) sensor->addr = 0b1101000;
	else if(addr == RSCS_CDM7160_ADDR_HIGH) sensor->addr = 0b1101001;
	else abort();

	return sensor;
}

rscs_e rscs_cdm7160_reset(rscs_cdm7160_t* sensor)
{
	return _wreg(sensor, RST, 1 << REST);
};

rscs_e rscs_cdm7160_mode_set(rscs_cdm7160_t* sensor, rscs_cdm7160_mode_t mode)
{
	if(mode == RSCS_CDM7160_MODE_CONTINUOUS)
		return _wreg(sensor, CTL, (1 << CTL2) | (1 << CTL1));
	else if(mode == RSCS_CDM7160_MODE_SLEEP)
		return _wreg(sensor, CTL, 0);
	return RSCS_E_INVARG;
};

rscs_e rscs_cdm7160_read(rscs_cdm7160_t* sensor, uint16_t* CO2_raw_conc)
{
	rscs_e error = RSCS_E_NONE;

	uint8_t flag;
	ch(_rreg(sensor, ST1, &flag, 1));

	if(((~flag) >> BUSY) & 1)
		return _rreg(sensor, DAL, CO2_raw_conc, 2);

	return RSCS_E_BUSY;
};

rscs_e rscs_cdm7160_write_pressure_corr(rscs_cdm7160_t* sensor, uint8_t press_coeff)
{
	return _wreg(sensor, HPA, press_coeff);
};

rscs_e rscs_cdm7160_write_altitude_corr(rscs_cdm7160_t* sensor, uint8_t alt_coeff)
{
	return _wreg(sensor, HIT, alt_coeff);
};

void rscs_cdm7160_deinit(rscs_cdm7160_t* sensor){
	free(sensor);
}

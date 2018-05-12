/*
 * cdm7160.c
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */
#include <stdlib.h>
#include <inttypes.h>
#include "stddef.h"

#include "error.h"
#include "i2c.h"

#include "cdm7160.h"

#define RST		0x00
	#define REST	0

#define CTL		0x01
	#define CTL0	0
	#define CTL1	1
	#define CTL2 	2

#define op(OP) error = OP; if(error != RSCS_E_NONE) goto end;

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
	if(NULL == sensor) return sensor;

	if(addr == RSCS_CDM7160_ADDR_LOW) sensor->addr = 0b11010000;
	else if(addr == RSCS_CDM7160_ADDR_HIGH) sensor->addr = 0b11010010;

	return sensor;
}

rscs_e rscs_cdm7160_reset(rscs_cdm7160_t* sensor)
{
	return _wreg(sensor, RST, 1 << REST);
};

rscs_e rscs_cdm7160_mode_set(rscs_cdm7160_t* sensor, rscs_cdm7160_mode_t mode)
{
	if(mode == RSCS_CDM7160_MODE_CONTINUOUS)
		return _wreg(sensor, CTL, (1 << CTL2) | (1 << CTL2));
	else if(mode == RSCS_CDM7160_MODE_SLEEP)
		return _wreg(sensor, CTL, 0);
	return RSCS_E_INVARG;
};

rscs_e rscs_cdm7160_read_CO2(rscs_cdm7160_sensor_t* sensor, uint8_t* data)
{
	error = rscs_i2c_start();
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_read);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(0x03);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_read(void * data_ptr, 2, true);
	if(error != RSCS_E_NONE) goto end;

	end:
				rscs_i2c_stop();
				return error;
};

rscs_e rscs_cdm7160_write_pressure_corr(rscs_cdm7160_sensor_t* sensor, int pressure)
{
	error = rscs_i2c_start();
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(0x09);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(pressure);
	if(error != RSCS_E_NONE) goto end;

		end:
				rscs_i2c_stop();
				return error;
};

rscs_e rscs_cdm7160_write_altitude_corr(rscs_cdm7160_sensor_t* sensor, int altitude)
{
	error = rscs_i2c_start();
		if(error != RSCS_E_NONE) goto end;
		error = rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write);
		if(error != RSCS_E_NONE) goto end;
		error = rscs_i2c_write_byte(0x0A);
		if(error != RSCS_E_NONE) goto end;
		error = rscs_i2c_write_byte(altitude);
		if(error != RSCS_E_NONE) goto end;

		end:
				rscs_i2c_stop();
				return error;
};



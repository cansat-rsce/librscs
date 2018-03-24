/*
 * cdm7160.c
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */
#include <stdlib.h>
#include "error.h"
#include "stddef.h"
#include "cdm7160.h"


rscs_cdm7160_sensor_t * rscs_cdm7160_init(addr){
	rscs_sdcard_t * self = (rscs_sdcard_t*)malloc(sizeof(rscs_sdcard_t));
	rscs_cdm7160_sensor_t* sensor = (rscs_cdm7160_sensor_t*)malloc(sizeof(rscs_cdm7160_sensor_t));
	if(NULL == sensor) return sensor;

	if(addr == LOW) sensor->addr = 0b1101000;
	if(addr == HIGH) sensor->addr = 0b1101001;

	return sensor;
}


rscs_e rscs_cdm7160_reset(rscs_cdm7160_sensor_t* sensor)
{
	error = rscs_i2c_start();
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(0x00);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(0x01);
	if(error != RSCS_E_NONE) goto end;

	end:
				rscs_i2c_stop();
				return error;
};

rscs_e rscs_cdm7160_mode_set(rscs_cdm7160_sensor_t* sensor, rscs_cdm7160_mode_t* mode)
{
	error = rscs_i2c_start();
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_send_slaw(sensor->addr, rscs_i2c_slaw_write);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(0x01);
	if(error != RSCS_E_NONE) goto end;
	error = rscs_i2c_write_byte(mode);
	if(error != RSCS_E_NONE) goto end;

	end:
			rscs_i2c_stop();
			return error;
};

rscs_e rscs_cdm7160_read_CO2(rscs_cdm7160_sensor_t* sensor)
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



/*
 * ina219.c
 *
 *  Created on: 18 марта 2017 г.
 *      Author: developer
 */
#include <stdlib.h>
#include <stdbool.h>

#include "../ina219.h"
#include "../i2c.h"

struct rscs_ina219_t
{
	uint8_t address;
	bool con_rdy;
};


static rscs_e _write_reg(rscs_ina219_t * device, uint8_t reg_addr, uint16_t reg_value)
{
	rscs_e error;

	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	error = rscs_i2c_send_slaw(device->address, rscs_i2c_slaw_write);
	if (error != RSCS_E_NONE)
		goto end;

	error = rscs_i2c_write_byte(reg_addr);
	if (error != RSCS_E_NONE)
		goto end;

	error = rscs_i2c_write_byte(reg_value >> 8);
	if (error != RSCS_E_NONE)
		goto end;

	error = rscs_i2c_write_byte(reg_value & 0x00FF);
	if (error != RSCS_E_NONE)
		goto end;
end:
	rscs_i2c_stop();
	return error;
}


static rscs_e _read_reg(rscs_ina219_t * device, uint8_t reg_addr, uint16_t * reg_value)
{
	rscs_e error;

	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	error = rscs_i2c_send_slaw(device->address, rscs_i2c_slaw_write);
	if (error != RSCS_E_NONE)
		goto end;

	error = rscs_i2c_write_byte(reg_addr);
	if (error != RSCS_E_NONE)
		goto end;

	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	error = rscs_i2c_send_slaw(device->address, rscs_i2c_slaw_read);
	if (error != RSCS_E_NONE)
		goto end;

	uint8_t data[2];

	error = rscs_i2c_read(data, 2, 1);
	if (error != RSCS_E_NONE)
		goto end;

	*reg_value = ((uint16_t)data[0] << 8) | data[1];

end:
	rscs_i2c_stop();
	return error;

}


rscs_ina219_t * rscs_ina219_init(uint8_t i2c_addr)
{
	rscs_ina219_t * retval = (rscs_ina219_t *)malloc(sizeof(rscs_ina219_t));
	if (!retval)
		return retval;

	retval->address = i2c_addr;
	return retval;
}


void rscs_ina219_deinit(rscs_ina219_t * device)
{
	free(device);
}


rscs_e rscs_ina219_start_single(rscs_ina219_t * device, rscs_ina219_channel_t mode)
{
	rscs_i2c_start();

	rscs_e error;

	switch (mode)
	{
	case RSCS_INA219_CHANNEL_SHUNT:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg |= 1;
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
			return error;
	}break;

	case RSCS_INA219_CHANNEL_BUS:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg |= 2;
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
					return error;
	}break;

	case RSCS_INA219_CHANNEL_POWER:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg |= 3;
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
					return error;
	}break;
	}

	rscs_i2c_stop();
	return RSCS_E_NONE;

}

rscs_e rscs_ina219_start_continuous(rscs_ina219_t * device, rscs_ina219_channel_t mode)
{
	rscs_i2c_start();

	rscs_e error;

		switch (mode)
		{
		case RSCS_INA219_CHANNEL_SHUNT:
		{
			uint16_t config_reg;
			error =_read_reg(device, 0x00, &config_reg);
			if (error != RSCS_E_NONE)
						return error;
			config_reg |= 5;
			error =_write_reg(device, 0x00, config_reg);
			if (error != RSCS_E_NONE)
						return error;
		}break;

		case RSCS_INA219_CHANNEL_BUS:
		{
			uint16_t config_reg;
			error =_read_reg(device, 0x00, &config_reg);
			if (error != RSCS_E_NONE)
						return error;
			config_reg |= 6;
			error =_write_reg(device, 0x00, config_reg);
			if (error != RSCS_E_NONE)
						return error;
		}break;

		case RSCS_INA219_CHANNEL_POWER:
		{
			uint16_t config_reg;
			error =_read_reg(device, 0x00, &config_reg);
			if (error != RSCS_E_NONE)
						return error;
			config_reg |= 9;
			error =_write_reg(device, 0x00, config_reg);
			if (error != RSCS_E_NONE)
						return error;

		}break;
		}

		rscs_i2c_stop();
		return RSCS_E_NONE;

}


rscs_e rscs_ina219_read(rscs_ina219_t * device, rscs_ina219_channel_t channel, uint16_t * rawvalue)
{
	rscs_i2c_start();

	rscs_e error;

	switch(channel)
	{
	case RSCS_INA219_CHANNEL_SHUNT:
	{
		error =_read_reg(device, 0x01, rawvalue);
		if (error != RSCS_E_NONE)
					return error;
	}break;

	case RSCS_INA219_CHANNEL_BUS:
	{
		_read_reg(device, 0x02, rawvalue);
		if (error != RSCS_E_NONE)
					return error;
	}break;

	case RSCS_INA219_CHANNEL_POWER:
	{
		_read_reg(device, 0x03, rawvalue);
		if (error != RSCS_E_NONE)
					return error;
	}break;
	}

	rscs_i2c_stop();
	return RSCS_E_NONE;
}

rscs_e rscs_rscs_ina219_wait_single (rscs_ina219_t * device)
{
	rscs_i2c_start();

	rscs_e error;
	uint16_t con_rdy;

	error = _read_reg (device, 0x02, &con_rdy);
		if(error != RSCS_E_NONE)
			return error;

	con_rdy &= 2;

	if(con_rdy == 2){
		device->con_rdy = true;
	}else {
		device->con_rdy = false;
	}

	rscs_i2c_stop();
	return RSCS_E_NONE;
}



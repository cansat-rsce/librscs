/*
 * ina219.c
 *
 *  Created on: 18 марта 2017 г.
 *      Author: developer
 */
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>

#include <stdio.h>

#include  "librscs_config.h"
#include "../ina219.h"
#include "../i2c.h"

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



struct rscs_ina219_t
{
	uint8_t address;
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

	error = rscs_i2c_write_byte(reg_value & 0xFF);
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
		return NULL;

	retval->address = i2c_addr;
	uint16_t temp = 0;
	temp |= (1<<INA_BADC2) | (1<<INA_BADC1) | (1<<INA_SADC2) | (1<<INA_SADC1);
	_write_reg(retval, 0x00, temp);

	return retval;
}


void rscs_ina219_deinit(rscs_ina219_t * device)
{
	free(device);
}


rscs_e rscs_ina219_start_single(rscs_ina219_t * device, rscs_ina219_channel_t mode)
{
	rscs_e error;

	switch (mode)
	{
	case RSCS_INA219_CHANNEL_SHUNT:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg &= ~((1<<INA_MODE1) | (1<<INA_MODE2) | (1<<INA_MODE3));
		config_reg |= (1<<INA_MODE1) | (0<<INA_MODE2) | (0<<INA_MODE3);
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
			return error;
	}break;

	case RSCS_INA219_CHANNEL_BUS:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg &= ~((1<<INA_MODE1) | (1<<INA_MODE2) | (1<<INA_MODE3));
		config_reg |= (0<<INA_MODE1) | (1<<INA_MODE2) | (0<<INA_MODE3);
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
					return error;
	}break;

	case RSCS_INA219_CHANNEL_POWER:
	{
		uint16_t config_reg;
		error =_read_reg(device, 0x00, &config_reg);
		config_reg &= ~((1<<INA_MODE1) | (1<<INA_MODE2) | (0<<INA_MODE3));
		config_reg |= (1<<INA_MODE1) | (1<<INA_MODE2) | (0<<INA_MODE3);
		error =_write_reg(device, 0x00, config_reg);
		if (error != RSCS_E_NONE)
					return error;
	}break;
	}

	return RSCS_E_NONE;

}

rscs_e rscs_ina219_start_continuous(rscs_ina219_t * device, rscs_ina219_channel_t mode)
{
	rscs_e error;

		switch (mode)
		{
		case RSCS_INA219_CHANNEL_SHUNT:
		{
			uint16_t config_reg;
			error =_read_reg(device, 0x00, &config_reg);
			if (error != RSCS_E_NONE)
						return error;
			config_reg &= ~((1<<INA_MODE1) | (0<<INA_MODE2) | (1<<INA_MODE3));
			config_reg |= (1<<INA_MODE1) | (0<<INA_MODE2) | (1<<INA_MODE3);
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
			config_reg &= ~((0<<INA_MODE1) | (1<<INA_MODE2) | (1<<INA_MODE3));
			config_reg |= (0<<INA_MODE1) | (1<<INA_MODE2) | (0<<INA_MODE3);
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
			config_reg &= ~((1<<INA_MODE1) | (1<<INA_MODE2) | (1<<INA_MODE3));
			config_reg |= (1<<INA_MODE1) | (1<<INA_MODE2) | (1<<INA_MODE3);
			error =_write_reg(device, 0x00, config_reg);
			if (error != RSCS_E_NONE)
						return error;

		}break;
		}

		return RSCS_E_NONE;

}


rscs_e rscs_ina219_read(rscs_ina219_t * device, rscs_ina219_channel_t channel, uint16_t * rawvalue)
{
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
		error = _read_reg(device, 0x02, rawvalue);
		if (error != RSCS_E_NONE)
					return error;
	}break;

	case RSCS_INA219_CHANNEL_POWER:
	{
		error = _read_reg(device, 0x03, rawvalue);
		if (error != RSCS_E_NONE)
					return error;
	}break;
	}

	return RSCS_E_NONE;
}

rscs_e rscs_ina219_wait_single(rscs_ina219_t * device)
{
	rscs_e error;
	uint16_t con_rdy;

	for (int i = 0; i < RSCS_INA219_TIMEOUT_CYCLES; i++)
	{
		error = _read_reg(device, 0x02, &con_rdy);
		if(error != RSCS_E_NONE)
			return error;

		if ((con_rdy &= 2) == 2)
			return RSCS_E_NONE;

		_delay_us(100);
	}
	return RSCS_E_TIMEOUT;
}

rscs_e rscs_ina219_set_cal(rscs_ina219_t * device, uint16_t calreg)
{
	rscs_e error;

	error = _write_reg(device, 0x05, calreg);
	if(error != RSCS_E_NONE)
		return error;

	return RSCS_E_NONE;
}


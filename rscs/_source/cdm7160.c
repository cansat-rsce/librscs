/*
 * cdm7160.c
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */

#include "error.h"

#include "cdm7160.h"

#define addr_H 0b1101001
#define addr_L 0b1101000

rscs_e rscs_cdm7160_reset(void)
{
	rscs_e error = rscs_i2c_start();
	if(error != 0) return error;
	error = rscs_i2c_send_slaw(addr_H, rscs_i2c_slaw_write);
	if(error != 0) return error;
	error = rscs_i2c_write_byte(0x00);
	if(error != 0) return error;
	error = rscs_i2c_write_byte(0x01);
	if(error != 0) return error;
	rscs_i2c_stop();
}



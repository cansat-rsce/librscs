#include "../include/rscs/i2c.h"

#include <stdbool.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "../include/rscs/error.h"

#define I2C_START_TRANSFERED 0x10
#define I2C_RESTART_TRANSFERED 0x08
#define I2C_SLAW_ACK 0x18
#define I2C_SLAW_NO_ACK 0x20
#define I2C_SLAR_ACK 0x40
#define I2C_SLAR_NO_ACK 0x48
#define I2C_READ_ACK 0x50
#define I2C_READ_NO_ACK 0x58
#define I2C_WRITE_ACK 0x28
#define I2C_WRITE_NO_ACK 0x30
#define I2C_ARB_LOST 0x38

static bool _i2c_internal_needinit = true;

inline static rscs_e i2c_status_to_error(uint8_t status)
{
	switch (status)
	{
	case I2C_ARB_LOST:
		return RSCS_E_IO;

	case I2C_SLAW_NO_ACK:
	case I2C_SLAR_NO_ACK:
	case I2C_WRITE_NO_ACK:
	case I2C_READ_NO_ACK:
		return RSCS_E_INVRESP;
	};

	return RSCS_E_INVARG;
}


void rscs_i2c_init(rscs_i2c_bus_t * bus)
{
	if(_i2c_internal_needinit){
		// настраиваем частоту на стандартные 100 кГц
		rscs_i2c_set_scl_rate(bus, 100);

		// сбрасываем модуль в исходное состояние
		rscs_i2c_reset(bus);
		_i2c_internal_needinit = false;
	}
}


void rscs_i2c_reset(rscs_i2c_bus_t * bus)
{
	TWCR &= ~(1 << TWEN);
}


void rscs_i2c_set_scl_rate(rscs_i2c_bus_t * bus, uint32_t f_scl_kHz)
{
	TWBR = (uint8_t)(F_CPU/(f_scl_kHz*1000) - 16)/2/1; // формула из даташита
}


void rscs_i2c_set_timeout(rscs_i2c_bus_t * bus, uint32_t timeout_us)
{
	bus->_timeout_us = timeout_us;
}


rscs_e rscs_i2c_start(rscs_i2c_bus_t * bus)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	for(int i = 0; i < bus->_timeout_us; i++){
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_START_TRANSFERED || status == I2C_RESTART_TRANSFERED)	{
				return RSCS_E_NONE;
			} else {
				return i2c_status_to_error(status);
			}
		}
		_delay_us(1);
	}
	return RSCS_E_TIMEOUT;
}


rscs_e rscs_i2c_stop(rscs_i2c_bus_t * bus)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	return 0;
}


rscs_e rscs_i2c_send_slaw(rscs_i2c_bus_t * bus, uint8_t slave_addr, bool read_access)
{
	TWDR = (slave_addr << 1) | read_access;
	TWCR = (1<<TWINT) | (1<<TWEN);

	for(int i = 0; i < bus->_timeout_us; i++ ) {
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_SLAW_ACK || status == I2C_SLAR_ACK) {
				return RSCS_E_NONE;
			} else {
				return i2c_status_to_error(status);
			}
		} else {
			_delay_us(1);
		}
	}

	return RSCS_E_TIMEOUT;
}


rscs_e rscs_i2c_write(rscs_i2c_bus_t * bus, const void * data_ptr, size_t data_size)
{
	const uint8_t * byte_ptr = (const uint8_t * )data_ptr;
	for(int i = 0; i < data_size; i++){
		TWDR =  byte_ptr[i];
		TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < bus->_timeout_us; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_WRITE_ACK) {
					break;
				} else {
					return i2c_status_to_error(status);
				}
			} else {
				_delay_us(1);
			}
		}

		if(timeout)	return RSCS_E_TIMEOUT;
	}

	return RSCS_E_NONE;
}


rscs_e rscs_i2c_read(rscs_i2c_bus_t * bus, void * data_ptr, size_t data_size, bool NACK_at_end)
{
	uint8_t * byte_ptr = (uint8_t * )data_ptr;

	for(int i = 0; i < data_size; i++){

		if(!(i == data_size-1))
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
		else
			TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < bus->_timeout_us; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_READ_ACK || status == I2C_READ_NO_ACK) {
					break;
				} else {
					return i2c_status_to_error(status);
				}
			} else {
				_delay_us(1);
			}
		}

		if(timeout)	return RSCS_E_TIMEOUT;
		byte_ptr[i] = TWDR;
	}

	return RSCS_E_NONE;
}


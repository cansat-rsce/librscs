#include <stdbool.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "librscs_config.h"
#include "../i2c.h"
#include "../error.h"


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

// Переменная для защиты от двойной инициализации
static bool _i2c_internal_needinit = true;

// перегонка статуса I2C модуля в код ошибки rscs
inline static rscs_e _i2c_status_to_error(uint8_t status)
{
	switch (status)
	{
	case I2C_ARB_LOST:
		return RSCS_E_IO;

	case I2C_SLAW_NO_ACK:
	case I2C_SLAR_NO_ACK:
	case I2C_WRITE_NO_ACK:
	case I2C_READ_NO_ACK:
		return RSCS_E_NODEVICE;
	};

	return RSCS_E_INVARG;
}


void rscs_i2c_init(void)
{
	if(_i2c_internal_needinit){
		// настраиваем частоту на стандартные 100 кГц
		rscs_i2c_set_scl_rate(100);

		// сбрасываем модуль в исходное состояние
		_i2c_internal_needinit = false;
	}
}


void rscs_i2c_set_scl_rate(uint32_t f_scl_kHz)
{
	TWBR = (uint8_t)(F_CPU/(f_scl_kHz*1000) - 16)/2/1; // формула из даташита
}


rscs_e rscs_i2c_start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	for(int i = 0; i < RSCS_I2C_TIMEOUT_CYCLES; i++){
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_START_TRANSFERED || status == I2C_RESTART_TRANSFERED)	{
				return RSCS_E_NONE;
			} else {
				return _i2c_status_to_error(status);
			}
		}
		_delay_us(RSCS_I2C_TIMEOUT_US);
	}
	return RSCS_E_TIMEOUT;
}


void rscs_i2c_stop(void)
{
	TWCR = (1<<TWINT)|(0<<TWEN)|(1<<TWSTO);
}


rscs_e rscs_i2c_send_slaw(uint8_t slave_addr, rscs_i2c_slaw_mode_t mode)
{
	TWDR = (slave_addr << 1) | (mode == rscs_i2c_slaw_read);
	TWCR = (1<<TWINT) | (1<<TWEN);

	for(int i = 0; i < RSCS_I2C_TIMEOUT_CYCLES; i++ ) {
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_SLAW_ACK || status == I2C_SLAR_ACK) {
				return RSCS_E_NONE;
			} else {
				return _i2c_status_to_error(status);
			}
		} else {
			_delay_us(RSCS_I2C_TIMEOUT_US);
		}
	}

	return RSCS_E_TIMEOUT;
}


rscs_e rscs_i2c_write_byte(uint8_t byte)
{
	return rscs_i2c_write(&byte, 1);
}


rscs_e rscs_i2c_write(const void * data_ptr, size_t data_size)
{
	const uint8_t * byte_ptr = (const uint8_t * )data_ptr;
	for(int i = 0; i < data_size; i++){
		TWDR =  byte_ptr[i];
		TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < RSCS_I2C_TIMEOUT_CYCLES; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_WRITE_ACK) {
					break;
				} else {
					return _i2c_status_to_error(status);
				}
			} else {
				_delay_us(RSCS_I2C_TIMEOUT_US);
			}
		}

		if(timeout)	return RSCS_E_TIMEOUT;
	}

	return RSCS_E_NONE;
}


rscs_e rscs_i2c_read(void * data_ptr, size_t data_size, bool NACK_at_end)
{
	uint8_t * byte_ptr = (uint8_t * )data_ptr;

	for(int i = 0; i < data_size; i++){

		if(!(i == data_size-1))
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
		else
			TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < RSCS_I2C_TIMEOUT_CYCLES; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_READ_ACK || status == I2C_READ_NO_ACK) {
					break;
				} else {
					return _i2c_status_to_error(status);
				}
			} else {
				_delay_us(RSCS_I2C_TIMEOUT_US);
			}
		}

		if(timeout)	return RSCS_E_TIMEOUT;
		byte_ptr[i] = TWDR;
	}

	return RSCS_E_NONE;
}


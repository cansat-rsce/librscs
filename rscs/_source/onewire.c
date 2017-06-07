#include <stdlib.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "librscs_config.h"

#include "../onewire.h"


// Статики
// ========================================================================

// Установка нуля на OW шине
inline static void _ow_set_bus_zero(void)
{
	RSCS_ONEWIRE_REG_DDR |= RSCS_ONEWIRE_PIN_MASK;
}

// Установка единицы на OW шине
inline static void _ow_set_bus_one(void)
{
	RSCS_ONEWIRE_REG_DDR &= ~RSCS_ONEWIRE_PIN_MASK;
}

// Анализ текущего значения OW шины
inline static bool _ow_get_bus_value(void)
{
	return (RSCS_ONEWIRE_REG_PIN & RSCS_ONEWIRE_PIN_MASK) != 0;
}


// ========================================================================

void rscs_ow_init_bus(void)
{
	RSCS_ONEWIRE_REG_PORT &= RSCS_ONEWIRE_PIN_MASK;
	RSCS_ONEWIRE_REG_DDR &= RSCS_ONEWIRE_PIN_MASK;
}


bool rscs_ow_reset(void)
{
	_ow_set_bus_zero();
	_delay_us(600);
	_ow_set_bus_one();
	_delay_us(10);

	bool isSomeoneHere = 0;
	for (int i = 0; i < 480; i++)
	{
		if (_ow_get_bus_value() == false)
			isSomeoneHere = true;

		_delay_us(1);
	}

	return isSomeoneHere;
}


void rscs_ow_write_bit(bool value)
{
#ifdef RSCS_ONEWIRE_ATOMIC
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
	{
		_ow_set_bus_zero();
		_delay_us(2);
		if (value != 0)
		{
			_ow_set_bus_one();
		}

		_delay_us(60);
		_ow_set_bus_one();
	}
}


bool rscs_ow_read_bit(void)
{
	bool x;
#ifdef RSCS_ONEWIRE_ATOMIC
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
	{
		_ow_set_bus_zero();
		_delay_us (2);
		_ow_set_bus_one();
		_delay_us (20);
		x = _ow_get_bus_value();
		_delay_us(30);
	}

	return x;
}


void rscs_ow_write(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t bit = (1<<i) & byte;
		rscs_ow_write_bit(bit);
	}
}


uint8_t rscs_ow_read(void)
{
	uint8_t retval = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t bit = rscs_ow_read_bit();
		retval = retval | (bit<<i);
	}
	return retval;
}


void rscs_ow_write_n(const void * buffer, size_t buffersize)
{
	const uint8_t * bytes_ptr = (const uint8_t*)buffer;
	for (size_t i = 0; i < buffersize; i++)
		rscs_ow_write(bytes_ptr[i]);
}


void rscs_ow_read_n(void * buffer, size_t buffersize)
{
	uint8_t * bytes_ptr = (uint8_t*)buffer;
	for (size_t i = 0; i < buffersize; i++)
		bytes_ptr[i] = rscs_ow_read();
}


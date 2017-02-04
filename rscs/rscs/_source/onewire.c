#include "../onewire.h"

#include <util/delay.h>


// Статики
// ========================================================================

// Установка нуля на OW шине
inline static void ow_set_bus_zero(rscs_ow_bus_t * bus)
{
	*bus->ddr_reg |= bus->pin_mask;
}

// Установка единицы на OW шине
inline static void ow_set_bus_one(rscs_ow_bus_t * bus)
{
	*bus->ddr_reg &= ~bus->pin_mask;
}

// Анализ текущего значения OW шины
inline static bool ow_get_bus_value(rscs_ow_bus_t * bus)
{
	if ((*bus->pin_reg & bus->pin_mask) != 0)
		return 1;
	else
		return 0;
}

// Передача бита по OW шине
inline static void ow_write_bit(rscs_ow_bus_t * bus, bool value)
{
	ow_set_bus_zero(bus);
	_delay_us(2);
	if (value != 0)
	{
		ow_set_bus_one(bus);
	}

	_delay_us(60);
	ow_set_bus_one(bus);
}

// чтение бита с OW шины
inline static bool read_bit(rscs_ow_bus_t * bus)
{
	ow_set_bus_zero(bus);
	_delay_us (2);
	ow_set_bus_one(bus);
	_delay_us (20);
	bool x = ow_get_bus_value(bus);
	_delay_us(30);
	return x;
}

// ========================================================================

void rscs_ow_init_bus(rscs_ow_bus_t * bus)
{
	*bus->port_reg &= ~bus->pin_mask;
	*bus->ddr_reg &= ~bus->pin_mask;
}


bool rscs_ow_reset(rscs_ow_bus_t * bus)
{
	ow_set_bus_zero(bus);
	_delay_us(600);
	ow_set_bus_one(bus);
	_delay_us(10);

	bool isSomeoneHere = 0;
	for (int i = 0; i < 480; i++)
	{
		if (ow_get_bus_value(bus) == false)
			isSomeoneHere = true;

		_delay_us(1);
	}

	return isSomeoneHere;
}


void rscs_ow_write(rscs_ow_bus_t * bus, uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t bit = (1<<i) & byte;
		ow_write_bit(bus, bit);
	}
}


uint8_t rscs_ow_read(rscs_ow_bus_t * bus)
{
	uint8_t retval = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t bit = read_bit(bus);
		retval = retval | (bit<<i);
	}
	return retval;
}


void rscs_ow_write_n(rscs_ow_bus_t * bus, const void * buffer, size_t buffersize)
{
	const uint8_t * bytes_ptr = (const uint8_t*)buffer;
	for (size_t i = 0; i < buffersize; i++)
		rscs_ow_write(bus, bytes_ptr[i]);
}


void rscs_ow_read_n(rscs_ow_bus_t * bus, void * buffer, size_t buffersize)
{
	uint8_t * bytes_ptr = (uint8_t*)buffer;
	for (size_t i = 0; i < buffersize; i++)
		bytes_ptr[i] = rscs_ow_read(bus);
}


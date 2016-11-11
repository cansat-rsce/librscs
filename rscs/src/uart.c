#include "../include/rscs/uart.h"

#include <stdlib.h>

#include <avr/io.h>

void rscs_uart_init(rscs_uart_bus_t * bus, int flags)
{
	// затираем биты, которые будем настраивать
	// а именно - включение RXTX и режим многопроцессорной связи, который мы маскируем
	*bus->USCRA &= ~(1 << MPCM0);
	*bus->USCRB &= ~((1 << TXEN0) | (1 << RXEN0));

	// теперь активируем RXC TXC согласно настройкам
	if (flags & RSCS_UART_FLAG_ENABLE_RX)
		*bus->USCRB |= (1 << RXC0);

	if (flags & RSCS_UART_FLAG_ENABLE_TX)
		*bus->USCRB |= (1 << TXC0);
}


void rscs_uart_set_character_size(rscs_uart_bus_t * bus, int character_size)
{
	// фильтруем некорректные значения
	// затираем имеющуюся настройку
	*bus->USCRB &= ~((UCSZ02));
	*bus->USCRC &= ~((UCSZ01));

	switch (character_size)
	{
	case 5:
		// все по нулям
		break;

	case 6:
		*bus->USCRC |= (1 << UCSZ00);
		break;

	case 7:
		*bus->USCRC |= (1 << UCSZ01);
		break;

	case 8:
		*bus->USCRC |= (1 << UCSZ00) | (1 << UCSZ01);
		break;

	case 9: // 9ка не поддерживается этой версией
	default:
		abort(); // критическая ошибка.
	}
}


void rscs_uart_set_baudrate(rscs_uart_bus_t * bus, uint32_t baudrate)
{
	// FIXME: тут не учитывается возможность установки бита U2Xn из USCRA

	const uint16_t brr_value = (uint16_t)(F_CPU/(16.0*baudrate)-1);
	*bus->UBRRH = brr_value / 0xFF;
	*bus->UBRRL = brr_value % 0xFF;
}


void rscs_uart_set_parity(rscs_uart_bus_t * bus, rscs_uart_parity_t parity)
{
	// зануляем текущее значение
	*bus->USCRC &= ~((1 << UPM00) | (1 << UPM01));

	switch(parity)
	{
	case RSCS_UART_PARITY_NONE:
		// по нулям
		break;

	case RSCS_UART_PARITY_ODD:
		*bus->USCRC |= (1 << UPM00) | (1 << UPM01);
		break;

	case RSCS_UART_PARITY_EVEN:
		*bus->USCRC |= (1 << UPM01);
		break;

	default:
		abort(); // пришло неверное значение
	}

}


void rscs_uart_set_stop_bits(rscs_uart_bus_t * bus, int stopbits)
{
	// зануляем текущую настройку
	*bus->USCRC &= ~(1 << USBS0);

	switch (stopbits)
	{
	case 1:
		// по нулям
		break;
	case 2:
		*bus->USCRC |= (1 << USBS0);
		break;

	default:
		abort(); // некорректное значение
	}
}


void rscs_uart_write(rscs_uart_bus_t * bus, const void * dataptr, size_t datasize)
{
	const uint8_t * const data = (const uint8_t*)dataptr;
	for (size_t i = 0; i < datasize; i++)
	{
		while (*bus->USCRA & UDRE0) {} // ждем пока буффер не осводобиться
		*bus->UDR = data[i];
	}
}


void rscs_uart_read(rscs_uart_bus_t * bus, void * dataptr, size_t datasize)
{
	uint8_t * const data = (uint8_t*)dataptr;

	for (size_t i = 0; i < datasize; i++)
	{
		while (0 == (*bus->USCRA & TXC0)) {} // ждем пока в буффере что-нибудь не появится
		data[i] = *bus->UDR;
	}
}


#include <stdlib.h>
#include <avr/io.h>

#include "librscs_config.h"

#include "../uart.h"


// Дискриптор UART шины
struct rscs_uart_bus
{
	volatile uint8_t * UDR;   // указатель на регистр UDR модуля
	volatile uint8_t * UCSRA; // указатель на регистр USCRA
	volatile uint8_t * UCSRB; // указатель на регистр USCRB
	volatile uint8_t * UCSRC; // указатель на регистр USCRC

	volatile uint8_t * UBRRL; // указатель на регистр UBRRL
	volatile uint8_t * UBRRH; // указатель на регистр UBRRH
};


rscs_uart_bus_t * rscs_uart_init(rscs_uart_id_t id, int flags)
{
	rscs_uart_bus_t * bus = (rscs_uart_bus_t *)malloc(sizeof(rscs_uart_bus_t));
	if (NULL == bus)
		return bus;

	if (RSCS_UART_ID_UART0 == id)
	{
		bus->UDR = &UDR0;
		bus->UCSRA = &UCSR0A;
		bus->UCSRB = &UCSR0B;
		bus->UCSRC = &UCSR0C;
		bus->UBRRL = &UBRR0L;
		bus->UBRRH = &UBRR0H;
	}
#if defined __AVR_ATmega128__
	else if (RSCS_UART_ID_UART1 == id)
	{
		bus->UDR = &UDR1;
		bus->UCSRA = &UCSR1A;
		bus->UCSRB = &UCSR1B;
		bus->UCSRC = &UCSR1C;
		bus->UBRRL = &UBRR1L;
		bus->UBRRH = &UBRR1H;
	}
#endif
	else
	{
		free(bus);
		return NULL;
	}

	// затираем биты, которые будем настраивать
	// а именно - включение RXTX и режим многопроцессорной связи, который мы маскируем
	*bus->UCSRA &= ~(1 << MPCM0);
	*bus->UCSRB &= ~((1 << TXEN0) | (1 << RXEN0));

	// теперь активируем RXC TXC согласно настройкам
	if (flags & RSCS_UART_FLAG_ENABLE_RX)
		*bus->UCSRB |= (1 << RXEN0);

	if (flags & RSCS_UART_FLAG_ENABLE_TX)
		*bus->UCSRB |= (1 << TXEN0);

	return bus;
}

void rscs_uart_deinit(rscs_uart_bus_t * bus)
{

	free(bus);
}


void rscs_uart_set_character_size(rscs_uart_bus_t * bus, int character_size)
{
	// фильтруем некорректные значения
	// затираем имеющуюся настройку
	*bus->UCSRB &= ~((UCSZ02));
	*bus->UCSRC &= ~((UCSZ01));

	switch (character_size)
	{
	case 5:
		// все по нулям
		break;

	case 6:
		*bus->UCSRC |= (1 << UCSZ00);
		break;

	case 7:
		*bus->UCSRC |= (1 << UCSZ01);
		break;

	case 8:
		*bus->UCSRC |= (1 << UCSZ00) | (1 << UCSZ01);
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
	*bus->UCSRC &= ~((1 << UPM00) | (1 << UPM01));

	switch(parity)
	{
	case RSCS_UART_PARITY_NONE:
		// по нулям
		break;

	case RSCS_UART_PARITY_ODD:
		*bus->UCSRC |= (1 << UPM00) | (1 << UPM01);
		break;

	case RSCS_UART_PARITY_EVEN:
		*bus->UCSRC |= (1 << UPM01);
		break;

	default:
		abort(); // пришло неверное значение
	}

}


void rscs_uart_set_stop_bits(rscs_uart_bus_t * bus, rscs_uart_stopbits_t stopbits)
{
	// зануляем текущую настройку
	*bus->UCSRC &= ~(1 << USBS0);

	switch (stopbits)
	{
	case RSCS_UART_STOP_BITS_ONE:
		// по нулям
		break;
	case RSCS_UART_STOP_BITS_TWO:
		*bus->UCSRC |= (1 << USBS0);
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
		while ( !(bus->UCSRA & (1 << UDRE0)) )
		{}
		bus->UDR = data[i];
	}
}


void rscs_uart_read(rscs_uart_bus_t * bus, void * dataptr, size_t datasize)
{
	uint8_t * const data = (uint8_t*)dataptr;

	for (size_t i = 0; i < datasize; i++)
	{
		while (0 == (*bus->UCSRA & (1 << RXC0))) {} // ждем пока в буффере что-нибудь не появится
		data[i] = *bus->UDR;
	}
}


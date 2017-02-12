#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "librscs_config.h"
#include "../ringbuf.h"
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

#ifdef RSCS_UART_USEBUFFERS
	rscs_ringbuf_t * txbuf;
	rscs_ringbuf_t * rxbuf;
#endif
};


// глобальные дескрипторы для контроля повтороной инициализации UART и
// доступа из прерываний
static rscs_uart_bus_t * _uart0_bus = NULL;
#ifdef __AVR_ATmega128__
static rscs_uart_bus_t * _uart1_bus = NULL;
#endif


rscs_uart_bus_t * rscs_uart_init(rscs_uart_id_t id, int flags)
{
	rscs_uart_bus_t * bus;
	if (RSCS_UART_ID_UART0 == id)
	{
		if (_uart0_bus != NULL)
			return NULL; // уже инициализирован

		bus = _uart0_bus = (rscs_uart_bus_t *)malloc(sizeof(rscs_uart_bus_t));
		if (NULL == bus)
			return NULL;

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
		if (_uart1_bus != NULL)
			return NULL; // уже инициализирован

		bus = _uart1_bus = (rscs_uart_bus_t *)malloc(sizeof(rscs_uart_bus_t));
		if (NULL == bus)
			return NULL;


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

#ifdef RSCS_UART_USEBUFFERS
	if ((flags & RSCS_UART_FLAG_ENABLE_RX) && (flags & RSCS_UART_FLAG_ENABLE_RX))
	{
		// TODO: Включить прерывания!
		bus->rxbuf = rscs_ringbuf_init(RSCS_UART_BUFSIZE_RX);
	}
	else
		bus->rxbuf = NULL;

	if ((flags & RSCS_UART_FLAG_ENABLE_TX) && (flags & RSCS_UART_FLAG_BUFFER_TX))
		bus->txbuf = rscs_ringbuf_init(RSCS_UART_BUFSIZE_TX);
	else
		bus->txbuf = NULL;

	// ну и включаем соответсвующие прерывания
	// TODO:
#endif

	return bus;
}

void rscs_uart_deinit(rscs_uart_bus_t * bus)
{

#ifdef RSCS_UART_USEBUFFERS
	if (bus->rxbuf)
	{
		// TODO: Выключить прерывания!
		rscs_ringbuf_deinit(bus->rxbuf);
	}

	if (bus->txbuf)
	{
		// TODO: Выключить прерывания!
		rscs_ringbuf_deinit(bus->txbuf);
	}
#endif

	if (_uart0_bus == bus)
		_uart0_bus = NULL;

#ifdef __AVR_ATmega128__
	if (_uart1_bus == bus)
		_uart1_bus = NULL;
#endif

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

//Функция для записи без буферизации
void _rscs_uart_write_wobuf(rscs_uart_bus_t * bus, const void * dataptr, size_t datasize)
{
	const uint8_t * const data = (const uint8_t*)dataptr;
	for (size_t i = 0; i < datasize; i++)
	{
		while ( !(*bus->UCSRA & (1 << UDRE0)) )
		{}
		*bus->UDR = data[i];

	}
}

//Функция для чтения без буферизации
void _rscs_uart_read_wobuf(rscs_uart_bus_t * bus, void * dataptr, size_t datasize)
{
	uint8_t * const data = (uint8_t*)dataptr;

	for (size_t i = 0; i < datasize; i++)
	{
		while (0 == (*bus->UCSRA & (1 << RXC0))) {} // ждем пока в буффере что-нибудь не появится
		data[i] = *bus->UDR;
	}
}

//Функция для записи с буферизацией
void _rscs_uart_write_wbuf(rscs_uart_id_t id) {
	uint8_t data;

#ifdef RSCS_UART_USETXBUFFER0
	if(id == RSCS_UART_ID_UART0) {
		if(rscs_ringbuf_pop(txbuf0, &data) == RSCS_E_BUSY) return;
		UDR0 = data;
	}
#endif

#ifdef RSCS_UART_USETXBUFFER1
	if(id == RSCS_UART_ID_UART1) {
		if(rscs_ringbuf_pop(txbuf1, &data) == RSCS_E_BUSY) return;
		UDR1 = data;
	}
#endif
}

//Функция для чтения с буферизацией
void _rscs_uart_read_wbuf(rscs_uart_id_t id) {
#ifdef RSCS_UART_USERXBUFFER0
	if(id == RSCS_UART_ID_UART0) {
		rscs_ringbuf_push(rxbuf0, UDR0); //FIXME что делать, если в буфер не влезает?
	}
#endif

#ifdef RSCS_UART_USETXBUFFER1
	if(id == RSCS_UART_ID_UART1) {
		rscs_ringbuf_push(rxbuf1, UDR1); //FIXME что делать, если в буфер не влезает?
	}
#endif
}

void rscs_uart_write(rscs_uart_bus_t * bus, const void * dataptr, size_t datasize)
{
	const uint8_t * const data = (const uint8_t*)dataptr;

#ifdef RSCS_UART_USEBUFFERS
	if (bus->txbuf != NULL)
	{
		// если буферизация доступна и разрешена - просто дергаем write_some
		// пока она не запишет все что может
		size_t writen = 0;
		while(writen < datasize)
		{
			writen += rscs_uart_write_some(bus, data+writen, datasize-writen);
		}

		return;
	}
#else
	// в противном случае действум по старинке
	for (size_t i = 0; i < datasize; i++)
	{
		while ( !(*bus->UCSRA & (1 << UDRE0)) )
		{}
		*bus->UDR = data[i];

	}
#endif
}


void rscs_uart_read(rscs_uart_bus_t * bus, void * dataptr, size_t datasize)
{
	uint8_t * const data = (const uint8_t*)dataptr;

#ifdef RSCS_UART_USEBUFFERS
	if (bus->rxbuf != NULL)
	{
		// если буферизация доступна и разрешена - просто дергаем read_some
		// пока она не прочитает сколько нам надо
		size_t readed = 0;
		while(readed < datasize)
		{
			readed += rscs_uart_read_some(bus, data+readed, datasize-readed);
		}

		return;
	}
#else
	// в противном случае действуем по-старинке

	for (size_t i = 0; i < datasize; i++)
	{
		while (0 == (*bus->UCSRA & (1 << RXC0))) {} // ждем пока в буффере что-нибудь не появится
		data[i] = *bus->UDR;
	}
#endif
}



#ifdef RSCS_UART_USEBUFFERS

size_t rscs_uart_write_some(rscs_uart_bus_t * bus, const void * dataptr, size_t datasize)
{
	const uint8_t * const data = (const uint8_t*)dataptr;
	size_t writen = 0;

	// загоняем все что есть в циклический буфер, пока в нем не кончится место
	// или пока у нас не кончатся данные
	while (writen < datasize)
	{
		if (!rscs_ringbuf_push(bus->txbuf, data[writen]))
			break;

		writen++;
	}

	// теперь, когда данные готовы в кольцевом буфере,
	// нужно запустить цепь прерываний, которые будут переносить данные из кольцевого буфера
	// в регистр соотетствующего уарта
	// вполне возможно, что эта цепь уже запущена - еще не отправлена предидущая порция
	// так или иначе - нам достаточно просто разрешить прерывание на событие пустого выходного регистра
	*bus->UCSRB |= (1 << UDRE0);

	// возвращаем сколько удалось записать
	return writen;
}


size_t rscs_uart_read_some(rscs_uart_bus_t * bus, void * dataptr, size_t dataisize)
{
	uint8_t * const data = (const uint8_t*)dataptr;
	size_t writen = 0;

	// TODO: дописать!

	return writen;
}


inline static void _do_rx_interrupt(rscs_uart_bus_t * bus)
{

}

inline static void _do_udre_interrupt(rscs_uart_bus_t * bus)
{

}


#ifdef __AVR_ATmega128__

	ISR(USART0_RX_vect)		{ _do_rx_interrupt(_uart0_bus);		}
	ISR(USART1_RX_vect)		{ _do_rx_interrupt(_uart1_bus);		}
	ISR(USART0_UDRE_vect)	{ _do_udre_interrupt(_uart0_bus);	}
	ISR(USART1_UDRE_vect)	{ _do_udre_interrupt(_uart1_bus);	}

#elif defined __AVR_ATmega328P__

	ISR(USART_RX_vect)		{ _do_rx_interrupt(_uart0_bus);		}
	ISR(USART_UDRE_vect)	{ _do_udre_interrupt(_uart0_bus);	}

#else
	#error "Не поддерживаемый микроконтроллер"
#endif // __AVR_ATmega128__

#endif // RSCS_UART_USEBUFFERS

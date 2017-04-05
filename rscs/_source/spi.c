#include "librscs_config.h"

#include "../spi.h"

#include <stdlib.h>

// для удобства - локальная инлайновая функция
inline static uint8_t _spi_do_inline(uint8_t value)
{
	SPDR = value;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

void rscs_spi_init(void)
{
	/*настройка портов ввода-вывода: все на вывод, кроме MISO */
	RSCS_SPI_DDRX  |=
			(1 << RSCS_SPI_MOSI) | (1 << RSCS_SPI_SCK) | (1 << RSCS_SPI_SS) | (0 << RSCS_SPI_MISO);
	RSCS_SPI_PORTX |=
			(1 << RSCS_SPI_MOSI) | (1 << RSCS_SPI_SCK) | (1 << RSCS_SPI_SS) | (0 << RSCS_SPI_MISO);

	/*разрешение spi,старший бит вперед,мастер, режим 0*/
	SPCR = (1 << SPE) | (1 << MSTR);

	rscs_spi_set_clk(63);
}


void rscs_spi_set_clk(uint32_t clock_kHz)
{
	// зануляем все биты, управляющие частотой
	// будем расставлять единицы где это необходимо
	SPCR &= ~((1 << SPR0) | (1 << SPR1));
	SPSR &= ~(1 << SPI2X);

	const uint16_t divisor = F_CPU/1000/clock_kHz; //uint16 тут нужен, чтобы избежать переполнения при попытке выбора очень малой скорости

	// начинаем с самого быстрого варианта
	// и проверяя все диапазоны округляем до меньшего делителя
	if (divisor <= 2) // к сожалению мы не можем установить меньший делитель
	{
		SPSR |= (1 << SPI2X);
	}
	else if (divisor <= 4) // но больше 2
	{
		// все по нулям
	}
	else if (divisor <= 8) // но больше 4
	{
		SPCR |= (1 << SPR0);
		SPSR |= (1 << SPI2X);
	}
	else if (divisor <= 16) // но больше 8
	{
		SPCR |= (1 << SPR0);
	}
	else if (divisor <= 32) // но больше 16
	{
		SPCR |= (1 << SPR1);
		SPSR |= (1 << SPI2X);
	}
	else if (divisor <= 64) // но больше 32
	{
		SPCR |= (1 << SPR1) | (1 << SPR0);
		SPSR |= (1 << SPI2X);
	}
	else if (divisor <= 128) // но больше 64
	{
		SPCR |= (1 << SPR1) | (1 << SPR0);
	}
	else // мы не можем быть настолько медленными
	{
		abort();
	}
}


void rscs_spi_set_pol(rscs_spi_polarity_t polarity)
{
	SPCR &= ~((1 << CPOL) | (1 << CPHA));

	switch(polarity)
	{
	case RSCS_SPI_POL_SAMPLE_RISE_SETUP_FALL:
		SPSR |= (0 << CPOL) | (0 << CPHA);
		break;
	case RSCS_SPI_POL_SETUP_RISE_SAMPLE_FALL:
		SPSR |= (0 << CPOL) | (1 << CPHA);
		break;
	case RSCS_SPI_POL_SAMPLE_FALL_SETUP_RISE:
		SPSR |= (1 << CPOL) | (0 << CPHA);
		break;
	case RSCS_SPI_POL_SETUP_FALL_SAMPLE_RISE:
		SPSR |= (1 << CPOL) | (1 << CPHA);
		break;
	default:
		abort();
	};
}


void rscs_spi_set_order(rscs_spi_order_t order)
{
	SPCR &= ~(1 << DORD);

	switch(order)
	{
	case RSCS_SPI_ORDER_LSB_FIRST:
		SPCR |= (1 << DORD);
		break;
	case RSCS_SPI_ORDER_MSB_FIRST:
		SPCR |= (0 << DORD);
		break;
	default:
		abort();
	};
}

uint8_t rscs_spi_do(uint8_t value)
{
	return _spi_do_inline(value);
}


void rscs_spi_read(void * read_buffer, size_t buffer_size, uint8_t dummy)
{
	for (size_t i = 0; i < buffer_size; i++)
		((uint8_t*)read_buffer)[i] = _spi_do_inline(dummy);
}


void rscs_spi_write(const void * write_buffer, size_t buffer_size)
{
	for (size_t i = 0; i < buffer_size; i++)
		_spi_do_inline(((const uint8_t*)write_buffer)[i]);
}


void rscs_spi_exchange(const void * write_buffer, void * read_buffer, size_t buffers_size)
{
	for (size_t i = 0; i < buffers_size; i++)
		((uint8_t*)read_buffer)[i] = _spi_do_inline(((const uint8_t*)write_buffer)[i]);
}

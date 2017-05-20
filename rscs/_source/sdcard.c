#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>

#include "librscs_config.h"

#include "../sdcard.h"
#include "../crc.h"
#include "../error.h"

// =========================================================
// Вспомогательные сущности
// =========================================================

struct rscs_sdcard
{
	volatile uint8_t * cs_ddr; 	// регистр DDR порта, на котором расположен CS пин
	volatile uint8_t * cs_port;	// регистр PORT порта, на котором расположен CS пин
	uint8_t cs_pin_mask;		// номер пина CS в порту

	uint32_t timeout;			// таймаут для операций SD карты
};

#define SD_R1_IDLE (1 << 0)
#define SD_R1_ILLEGAL_CMD (1 << 2)
#define SD_R1_CRC_ERR (1 << 3)

#define SD_DATA_RESP_MASK           0x1F  // маска ответа SD карты о приеме данных
#define SD_DATA_RESP_DATA_ACCEPTED  0x05  // ответ SD карты - данные получены
#define SD_DATA_RESP_CRC_ERROR      0x0B  // ответ SD карты - ошибка CRC
#define SD_DATA_RESP_WRITE_ERROR    0x0D  // ответ SD карты - ошибка записи

#define SD_TOKEN_SINGLE_DATA 0xFE   // Токен заголовка пакета для CMD17/18/24
#define SD_TOKEN_MULTI_DATA 0xFC    // Токен заголовка пакета для CMD25
#define SD_TOKEN_NO_MORE_DATA 0xFD  // Токен заверешения передачи данных для CMD25

#define GOTO_END_IF_ERROR(X) if ((error = (X)) != RSCS_E_NONE) goto end;

// =================================================================

rscs_sdcard_t * rscs_sd_init(volatile uint8_t * cs_ddr_reg,volatile uint8_t * cs_port_reg, uint8_t cs_pin_mask)
{
	rscs_sdcard_t * self = (rscs_sdcard_t*)malloc(sizeof(rscs_sdcard_t));
	if (NULL == self)
		return self;

	self->cs_ddr = cs_ddr_reg;
	self->cs_port = cs_port_reg;
	self->cs_pin_mask = cs_pin_mask;
	self->timeout = 4000;

	// настраиваем cs пин на вывод
	*self->cs_ddr |= (self->cs_pin_mask);
	// и ставим его в OFF состояние, воизбежание
	rscs_sd_cs(self, false);

	return self;
}

void rscs_sd_set_timeout(rscs_sdcard_t * self, uint32_t timeout_us){
	self->timeout = timeout_us;
}

void rscs_sd_deinit(rscs_sdcard_t * self)
{
	free(self);
}


void rscs_sd_spi_setup(void)
{
	rscs_spi_set_order(RSCS_SPI_ORDER_MSB_FIRST);
	rscs_spi_set_pol(RSCS_SPI_POL_SAMPLE_RISE_SETUP_FALL);
	rscs_spi_set_clk(RSCS_SDCARD_SPI_CLK_FAST); // частоту на маскимально возможную
}


void rscs_sd_spi_setup_slow(void)
{
	rscs_spi_set_order(RSCS_SPI_ORDER_MSB_FIRST);
	rscs_spi_set_pol(RSCS_SPI_POL_SAMPLE_RISE_SETUP_FALL);
	rscs_spi_set_clk(RSCS_SDCARD_SPI_CLK_SLOW); // не более 400кгц
}


void rscs_sd_cs(rscs_sdcard_t * self, bool state)
{
	if (state)
		*self->cs_port |= self->cs_pin_mask;
	else
		*self->cs_port &= ~self->cs_pin_mask;
}


void rscs_sd_write(rscs_sdcard_t * card, const void * buffer, size_t buffer_size)
{
	rscs_spi_write(buffer, buffer_size);
}


void rscs_sd_read(rscs_sdcard_t * card, void * buffer, size_t buffer_size)
{
	rscs_spi_read(buffer, buffer_size, 0xFF);
}


rscs_sd_resp_t rscs_sd_response_type(rscs_sd_cmd_t cmd)
{
	switch (cmd)
	{
	case RSCS_SD_CMD0:
	case RSCS_SD_CMD1:
	case RSCS_SD_CMD12:
	case RSCS_SD_CMD17:
	case RSCS_SD_CMD18:
	case RSCS_SD_CMD24:
	case RSCS_SD_CMD55:
	case RSCS_SD_CMD41:
	case RSCS_SD_CMD25:
		return RSCS_SD_R1;
	case RSCS_SD_CMD8:
		return RSCS_SD_R7;
	default:
		abort();
		return 0; // не дойдем до сюда
	}
}


size_t rscs_sd_response_length(rscs_sd_resp_t resp)
{
	switch (resp)
	{
	case RSCS_SD_R1:
		return 1;
	case RSCS_SD_R3:
		return 4;
	case RSCS_SD_R7:
		return 5;
	default:
		abort();
		return 0; // не дойдем до сюда, но компилятор ругается
	}
}


rscs_e rscs_sd_cmd(rscs_sdcard_t * self, rscs_sd_cmd_t cmd, uint32_t argument, void * response)
{
	const uint8_t * arg = (const uint8_t *)&argument;

	uint8_t data[] = { (uint8_t)((cmd & 0x3F) | 0x40), arg[3], arg[2], arg[1], arg[0], 0x95/*CRC*/};
	// 0x95 - фиксированное значение CRC для CMD0, для которой без CRC никак. Для остальных CRC мы не испольуем
	data[5] = rscs_crc7(data, 5);
	rscs_sd_write(self, data, sizeof(data));

	rscs_sd_resp_t resp_type = rscs_sd_response_type(cmd);
	size_t response_length = rscs_sd_response_length(resp_type);

	uint8_t * volatile first_response_byte_ptr = (uint8_t*)response;
	uint32_t timespent = 0;
	while(1)
	{
		rscs_sd_read(self, first_response_byte_ptr, 1);
		if ((*first_response_byte_ptr & 0x80) != 0) // первый бит любого ответа должен быть равен 0
			break;

		// пока ответа нет - шина держится в единице (0xFF)
		if (self->timeout)
		{
			_delay_us(1);
			if (timespent++ > self->timeout)
				return RSCS_E_TIMEOUT;
		}

	}

	// дочитываем остальное
	rscs_sd_read(self, first_response_byte_ptr+1, response_length-1);

	return RSCS_E_NONE;
}


rscs_e rscs_sd_wait_busy(rscs_sdcard_t * self)
{
	uint32_t timespent = 0;
	while(1)
	{
		uint8_t buffer;
		rscs_sd_read(self, &buffer, 1);
		if (0x00 == buffer)
			break;

		_delay_us(1);
		if (self->timeout)
		{
			timespent++;
			if (timespent > self->timeout)
				return RSCS_E_TIMEOUT;
		}
	}

	return RSCS_E_NONE;
}


rscs_e rscs_sd_startup(rscs_sdcard_t * self)
{
	rscs_e error = RSCS_E_NONE;
	rscs_sdcard_type_t card_type;

	// прокачиваем 20 SPI тактов на SD карту при выключенном CS и удерживаю на линии MOSI единицу (0xFF)
	rscs_sd_cs(self, false);
	const uint8_t dummy = 0xFF;
	for (uint_fast8_t i = 0; i < 20; i++)
		rscs_sd_write(self, &dummy, 1);

	// Переключаем SPI на высокую скорость
	rscs_sd_cs(self, true);
	// отправляем карте команду CMDO уже при включенном CS
	uint8_t r1_resp = 0x00;
	GOTO_END_IF_ERROR(rscs_sd_cmd(self, RSCS_SD_CMD0, 0x00, &r1_resp));
	if (r1_resp & ~SD_R1_IDLE) // должен быть только IDLE бит, без других ошибок
	{
		error = RSCS_E_INVRESP;
		goto end;
	}

	// отлично - карта в состоние IDLE

	// заставим её проверить напряжение питания
	// ошибки игнорируем, тк эту команду понимают только некоторые SD карты
	// заодно и выясним - какой тип карточки у нас
	uint8_t r7_resp[5];
	rscs_sd_cmd(self, RSCS_SD_CMD8, 0x000001AA, r7_resp); // 0xAA - спец обязательный паттерн. 0x01 - команда на проверку питания
	if (r7_resp[0] & SD_R1_ILLEGAL_CMD) // карта не поняла команду - это SD1
		card_type = RSCS_SD_TYPE_SD1;
	else if (0xAA == r7_resp[4]) // карта поняла нашу команду - это SD2 карта
		card_type = RSCS_SD_TYPE_SD2;
	else // карта ведет себя неадекватно
	{
		error = RSCS_E_INVRESP;
		goto end;
	}

	// долбим карту командами не включение, пока ей не надоест
	// если это SDHC, она ждет от нас ACMD41

	// для SD2 аргумент команды 41 должен быть 0X40000000, для остальных 0
	uint32_t cmd41_arg = (card_type == RSCS_SD_TYPE_SD2) ? 0X40000000 : 0;
	do
	{
		GOTO_END_IF_ERROR(rscs_sd_cmd(self, RSCS_SD_CMD55, 0x00, &r1_resp));
		if (r1_resp & ~SD_R1_IDLE) // должен быть только IDLE бит, без других ошибок
		{
			error = RSCS_E_INVRESP;
			goto end;
		}

		GOTO_END_IF_ERROR(rscs_sd_cmd(self, RSCS_SD_CMD41, cmd41_arg, &r1_resp));
		if (r1_resp & ~SD_R1_IDLE) // есть биты ошибки
		{
			if (RSCS_SD_TYPE_SD2 == card_type) // SD V2 не позволительно тут давать ошибку - с ней что-то не то
			{
				error = RSCS_E_INVRESP;
				goto end;
			}
			else
			{
				break; // остальные карты могут ошибаться - мы будем пробовать на них CMD1
			}
		}
	} while (r1_resp != 0x00);


	// если это карта, которая не понимает ACMD41 и idle бит все еще стоит - пробуем запустить её через CMD1
	while (r1_resp != 0x00)
	{
		GOTO_END_IF_ERROR(rscs_sd_cmd(self, RSCS_SD_CMD1, 0x00, &r1_resp));
		if (r1_resp & ~SD_R1_IDLE) // есть биты ошибки
		{
			error = RSCS_E_INVRESP;
			goto end;
		}
	}

	// все, карта готова
end:
	rscs_sd_cs(self, false);
	return error;
}


rscs_e rscs_sd_block_write(rscs_sdcard_t * self, size_t offset, const void * block_start, size_t block_count)
{
	rscs_e error = RSCS_E_NONE;
	const uint16_t dummy = 0xFFFF;
	// определяемся с командой и токенами - запись нескольких блоков или одного
	const uint8_t packet_token = block_count > 1 ? SD_TOKEN_MULTI_DATA : SD_TOKEN_SINGLE_DATA;
	const rscs_sd_cmd_t write_cmd = block_count > 1 ? RSCS_SD_CMD25 : RSCS_SD_CMD24;

	rscs_sd_cs(self, true);
	GOTO_END_IF_ERROR(rscs_sd_wait_busy(self));

	// отправляем команду на запись
	uint8_t resp = 0x00;
	GOTO_END_IF_ERROR(rscs_sd_cmd(self, write_cmd, (uint32_t)offset, &resp));
	if (resp != 0x00)
	{
		error = RSCS_E_INVRESP;
		goto end;
	}
	// теперь отправляем специальный "пропускающий" байт
	rscs_sd_write(self, &dummy, 1);

	for (size_t i = 0; i < block_count; i++)
	{
		if (i != 0)
			GOTO_END_IF_ERROR(rscs_sd_wait_busy(self)); // ждем пока карта освободиться

		// заголовок пакета данных
		rscs_sd_write(self, &packet_token, 1);
		// сами данные
		rscs_sd_write(self, (uint8_t*)block_start + 512*i, 512);
		// контрольная сумма
		rscs_sd_write(self, &dummy, 2);

		// sd карта отвечает на пакет - слушаем
		rscs_sd_read(self, &resp, 1);
		resp &= SD_DATA_RESP_MASK;
		if (resp != SD_DATA_RESP_DATA_ACCEPTED)
		{
			error = RSCS_E_INVRESP;
			break;
		}
	}

	if (block_count > 1)
	{
		GOTO_END_IF_ERROR(rscs_sd_wait_busy(self));
		// заканчиваем - отправляем соответсвующий токен
		const uint8_t stop_tran_token = SD_TOKEN_NO_MORE_DATA;
		rscs_sd_write(self, &stop_tran_token, 1);
		// нужно прогнать еще один пропускающий байт, чтобы карта вошла в состояние busy
		// чтобы начать запись на самой sd карте
		// нужно отправить еще хотябы один байт по SPI в состоянии busy
		// (согласно статье http://elm-chan.org/docs/mmc/mmc_e.html)
		// поэтому отправляем сразу два байта
		rscs_sd_write(self, &dummy, 2);
	}

	//sd_wait_busy();

end:
	rscs_sd_cs(self, false);
	return error;
}


rscs_e rscs_sd_block_read(rscs_sdcard_t * self, size_t offset, void * block, size_t block_count)
{
	rscs_e error = RSCS_E_NONE;

	// определяемся с командой на чтение в зависимости от количества блоков
	rscs_sd_cmd_t read_cmd = block_count > 1 ? RSCS_SD_CMD18 : RSCS_SD_CMD17;

	rscs_sd_cs(self, true);
	GOTO_END_IF_ERROR(rscs_sd_wait_busy(self));

	// Подготовка к чтению блока - отправка команды и заголовков
	uint8_t resp_r1 = 0;
	GOTO_END_IF_ERROR(rscs_sd_cmd(self, read_cmd, (uint32_t)offset, &resp_r1));
	if (resp_r1 != 0x00)
	{
		error = RSCS_E_INVRESP;
		goto end;
	}

	for (size_t i = 0; i < block_count; i++)
	{
		// ждем токена о начале передачи
		uint8_t token;
		do
		{
			rscs_sd_read(self, &token, 1);
		} while (token == 0xFF);

		// проверяем токен
		if (token != SD_TOKEN_SINGLE_DATA)
		{
			error = RSCS_E_INVRESP;
			break;
		}

		// читаем сам блок
		rscs_sd_read(self,(uint8_t*)block + 512*i, 512);

		// читаем CRC
		uint16_t crc;
		rscs_sd_read(self, &crc, 2);

	}

	if (block_count > 1)
	{
		// горшочек - не вари. Засылаем спеиальную команду на остановку
		rscs_sd_cmd(self, RSCS_SD_CMD12, 0x00, &resp_r1);
		if (resp_r1 != 0x00)
		{
			error = RSCS_E_INVRESP;
			goto end;
		}
	}

	// карта вошла в состояние busy.
	// когда блоки пишутся, нужно отправить хотябы один байт, чтобы запись реально пошла
	// может быть с чтением такая же ерунда?
	uint8_t dummy;
	rscs_sd_read(self, &dummy, 1);
	//sd_wait_busy();

end:
	rscs_sd_cs(self, false);
	return error;
}



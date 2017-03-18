/* */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../gps_nmea.h"


#ifdef RSCS_UART_USEBUFFERS // Модуль имеет смысл только с включенной буферизацией UART


typedef enum
{
	GPS_STATE_IDLE, 		// ждем доллара
	GPS_STATE_ACCUMULATE,	// накапливаем символы
} state_t;

struct rscs_gps_t
{
	rscs_uart_bus_t * uart;
	state_t state;
	char buffer[RSCS_GPS_BUFFER_SIZE];
	size_t buffer_carret;
};


rscs_gps_t * rscs_gps_init(rscs_uart_id_t uartId)
{
	rscs_uart_bus_t * uart = rscs_uart_init(uartId,
			RSCS_UART_FLAG_ENABLE_RX | RSCS_UART_FLAG_BUFFER_RX);

	if (NULL == uart)
		return NULL;

	// создаем дескритор и настраиваем
	rscs_gps_t * retval = (rscs_gps_t *)malloc(sizeof(rscs_gps_t));
	if (NULL == retval)
	{
		rscs_uart_deinit(uart);
		return NULL;
	}

	retval->uart = uart;
	retval->buffer_carret = 0;
	retval->state = GPS_STATE_IDLE;
	// настройка UART
	rscs_uart_set_baudrate(retval->uart, RSCS_GPS_UART_BAUD_RATE);
	rscs_uart_set_stop_bits(retval->uart, RSCS_GPS_UART_STOP_BITS);
	rscs_uart_set_parity(retval->uart, RSCS_GPS_UART_PARITY);
	rscs_uart_set_character_size(retval->uart, 8);

	return retval;
}


void rscs_gps_deinit(rscs_gps_t * gps)
{
	rscs_uart_deinit(gps->uart);
	free(gps);
}


static int _explode(const char * str, size_t msgSize, char symbol, const char ** results,
		size_t results_size)
{
	uint8_t numbersOfStr = 0;
	for (int i = 0; i < msgSize; i++){
		if (str[i] == symbol){
			results[numbersOfStr] = &str[i + 1];
			if (numbersOfStr > results_size)
				return 0;
			numbersOfStr++;
		}
	}
	return numbersOfStr;
}

static bool _handle_message(const char * msg_signed, size_t msgSize, float * lon, float * lat,
						    float * height, bool * hasFix)
{
	const uint8_t * msg = (const uint8_t *)msg_signed;
	char chksum = msg[1]; // пропускаем нулевой символ $

	size_t chksumLimit = msgSize - 5; // Пропускаем *XX\r\n
	for (size_t i = 2; i < chksumLimit; i++)
		chksum = chksum ^ msg[i];

	int expectedChksumValue;
	if (sscanf(msg_signed + chksumLimit+1, "%X", &expectedChksumValue) == 0)
		return false;

	if (chksum != expectedChksumValue)
		return false;

	const char * results[16];
	uint8_t numbersOfStr = _explode(msg_signed, msgSize, ',', results, sizeof(results));
	if (numbersOfStr == 0)
		return false;

	// разбираем долготу
	if (sscanf(results[1], "%f", lon) != 1)
		return false;

	if(*results[2] != 'N')
		*lon *= -1;

	//разбираем широту
	if (sscanf(results[3], "%f", lat) != 1)
		return false;

	if (*results[4] != 'E')
		*lat *= -1;

	//разбираем высоту
	if (sscanf(results[9], "%f", height) != 1)
		return false;

	//проверка качества
	int fixQual;
	if (sscanf(results[5], "%i", &fixQual) != 0)
		*hasFix = false;
		else
		*hasFix = true;
	return true;
}


rscs_e rscs_gps_read(rscs_gps_t * gps, float * lon, float * lat,
					 float * height, bool * hasFix)
{
	// нужно прочитать данные из связанного UART
	// выкинуть все не нужное, найти нужное сообщение
	// и выделить из него широту, долготу и высоту

	char symbol;
	size_t readed;

again:
	switch(gps->state)
	{
	case GPS_STATE_IDLE:
		while(0 != (readed = rscs_uart_read_some(gps->uart, &symbol, 1)))
		{
			if ('$' == symbol)
				break;
		}

		if (readed > 0 && '$' == symbol)
		{
			gps->state = GPS_STATE_ACCUMULATE;
			gps->buffer[0] = symbol;
			gps->buffer_carret = 1;
		}
		else
			return RSCS_E_BUSY;

		// брейк опущен сознательно
	case GPS_STATE_ACCUMULATE:
		while(0 != (readed = rscs_uart_read_some(gps->uart, &symbol, 1)))
		{
			if ('$' == symbol)
			{
				gps->buffer[0] = symbol;
				gps->buffer_carret = 1;
				continue;
			}

			if (gps->buffer_carret >= sizeof(gps->buffer))
			{
				gps->state = GPS_STATE_IDLE;
				goto again;
			}

			gps->buffer[gps->buffer_carret] = symbol;
			gps->buffer_carret += readed;

			if (gps->buffer_carret == 6 && memcmp(gps->buffer, "$GPGGA", 6) != 0)
			{
				gps->state = GPS_STATE_IDLE;
				goto again;
			}

			if (gps->buffer_carret >= 2
				&& '\n' == gps->buffer[gps->buffer_carret-1]
			    && '\r' == gps->buffer[gps->buffer_carret-2])
			{
				if (_handle_message(gps->buffer, gps->buffer_carret, lon, lat, height, hasFix))
				{
					gps->state = GPS_STATE_IDLE;
					return RSCS_E_NONE;
				}
				else
				{
					gps->state = GPS_STATE_IDLE;
					goto again;
				}
			}
		}
		break;
	}

	// если сообщения не нашлось, нужно вернуть RSCS_E_BUSY
	return RSCS_E_BUSY;
}


#endif

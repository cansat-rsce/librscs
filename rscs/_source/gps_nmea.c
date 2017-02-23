/* */

#include <stdlib.h>

#include "../gps_nmea.h"


#ifdef RSCS_UART_USEBUFFERS // Модуль имеет смысл только с включенной буферизацией UART


struct rscs_gps_t
{
	rscs_uart_bus_t * uart;
};


rscs_gps_t * rscs_gps_init(rscs_uart_bus_t * uart)
{
	// если нам дали плохой уарт - сами дураки
	if (NULL == uart)
		return NULL;

	// создаем дескритор и настраиваем
	rscs_gps_t * retval = (rscs_gps_t *)malloc(sizeof(rscs_gps_t));
	if (NULL == retval)
		return NULL;

	retval->uart = uart;
	// настройка UART
	//rscs_uart_set_baudrate(retval->uart, ...);

	return retval;
}


void rscs_gps_deinit(rscs_gps_t * gps)
{
	rscs_uart_deinit(gps->uart);
	free(gps);
}


rscs_e rscs_gps_read(rscs_gps_t * gps, float * lon, float * lat, float * height)
{
	// нужно прочитать данные из связанного UART
	// выкинуть все не нужное, найти нужное сообщение
	// и выделить из него широту, долготу и высоту

	// если сообщения не нашлось, нужно вернуть RSCS_E_BUSY
	return RSCS_E_BUSY;
}


#endif

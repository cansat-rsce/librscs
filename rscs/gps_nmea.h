/* */

#ifndef RSCS_GPS_NMEA_H_
#define RSCS_GPS_NMEA_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "librscs_config.h"
#include "uart.h"
#include "error.h"

#ifdef RSCS_UART_USEBUFFERS // Модуль имеет смысл только с включенной буферизацией UART

struct rscs_gps_t;
typedef struct rscs_gps_t rscs_gps_t;

// Инициализация GPS, принимает идентификатор UART-а на котором висит GPS приёмник
// сам открывает UART и настраивает его
rscs_gps_t * rscs_gps_init(rscs_uart_id_t uartId);

// Деинициализация GPS, вместе c UART, на котором он висит
void rscs_gps_deinit(rscs_gps_t * gps);

// чтение накопленных в буфере UART данных, их разбор и выделение из них широты, долготы и высоты
// если в буфере UART еще не накопилось необходимое сообщение - вернет ошибку RSCS_E_BUSY
rscs_e rscs_gps_read(rscs_gps_t * gps, float * lon, float * lat, float * height, bool * hasFix);


#endif
#endif /* RSCS_GPS_NMEA_H_ */

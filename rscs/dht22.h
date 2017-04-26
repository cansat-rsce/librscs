/*
 * dht022.h
 *
 * Работа с датчиком температуры и относительной влажности DHT022
 * Даташит: https://iprototype.nl/docs/DHT22-temperature-humidity-technisch-datasheet-dht22.pdf
 * Неплохое описание на русском: http://avrproject.ru/publ/kak_podkljuchit/rabota_s_datchikom_vlazhnosti_dht11_v_bascom_avr/2-1-0-72
 */

#ifndef DHT022_H_
#define DHT022_H_

#include <stdint.h>

#include "error.h"

struct rscs_dht22_t;
typedef struct rscs_dht22_t rscs_dht22_t;

// Возможные значения ошибок при работе с DHT022
/*typedef enum
{
	DHT_ERROR_NO_REPLY = -1, 		//!< Устройство не отвечает сигналом присутствия
	DHT_ERROR_REPLY_TOO_LONG = -2,	//!< Сигнал присутствия устрйоства слишком долгий (короткое замыкание на ноль?)
	DHT_ERROR_WAIT_TO_LONG = -3,	//!< Пауза перед выдачей бита данных слишком долгая (короткое замыкание на ноль?)
	DHT_ERROR_BIT_TO_LONG = -4,		//!< Сигнал передачи бита слишком длинный (короткое замыкание на +5?)
	DHT_ERROR_CHECKSUM_WRONG = -5, //!< Неверная контрольная сумма пакета
} DHT_error_value;*/


// Инициализация DHT22.
/* Параметров не подразумевает. номер пина смотри в соответсвующем .c файле*/
rscs_dht22_t *  rscs_dht22_init(volatile uint8_t * PORTREG, volatile uint8_t * PINREG, volatile uint8_t * DDRREG, uint8_t PIN);

// Получение данных от DHT22
/*
 Длительность обмена ~5мс
 Даташит не велит запускать чаще чем раз в две секунды.
 Если все хорошо - функция возвращает ноль и записывает температуру и влажность по указателям,
 переданным в аргуметах.

 В случае ошибки фунцкия вовзращает одно из значений енума DHT_error_value
 Значения с датчикам по указателям при этом не передаются

 Возвращаемые этой функцией сырые значения переводятся в % относительной влажности и в градусы простым делением на 10.
 %RH = humididty/10.0f;
 °C = temp/10.0f;
*/
rscs_e rscs_dht22_read(rscs_dht22_t * dht, uint16_t * humidity, int16_t * temp);


#endif /* DHT022_H_ */

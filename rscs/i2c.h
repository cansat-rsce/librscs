/*
 * i2c.h
 *
 * Работа с шиной I2C
 *
 * Сама шина хорошо описана тут:
 * http://easyelectronics.ru/interface-bus-iic-i2c.html
 *
 * Описание модуля I2C микроконтроллеров AVR в переведенном даташите - тут
 * http://www.gaw.ru/html.cgi/txt/doc/micros/avr/arh128/18.htm
 *
 */

#ifndef RSCS_I2C_H_
#define RSCS_I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "error.h"

typedef uint8_t i2c_addr_t;

// Инициализация i2c.
void rscs_i2c_init(void);

// Сброс модуля I2C. Полезно при повторном запуске после обнаружения ошибки
// рекомендуется делать перед первым стартом сесии обмена данными
void rscs_i2c_reset(void);

// Установка частоты линии scl (кГц)
void rscs_i2c_set_scl_rate(uint32_t f_scl_kHz);

// Создает на i2c шине событие START.
// Если шина и так находится под нашим контролем - этот вызов создаст событие RESTART
rscs_e rscs_i2c_start(void);

// Создает на шине состояние STOP
rscs_e rscs_i2c_stop(void);

// Отправляет на шину 7мибитный адрес ведомого устройства с битом-флагом чтения/записи
/* Второй пареметр == 0 - обращение к ведомому в режиме записи.
   Второй параметр == 1 - обращение к ведомому в режиме чтения.*/
rscs_e rscs_i2c_send_slaw(uint8_t slave_addr, bool read_access);

// Передача данных ведомому устройству
/* Перед вызовом этой функции на шину нужно передать адрес ведомого без бита чтения - `i2c_send_slaw(..., 0);`

   Записывает data_size байт из памяти по указателю data_ptr на устройство. */
rscs_e rscs_i2c_write(const void * data_ptr, size_t data_size);

// Получение данных от ведомого устройства
/* Перед вызовом этой функции на шину нужно передать адрес ведомого без бита чтения - `i2c_send_slaw(..., 0);`

   Получает data_size байт от ведомого устройства и записывает по указанному указателю data_ptr.
   аргумент NACK_at_end позволяет управлять подтверждением получения байта от ведомого.
   если этот аргумент установлен в true, то после получения последнего байта ведомому будет передан
   Not ACKnowledge, что означает завершение сеанса чтения. */
rscs_e rscs_i2c_read(void * data_ptr, size_t data_size, bool NACK_at_end);

#endif /* RSCS_I2C_H_ */

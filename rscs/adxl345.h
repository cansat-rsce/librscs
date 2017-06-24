#ifndef RSCS_ADXL345_H_
#define RSCS_ADXL345_H_

#include <stdint.h>
#include <avr/io.h>	//ВРЕМЕННО

#include "i2c.h"

#include "error.h"

/*
	модуль работы с акселерометром ADXL375 через интерфейс I2C
	Даташит: http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
	Неплохое описание на русском: http://www.russianelectronics.ru/leader-r/review/2193/doc/49907/
*/
//TODO: ADXL: написать функцию rscs_adxl345_low_power(bool low_power)
//TODO: ADXL: написать функцию, запускающую self-test


// дескриптор устройства
struct rscs_adxl345_t;
typedef struct rscs_adxl345_t rscs_adxl345_t;


// Возможные адреса устройства на шине
typedef enum
{
	// основной адрес - используется когда ножка ALT ADDRESS подтянута к 1
	RSCS_ADXL345_ADDR_MAIN = 0x1D,
	// альтернативный адрес - используется когда ножка ALT ADDRESS подтянута к 0
	RSCS_ADXL345_ADDR_ALT = 0x53,
} rscs_adxl345_addr_t;


// параметры пределов измерений
typedef enum
{
	RSCS_ADXL345_RANGE_2G	= 0b00,	// 2G
	RSCS_ADXL345_RANGE_4G	= 0b01,	// 4G
	RSCS_ADXL345_RANGE_8G	= 0b10,	// 8G
	RSCS_ADXL345_RANGE_16G	= 0b11,	// 16G
} rscs_adxl345_range_t;


// параметры частоты измерений
typedef enum
{
	RSCS_ADXL345_RATE_0DOT10HZ	= 0b0000,	// 0.10 Герц
	RSCS_ADXL345_RATE_0DOT20HZ	= 0b0001,	// 0.20 Герц
	RSCS_ADXL345_RATE_0DOT39HZ	= 0b0010,	// 0.39 Герц
	RSCS_ADXL345_RATE_0DOT78HZ	= 0b0011,	// 0.78 Герц
	RSCS_ADXL345_RATE_1DOT56HZ	= 0b0100,	// 1.56 Герц
	RSCS_ADXL345_RATE_3DOT13HZ	= 0b0101,	// 3.13 Герц
	RSCS_ADXL345_RATE_6DOT25HZ	= 0b0110,	// 6.25 Герц
	RSCS_ADXL345_RATE_12DOT5HZ	= 0b0111,	// 12.5 Герц
	RSCS_ADXL345_RATE_25HZ		= 0b1000,	//   25 Герц
	RSCS_ADXL345_RATE_50HZ		= 0b1001,	//   50 Герц
	RSCS_ADXL345_RATE_100HZ		= 0b1010,	//  100 Герц
	RSCS_ADXL345_RATE_200HZ		= 0b1011,	//  200 Герц
	RSCS_ADXL345_RATE_400HZ		= 0b1100,	//  400 Герц
	RSCS_ADXL345_RATE_800HZ		= 0b1101,	//  800 Герц
	// более высокие частоты недоступны в режиме работы через I2C
	RSCS_ADXL345_RATE_1600HZ	= 0b1110,	//  1600 Герц
	RSCS_ADXL345_RATE_3200HZ	= 0b1111,	//  3200 Герц
} rscs_adxl345_rate_t;


rscs_e rscs_adxl345_getRegisterValue(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t * read_data);

// Функции инициализации дескриптора (для разных интерфейсов)
/*	Параметры:
	- addr - адрес устройства на шине. Зависит от значения на ножке акселерометра ALT ADDRESS */
rscs_adxl345_t * rscs_adxl345_initi2c(rscs_i2c_addr_t addr);

// первичная настройка (обязательна к использованию сразу после rscs_adxl345_initi2c())
rscs_e rscs_adxl345_startup(rscs_adxl345_t * adxl);

// освобождение дескриптора
void rscs_adxl345_deinit(rscs_adxl345_t * device);

// установка пределов измерения
/* в случае ошибки взаимодействия по I2C возвращает код ошибки */
rscs_e rscs_adxl345_set_range(rscs_adxl345_t * device, rscs_adxl345_range_t range);

// установка частоты измерений
/* в случае ошибки взаимодействия по I2C возвращает код ошибки */
rscs_e rscs_adxl345_set_rate(rscs_adxl345_t * device, rscs_adxl345_rate_t rate);

// Чтение данных
/*	Это сырые данные в том виде, в каком они прочитанны из регистров акселерометра
	для их перевода в едницы системы измерения СИ требуются специальные формулы
	TODO: ADXL: Результаты зависят от формата данных, который указывается в конфигурационных
	регистрах - определиться с тем, какой используется и написать в этом комментарии
	Параметры:
 	 - x - указатель для прочитанного ускорения по оси X
 	 - y - указатель для прочитанного ускорения по оси Y
 	 - z - указатель для прочитанного ускорения по оси Z

 	 в случае ошибки взаимодействия по I2C возвращает код ошибки
*/
rscs_e rscs_adxl345_read(rscs_adxl345_t * device, int16_t * x, int16_t * y, int16_t * z);

// Перевод сырых данных акселерометра в G
void rscs_adxl345_cast_to_G(rscs_adxl345_t * device, int16_t x, int16_t y, int16_t z, float * x_g, float * y_g, float * z_g);

/* ЧТЕНИЕ ДАННЫХ ADXL345 В БИНАРНОМ ВИДЕ И ПРЕОБРАЗОВАНИЕ В ЕДИНИЦЫ g */
rscs_e rscs_adxl345_GetGXYZ(rscs_adxl345_t * device, int16_t* x, int16_t* y, int16_t* z, float* x_g, float* y_g, float* z_g);


#endif /* RSCS_ADXL345_H_ */

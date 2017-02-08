#ifndef ADXL345_H_
#define ADXL345_H_

#include <stdint.h>

#include "error.h"

/*
	модуль работы с акселерометром ADXL375 через интерфейс I2C
	Даташит: http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
	Неплохое описание на русском: http://www.russianelectronics.ru/leader-r/review/2193/doc/49907/
*/

// дескриптор устройства
struct rscs_adxl345_t;
typedef struct rscs_adxl345_t rscs_adxl345_t;


// Возможные адреса устройства на шине
typedef enum
{
	// основной адрес - используется когда ножка ALT ADDRESS подтянута к 1
	RSCS_ADXL345_ADDR_MAIN = 0x1D,
	// альтернативный адрес - используется когда ножка ALT ADDRESS подтянута к 0
	RSCS_ADXL345_ADDR_ALT  = 0x3A,
} rscs_adxl345_addr_t;


// параметры пределов измерений
typedef enum
{
	RSCS_ADXL345_RANGE_2G,	// 2G
	RSCS_ADXL345_RANGE_4G,	// 4G
	RSCS_ADXL345_RANGE_8G,	// 8G
	RSCS_ADXL345_RANGE_16G,	// 16G
} rscs_adxl345_range_t;


// параметры частоты измерений
typedef enum
{
	RSCS_ADXL345_RATE_0DOT10HZ,	// 0.10 Герц
	RSCS_ADXL345_RATE_0DOT20HZ,	// 0.20  Герц
	RSCS_ADXL345_RATE_0DOT39HZ,	// 0.37  Герц
	RSCS_ADXL345_RATE_0DOT78HZ,	// 0.78  Герц
	RSCS_ADXL345_RATE_1DOT56HZ,	// 1.56  Герц
	RSCS_ADXL345_RATE_3DOT13HZ,	// 3.13  Герц
	RSCS_ADXL345_RATE_6DOT25HZ,	// 6.25  Герц
	RSCS_ADXL345_RATE_12DOT5HZ,	// 12.5 Герц
	RSCS_ADXL345_RATE_25HZ,		//   25 Герц
	RSCS_ADXL345_RATE_50HZ,		//   50 Герц
	RSCS_ADXL345_RATE_100HZ,	//  100 Герц
	RSCS_ADXL345_RATE_200HZ,	//  200 Герц
	RSCS_ADXL345_RATE_400HZ,	//  400 Герц
	RSCS_ADXL345_RATE_800HZ,	//  800 Герц
	// более высокие частоты недоступны в режиме работы через I2C
} rscs_adxl345_rate_t;


// Инициализация дескриптора
/*	Параметры:
	- addr - адрес устройства на шине. Зависит от значения на ножке акселерометра ALT ADDRESS */
rscs_adxl345_t * rscs_adxl345_init(rscs_adxl345_addr_t addr);

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



#endif /* ADXL345_H_ */

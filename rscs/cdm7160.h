/*
 * cdm7160.h
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */
/* Библиотека для работы с инфракрасным датчиком СО2 cdm7160 по шине I2C */

#ifndef CDM7160_H_
#define CDM7160_H_

/* Библиотека для работы с инфракрасным датчиком СО2 cdm7160 по шине I2C */

#include <inttypes.h>

typedef enum{
	RSCS_CDM7160_MODE_CONTINUOUS = 0x06,
	RSCS_CDM7160_MODE_SLEEP = 0x00,
} rscs_cdm7160_mode_t; // Перечисление режимов работы модуля

typedef enum{
	RSCS_CDM7160_ADDR_HIGH,
	RSCS_CDM7160_ADDR_LOW,
} rscs_cdm7160_address_t; // Перечисление возможных адресов


typedef struct {
	uint8_t addr;
} rscs_cdm7160_t; // Объявление типа структуры для хранения дескриптора устройства

// Функция создаёт дескриптор устройства на шине
rscs_cdm7160_t* rscs_cdm7160_init(rscs_cdm7160_address_t);

// Функция осуществляет программный reset устройства
rscs_e rscs_cdm7160_reset(rscs_cdm7160_t*);

// функция выбора режима работы датчика (постоянное измерение/спящий режим)
rscs_e rscs_cdm7160_mode_set(rscs_cdm7160_t*, rscs_cdm7160_mode_t);

// Функция чтения показаний концентрации CO2
rscs_e rscs_cdm7160_read(rscs_cdm7160_t*, uint16_t*);

// Функция коррекции показаний с учётом атмосферного давления
rscs_e rscs_cdm7160_write_pressure_corr(rscs_cdm7160_t*, uint8_t press_coeff);

// Функция коррекции показаний с учётом высоты
rscs_e rscs_cdm7160_write_altitude_corr(rscs_cdm7160_t*, uint8_t alt_coeff);

#endif /* CDM7160_H_ */

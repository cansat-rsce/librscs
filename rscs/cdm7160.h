/*
 * cdm7160.h
 *
 *  Created on: 24 марта 2018 г.
 *      Author: developer
 */
/* Библиотека для работы с инфракрасным датчиком СО2 cdm7160 по шине I2C */

#ifndef CDM7160_H_
#define CDM7160_H_



#endif /* CDM7160_H_ */
/* Библиотека для работы с инфракрасным датчиком СО2 cdm7160 по шине I2C */

rscs_e rscs_cdm7160_reset(void); // функция осуществляет программный reset устройства
rscs_e rscs_cdm7169_init(void); // инициализирует cdm7160

typedef enum{
	CONTINUOUS = 0x06,
	SLEEP = 0x00,
} rscs_cdm7160_mode_t; // Перечисление режимов работы модуля

typedef enum{
	HIGH,
	LOW,
} rscs_cdm7160_address_t; // Перечисление возможных адресов


typedef struct {
	uint8_t addr;
} rscs_cdm7160_sensor_t; // Объявление типа структуры для хранения дескриптора устройства

rscs_e rscs_cdm7160_init(rscs_cdm7160_address_t); // Функция создаёт дескриптор устройства на шине
rscs_e rscs_cdm7160_reset(rscs_cdm7160_sensor_t); // Функция осуществляет программный reset устройства
rscs_e rscs_cdm7160_mode_set(rscs_cdm7160_sensor_t, rscs_cdm7160_mode_t); // функция выбора режима работы датчика (постоянное измерение/спящий режим)
rscs_e rscs_cdm7160_readCO2(rscs_cdm7160_sensor_t); // Функция чтения показаний концентрации CO2
rscs_e rscs_cdm7160_write_pressure_corr(rscs_cdm7160_sensor_t, int pressure); // Функция коррекции показаний с учётом атмосферного давления
rscs_e rscs_cdm7160_write_altitude_corr(rscs_cdm7160_sensor_t, int altitude); // Функция коррекции показаний с учётом высоты


#endif /* CDM7160_H_ */

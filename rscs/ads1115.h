/* */

#ifndef ADS1115_H_
#define ADS1115_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "i2c.h"
#include "error.h"

struct rscs_ads1115_t;
typedef struct rscs_ads1115_t rscs_ads1115_t;

//Перечисление режимов измерения
typedef enum {
	RSCS_ADS1115_CHANNEL_0			= 4, //Волшебные числа - то, что надо записать в регистр
	RSCS_ADS1115_CHANNEL_1			= 5,
	RSCS_ADS1115_CHANNEL_2			= 6,
	RSCS_ADS1115_CHANNEL_3			= 7,
	RSCS_ADS1115_CHANNEL_DIFF_01	= 0,
	RSCS_ADS1115_CHANNEL_DIFF_03	= 1,
	RSCS_ADS1115_CHANNEL_DIFF_13	= 2,
	RSCS_ADS1115_CHANNEL_DIFF_23	= 3,

} rscs_ads1115_channel_t;

//Перечисление режимов измерения
typedef enum {
	RSCS_ADS1115_MODE_HALT 			= 0, // Выключен
	RSCS_ADS1115_MODE_SINGLE 		= 1, // Одно измерение, после чего остановиться
	RSCS_ADS1115_MODE_CONTINIOUS	= 2, // Переодические измерения
} rscs_ads1115_mode_t;

//Перечисление частоты измерений
typedef enum {
	RSCS_ADS1115_DATARATE_8SPS 		= 0, //Волшебные числа - то, что надо записать в регистр
	RSCS_ADS1115_DATARATE_16SPS 	= 1,
	RSCS_ADS1115_DATARATE_32SPS 	= 2,
	RSCS_ADS1115_DATARATE_64SPS 	= 3,
	RSCS_ADS1115_DATARATE_128SPS 	= 4,
	RSCS_ADS1115_DATARATE_250SPS 	= 5,
	RSCS_ADS1115_DATARATE_475SPS 	= 6,
	RSCS_ADS1115_DATARATE_860SPS 	= 7,
} rscs_ads1115_datarate_t;

//Перечисление диапазонов измерений
typedef enum {
	RSCS_ADS1115_RANGE_6DOT144 	= 0, //Волшебные числа - то, что надо записать в регистр
	RSCS_ADS1115_RANGE_4DOT096 	= 1,
	RSCS_ADS1115_RANGE_2DOT048 	= 2,//TODO
	RSCS_ADS1115_RANGE_1DOT024 	= 3,
	RSCS_ADS1115_RANGE_0DOT512 	= 4,
	RSCS_ADS1115_RANGE_0DOT256 	= 5,
} rscs_ads1115_range_t;

//Возможные адреса ацп на шине (в зависимости от подключения ADDR пина)
#define RSCS_ADS1115_ADDR_GND 0x90 //ADDR подключен к GND
#define RSCS_ADS1115_ADDR_VCC 0x91 //ADDR подключен к VCC
#define RSCS_ADS1115_ADDR_SDA 0x92 //ADDR подключен к SDA
#define RSCS_ADS1115_ADDR_SCL 0x93 //ADDR подключен к SCL

// Инициализация датчика.
/* Аргумент - семибитный адрес устройства на I2C шине, который задается перемычками */
rscs_ads1115_t * rscs_ads1115_init(i2c_addr_t addr);

// Деинициализация и освобождение дескритора
void rscs_ads1115_deinit(rscs_ads1115_t * device);

// Изменение канала измерений
rscs_e rscs_ads1115_set_channel(rscs_ads1115_t * device, rscs_ads1115_channel_t channel);

// Изменение диапазона
rscs_e rscs_ads1115_set_range(rscs_ads1115_t * device, rscs_ads1115_range_t range);

// Изменение  измерений
rscs_e rscs_ads1115_set_datarate(rscs_ads1115_t * device, rscs_ads1115_datarate_t datarate);

// Изменение режима измерений
rscs_e rscs_ads1115_changemode(rscs_ads1115_t * device, rscs_ads1115_mode_t mode);

// Чтение данных из выбранного в rscs_ads1115_set_channel() канала устройства
/* Параметры:
   - device - дескриптор датчика
   - value - указатель на переменную, в которую следует разместить результат измерения */
rscs_e rscs_ads1115_read(rscs_ads1115_t * device, uint16_t * value);

//TODO в будущем возможна реализация функциональности компаратора

#endif /* ADS1115_H_ */

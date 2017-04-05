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
	RSCS_ADS1115_RANGE_2DOT048 	= 2,
	RSCS_ADS1115_RANGE_1DOT024 	= 3,
	RSCS_ADS1115_RANGE_0DOT512 	= 4,
	RSCS_ADS1115_RANGE_0DOT256 	= 5,
} rscs_ads1115_range_t;

//Возможные адреса ацп на шине (в зависимости от подключения ADDR пина)
#define RSCS_ADS1115_ADDR_GND 0x48 //ADDR подключен к GND
#define RSCS_ADS1115_ADDR_VCC 0x49 //ADDR подключен к VCC
#define RSCS_ADS1115_ADDR_SDA 0x50 //ADDR подключен к SDA
#define RSCS_ADS1115_ADDR_SCL 0x51 //ADDR подключен к SCL

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

// Начало одиночного измерения
// Выдаст RSCS_E_BUSY, если есть измерение в процессе
rscs_e rscs_ads1115_start_single(rscs_ads1115_t * device);

// Начало повторяющихся измерений
// Выдаст RSCS_E_BUSY, если есть измерение в процессе
rscs_e rscs_ads1115_start_continuous(rscs_ads1115_t * device);

// Конец повторяющихся измерений
rscs_e rscs_ads1115_stop_continuous(rscs_ads1115_t * device);

// Чтение данных, полученных после последнего измерения
/* Параметры:
   - device - дескриптор датчика
   - value - указатель на переменную, в которую следует разместить результат измерения */
rscs_e rscs_ads1115_read(rscs_ads1115_t * device, int16_t * value);

// Дождаться окончания измерения
// (имеет смысл только в одиночном режиме, в режиме множества измеерний сразу закончит ждать)
rscs_e rscs_ads1115_wait_result(rscs_ads1115_t * device);

//TODO в будущем возможна реализация функциональности компаратора

#endif /* ADS1115_H_ */

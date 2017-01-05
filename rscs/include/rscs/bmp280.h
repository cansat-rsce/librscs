#ifndef BMP280_H_
#define BMP280_H_

#include "i2c.h"

#define BMP280_ADDR_LOW 0xEC
#define BMP280_ADDR_HIGH 0xEE


//Количество измерений на один результат. Выставляется отдельно для термометра и барометра
typedef enum {
	RSCS_BMP280_OVERSAMPLING_OFF 	= 0,
	RSCS_BMP280_OVERSAMPLING_X1 	= 1,
	RSCS_BMP280_OVERSAMPLING_X2 	= 2,
	RSCS_BMP280_OVERSAMPLING_X4 	= 3,
	RSCS_BMP280_OVERSAMPLING_X8 	= 4,
	RSCS_BMP280_OVERSAMPLING_X16 	= 5

} rscs_bmp280_oversampling_t;

//Время между двумя измерениями в непрерывном режиме
typedef enum {
	RSCS_BMP280_STANDBYTIME_500US 	= 0,
	RSCS_BMP280_STANDBYTIME_63MS 	= 1,
	RSCS_BMP280_STANDBYTIME_125MS 	= 2,
	RSCS_BMP280_STANDBYTIME_250MS 	= 3,
	RSCS_BMP280_STANDBYTIME_500MS 	= 4,
	RSCS_BMP280_STANDBYTIME_1S 		= 5,
	RSCS_BMP280_STANDBYTIME_2S 		= 6,
	RSCS_BMP280_STANDBYTIME_4S 		= 7,
} rscs_bmp280_standbytime_t;

/*Режимы работы.
 * SLEEP - сон, FORCED - одиночное измерение по команде, NORMAL - непрерывное измерение */
typedef enum {
	RSCS_BMP280_MODE_SLEEP 	= 0,
	RSCS_BMP280_MODE_FORCED = 1,
	RSCS_BMP280_MODE_NORMAL	= 3,
} rscs_bmp280_mode_t;

typedef enum {
	RSCS_BMP280_FILTER_OFF 	= 0,
	RSCS_BMP280_FILTER_X2 	= 2,
	RSCS_BMP280_FILTER_X4 	= 4,
	RSCS_BMP280_FILTER_X8 	= 8,
	RSCS_BMP280_FILTER_X16 	= 16,
} rscs_bmp280_filter_t;

//Калибровочные значения
typedef struct {
	uint16_t T[3], P[9];
} rscs_bmp280_calibration_values_t;

/*Дескриптор датчика. Все поля, кроме mode, заполняются пользователем в самом дескрипторе
 *перед вызовом rscs_bmp280_init()
 *Поле mode заполняется rscs_bmp280_changemode()*/
typedef struct {
	rscs_i2c_bus_t i2c;
	i2c_addr_t address;
	rscs_bmp280_oversampling_t pressure_oversampling;
	rscs_bmp280_oversampling_t temperature_oversampling;
	rscs_bmp280_standbytime_t standbytyme;
	rscs_bmp280_filter_t filter;
	rscs_bmp280_mode_t mode;
	rscs_bmp280_calibration_values_t calibration_values;
	double * pressure, temperature;

} rscs_bmp280_descriptor_t;

//Инилиализация датчика
rscs_e rscs_bmp280_init(rscs_bmp280_descriptor_t * bmp);

//Изменение режима работы. Для начала одиночного измерения преведите в FORCE режим
rscs_e rscs_bmp280_changemode(rscs_bmp280_descriptor_t * bmp, rscs_bmp280_mode mode);

//Чтение данных из BMP с последующими рассчётами значений и записью по указателям в дескрипторе
rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * bmp);

#endif /* BMP280_H_ */

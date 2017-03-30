#ifndef BMP280_H_
#define BMP280_H_

#include "error.h"

//Адреса регистров устройства
#define RSCS_BMP280_REG_CALVAL_START 0x88
#define RSCS_BMP280_REG_ID 0xD0
#define RSCS_BMP280_REG_RESET 0xE0
#define RSCS_BMP280_REG_STATUS 0xF3
#define RSCS_BMP280_REG_CTRL_MEAS 0xF4
#define RSCS_BMP280_REG_CONFIG 0xF5
#define RSCS_BMP280_REG_PRESS_MSB 0xF7
#define RSCS_BMP280_REG_PRESS_LSB 0xF8
#define RSCS_BMP280_REG_PRESS_XLSB 0xF9
#define RSCS_BMP280_REG_TEMP_MSB 0xFA
#define RSCS_BMP280_REG_TEMP_LSB 0xFB
#define RSCS_BMP280_REG_TEMP_XLSB 0xFC

//Правильное содержимое регистра ID
#define RSCS_BMP280_IDCODE 0x58

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
	RSCS_BMP280_STANDBYTIME_500US 		= 0,
	RSCS_BMP280_STANDBYTIME_62DOT5MS 	= 1,
	RSCS_BMP280_STANDBYTIME_125MS 		= 2,
	RSCS_BMP280_STANDBYTIME_250MS 		= 3,
	RSCS_BMP280_STANDBYTIME_500MS 		= 4,
	RSCS_BMP280_STANDBYTIME_1S 			= 5,
	RSCS_BMP280_STANDBYTIME_2S 			= 6,
	RSCS_BMP280_STANDBYTIME_4S 			= 7,
} rscs_bmp280_standbytime_t;

/*Режимы работы.
 * SLEEP - сон, FORCED - одиночное измерение по команде, NORMAL - непрерывное измерение */
typedef enum {
	RSCS_BMP280_MODE_SLEEP 	= 0,
	RSCS_BMP280_MODE_FORCED = 1,
	RSCS_BMP280_MODE_NORMAL	= 3,
} rscs_bmp280_mode_t;

//Режимы фильтрованных измерений (см. даташит)
typedef enum {
	RSCS_BMP280_FILTER_OFF 	= 0,
	RSCS_BMP280_FILTER_X2 	= 1,
	RSCS_BMP280_FILTER_X4 	= 2,
	RSCS_BMP280_FILTER_X8 	= 3,
	RSCS_BMP280_FILTER_X16 	= 4,
} rscs_bmp280_filter_t;

//Калибровочные значения
typedef struct {
	uint16_t T1;
	int16_t T2, T3;
	uint16_t P1;
	int16_t P2, P3, P4, P5, P6, P7, P8, P9;
} rscs_bmp280_calibration_values_t;

/*Параметры датчика. Все поля заполняются пользователем в самой структуре
 *перед вызовом rscs_bmp280_init() */
/* Рекомендуемые параметры
 * ===================================================================
 * |     Use case       |  Mode  | osrs_p | osrs_t | filter | timing |
 * ===================================================================
 * |  Handheld device   | Normal |   x16  |   x2   |    4   | 62.5ms |
 * |     (low-power)    |        |        |        |        |        |
 * -------------------------------------------------------------------
 * |  Handheld device   | Normal |   x4   |   x1   |   16   |  0.5ms |
 * |      (dynamic)     |        |        |        |        |        |
 * -------------------------------------------------------------------
 * | Weather monitoring | Forced |   x1   |   x1   |   Off  |  1/min |
 * |   (lowest power)   |        |        |        |        |        |
 * -------------------------------------------------------------------
 * |   Elevator/floor   | Normal |   x4   |   x1   |    4   |  125ms |
 * |  change detection  |        |        |        |        |        |
 * -------------------------------------------------------------------
 * |   Drop detection   | Normal |   x2   |   x1   |   Off  |  0.5ms |
 * -------------------------------------------------------------------
 * |  Indoor navigation | Normal |   x16  |   x2   |   16   |  0.5ms |
 * ===================================================================
 * osrs_t - количество измерений на один результат для температуры.
 * osrs_p - количество измерений на один результат для давления.
 * */
typedef struct {
	// Режим префильтрации измерений давления
	rscs_bmp280_oversampling_t pressure_oversampling;
	// Режим префильтрации измерений температуры
	rscs_bmp280_oversampling_t temperature_oversampling;
	// Период непрерывных измерений
	rscs_bmp280_standbytime_t standbytyme;
	// Режим фильтрации
	rscs_bmp280_filter_t filter;
} rscs_bmp280_parameters_t;

//Дескриптор датчика, содержимое недоступно извне
struct rscs_bmp280_descriptor;
typedef struct rscs_bmp280_descriptor rscs_bmp280_descriptor_t;

// Создание дескриптора датчика
// Не инициализирует сам датчик.
rscs_bmp280_descriptor_t * rscs_bmp280_init();

// Освобождение дескритора датчика
void rscs_bmp280_deinit(rscs_bmp280_descriptor_t * descr);

// Инилиализация датчика
// загрузка калибровочных коээфициентов и передача параметров конфигурации
rscs_e rscs_bmp280_setup(rscs_bmp280_descriptor_t * descr, const rscs_bmp280_parameters_t * params);

// Возвращает указатель не текущую конфигурацию датчика
/* Подразумвается сохраненная на МК копия конфигурации, а не та, что
   хранится в самом датчике. Указатель действителен до тех пор, пока не будет
   вызвана rscs_bmp280_deinit для этого дескриптора */
const rscs_bmp280_parameters_t * rscs_bmp280_get_config(rscs_bmp280_descriptor_t * descr);

//Отправляет датчику и сохраняет локально новые настройки params
rscs_e rscs_bmp280_set_config(rscs_bmp280_descriptor_t * descr, rscs_bmp280_parameters_t * params);

// Возвращает указатель на значения калибровоных коэффициентов датчика
/*  Указатель действителен до тех пор, пока не будет
   вызвана rscs_bmp280_deinit для этого дескриптора */
const rscs_bmp280_calibration_values_t * rscs_bmp280_get_calibration_values(rscs_bmp280_descriptor_t * descr);

//Изменение режима работы. Для начала одиночного измерения переведите в FORCE режим
rscs_e rscs_bmp280_changemode(rscs_bmp280_descriptor_t * bmp, rscs_bmp280_mode_t mode);

//Чтение данных из BMP(в сыром виде)
rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * bmp, int32_t * rawpress, int32_t * rawtemp);

/*Рассчёт давления и температуры из сырых значений.
 *WARNING: на ATMega очень медленно и неточно, лучше считать на земле
 *Rawpress и rawtemp - сырые данные, press_p и temp_p - указатели на переменные, куда будут
 *записаны давление и температура, выраженные в паскалях и сотых долях градуса соответственно.
 *Может вернуть RSCS_E_NULL, если произошла ошибка деления на ноль*/
rscs_e rscs_bmp280_calculate(const rscs_bmp280_calibration_values_t * calvals , int32_t rawpress, int32_t rawtemp, int32_t * press_p, int32_t * temp_p);

//Прочитать регистр статуса устройства
uint8_t rscs_bmp280_read_status(rscs_bmp280_descriptor_t * bmp);

#endif /* BMP280_H_ */

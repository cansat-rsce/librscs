#include "../include/rscs/bmp280.h"

/*Дескриптор датчика.
 *Поле mode заполняется rscs_bmp280_changemode()*/
struct rscs_bmp280_descriptor {
	// Шина, на которой датчик доступен
	rscs_i2c_bus_t * i2c;
	// Адрес датчика на шине - RSCS_BMP280_ADDR_LOW или RSCS_BMP280_ADDR_HIGH
	i2c_addr_t address;
	rscs_bmp280_parameters_t parameters;
	rscs_bmp280_calibration_values_t calibration_values;
	// Режим работы - непрерывный, одиночный, ожидания
	rscs_bmp280_mode_t mode;
};




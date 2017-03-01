#include <stdlib.h>
#include <stdbool.h>

#include "../bmp280.h"

#include "../i2c.h"
#include "../spi.h"

#include "librscs_config.h"

//Далее определены макросы для удобного написания кода, все они #undef в конце файла

//Макрос для возможности обработки ошибок
#define OPERATION(OP) error = OP if(error != RSCS_E_NONE) goto end;

//Макрос инициализации SPI
#define INITSPI \
	RSCS_BMP280_CSDDR |= (1 << RSCS_BMP280_CSPIN); \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN); \
	rscs_spi_init();

//Макрос чтения регистров по SPI
#define READREGSPI(REG, DATA, COUNT) \
	uint8_t * data = (uint8_t *) DATA; \
	RSCS_BMP280_CSPORT &= ~(1 << RSCS_BMP280_CSPIN); \
	for(int i = 0; i < COUNT; i++) { \
		OPERATION(	rscs_spi_do( (REG + i) | (1 << 8) ); ) \
		OPERATION(	data[i] = rscs_spi_do(0xFF); ) \
	} \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN);

//Макрос записи регистров по SPI
#define WRITEREGSPI(REG, DATA, COUNT) \
	uint8_t * data = (uint8_t *) DATA; \
	RSCS_BMP280_CSPORT &= ~(1 << RSCS_BMP280_CSPIN); \
	for(int i = 0; i < COUNT; i++) { \
		OPERATION(	rscs_spi_do( (REG + i) & ~(1 << 8) ); ) \
		OPERATION(	rscs_spi_do(data[i]); ) \
	} \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN);

//Выбор используемых макросов в зависимости от выбранного интерфейса
#if RSCS_BMP280_IF == SPI

#define IFINIT INITSPI
#define READREG(REG, DATA, COUNT) READREGSPI(REG, DATA, COUNT)
#define WRITEREG(REG, DATA, COUNT) WRITEREGSPI(REG, DATA, COUNT)

#elif RSCS_BMP280_IF == I2C

#error "BMP280: не написан обмен по I2C"

#else

#error "BMP280: некорректное значение интерфейса"

#endif

/*Дескриптор датчика.
 *Поле mode заполняется rscs_bmp280_changemode()*/
struct rscs_bmp280_descriptor {
	// Адрес датчика на шине - RSCS_BMP280_ADDR_LOW или RSCS_BMP280_ADDR_HIGH
	i2c_addr_t address;
	rscs_bmp280_parameters_t parameters;
	rscs_bmp280_calibration_values_t calibration_values;
	// Режим работы - непрерывный, одиночный, ожидания
	rscs_bmp280_mode_t mode;
};
//TODO подумать над названием
rscs_bmp280_descriptor_t * rscs_bmp280_init(i2c_addr_t address){
	rscs_bmp280_descriptor_t * pointer = (rscs_bmp280_descriptor_t *) malloc(sizeof(rscs_bmp280_descriptor_t));
	pointer->address = address;
	return pointer;
}

void rscs_bmp280_deinit(rscs_bmp280_descriptor_t * descr){
	free(descr);
}

rscs_e rscs_bmp280_setup(rscs_bmp280_descriptor_t * descr, const rscs_bmp280_parameters_t * params){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp[2];

	IFINIT
	READREG(RSCS_BMP280_REG_ID, tmp, 1)

	if(tmp[0] != RSCS_BMP280_IDCODE) return RSCS_E_INVRESP;

	READREG(RSCS_BMP280_REG_CALVAL_START, &(descr->calibration_values), sizeof(descr->calibration_values))
	rscs_i2c_stop();


	tmp[0] = 	(params->temperature_oversampling << 5) |
				(params->pressure_oversampling << 2);
	tmp[1] = 	(params->standbytyme << 5) |
				(params->filter << 2);
	WRITEREG(RSCS_BMP280_REG_CTRL_MEAS, tmp, 2)

	descr->parameters = *params;

end:
	rscs_i2c_stop();
	return error;
}

const rscs_bmp280_parameters_t * rscs_bmp280_get_config(rscs_bmp280_descriptor_t * descr){
	return &descr->parameters;
}

const rscs_bmp280_calibration_values_t * rscs_bmp280_get_calibration_values(rscs_bmp280_descriptor_t * descr){
	return &descr->calibration_values;
}

rscs_e rscs_bmp280_changemode(rscs_bmp280_descriptor_t * bmp, rscs_bmp280_mode_t mode){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp = (	bmp->parameters.temperature_oversampling << 5) |
			(bmp->parameters.pressure_oversampling << 2) |
			mode;

	WRITEREG(RSCS_BMP280_REG_CTRL_MEAS, &tmp, 1)

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * bmp, uint32_t * rawpress, uint32_t * rawtemp){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp[6];

	READREG(RSCS_BMP280_REG_PRESS_MSB, tmp, 6)
	*rawpress = (tmp[0] << 12) | (tmp[1] << 4) | (tmp[2] >> 4);
	*rawtemp = (tmp[3] << 12) | (tmp[4] << 4) | (tmp[5] >> 4);

end:
	rscs_i2c_stop();
	return error;
}

#undef OPERATION

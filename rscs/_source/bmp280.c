#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <util/delay.h>

#include "../bmp280.h"

#include "../error.h"
#include "../i2c.h"
#include "../spi.h"

#include "librscs_config.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label" /* Игнорируем ворнинг неиспользуемого лейбла,
чтобы не было лишних ворнингов на end: при использовании интерфейсов без обработки ошибок*/

//Далее определены макросы для удобного написания кода, все они #undef в конце файла

//Макрос для возможности обработки ошибок
#define OPERATION(OP) error = OP if(error != RSCS_E_NONE) goto end;

//Макрос инициализации SPI
#define INITSPI \
	RSCS_BMP280_CSDDR |= (1 << RSCS_BMP280_CSPIN); \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN); \
	rscs_spi_init();\
	rscs_spi_set_clk(RSCS_BMP280_SPI_FREQ_kHz); \
	rscs_spi_set_pol(RSCS_SPI_POL_SETUP_FALL_SAMPLE_RISE); \
 	rscs_spi_set_order(RSCS_SPI_ORDER_MSB_FIRST);

//Макрос чтения регистров по SPI
#define READREGSPI(REG, DATA, COUNT) \
	RSCS_BMP280_CSPORT &= ~(1 << RSCS_BMP280_CSPIN);\
	rscs_spi_do( (uint8_t) (REG | (1 << 7)) ); \
	for(int i = 0; i < COUNT; i++) { \
		((uint8_t *)DATA)[i] = rscs_spi_do(0xFF); \
	} \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN);

//Макрос записи регистров по SPI
#define WRITEREGSPI(REG, DATA, COUNT) \
	RSCS_BMP280_CSPORT &= ~(1 << RSCS_BMP280_CSPIN);\
	for(int i = 0; i < COUNT; i++) { \
		RSCS_DEBUG("BMP280: WRITEREG sending addr\n"); \
		rscs_spi_do( ((REG + i) & ~(1 << 7)) ); \
		RSCS_DEBUG("BMP280: WRITEREG sending data\n"); \
		rscs_spi_do((((uint8_t *)DATA)[i])); \
	} \
	RSCS_BMP280_CSPORT |= (1 << RSCS_BMP280_CSPIN);

//Выбор используемых макросов в зависимости от выбранного интерфейса
#if RSCS_BMP280_IF == RSCS_IF_SPI

#define IFINIT INITSPI
#define READREG(REG, DATA, COUNT) READREGSPI(REG, DATA, COUNT)
#define WRITEREG(REG, DATA, COUNT) WRITEREGSPI(REG, DATA, COUNT)

#elif RSCS_BMP280_IF == RSCS_IF_I2C

#error "BMP280: не написан обмен по I2C"

#else

#error "BMP280: некорректное значение интерфейса"

#endif

/*Дескриптор датчика.
 *Поле mode заполняется rscs_bmp280_changemode()*/
struct rscs_bmp280_descriptor {
	rscs_bmp280_parameters_t parameters;
	rscs_bmp280_calibration_values_t calibration_values;
	// Режим работы - непрерывный, одиночный, ожидания
	rscs_bmp280_mode_t mode;
};

rscs_bmp280_descriptor_t * rscs_bmp280_init(){
	rscs_bmp280_descriptor_t * pointer = (rscs_bmp280_descriptor_t *) malloc(sizeof(rscs_bmp280_descriptor_t));
	return pointer;
}

void rscs_bmp280_deinit(rscs_bmp280_descriptor_t * descr){
	free(descr);
}

rscs_e rscs_bmp280_setup(rscs_bmp280_descriptor_t * descr, const rscs_bmp280_parameters_t * params){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp[2] = {231, 123};

#ifdef RSCS_DEBUGMODE
	for(int i = 0; i < sizeof(descr->calibration_values); i++) {
		*( ( (uint8_t *) &descr->calibration_values) + i ) = 237;
	}
#endif

	RSCS_DEBUG("BMP280: SETUP: trying to IFINIT\n");
	IFINIT

	_delay_ms(50);

	RSCS_DEBUG("BMP280: SETUP: trying to read ID reg\n");
	READREG(RSCS_BMP280_REG_ID, tmp, 1)
	RSCS_DEBUG("BMP280: returned IDCODE 0x%X\n", tmp[0]);
	if(tmp[0] != RSCS_BMP280_IDCODE) {
		return RSCS_E_INVRESP;
	}

	RSCS_DEBUG("BMP280: SETUP: trying to reset\n");
	tmp[0] = 0xb6;  // специальное значение, которое сбрасывает датчик
	WRITEREG(RSCS_BMP280_REG_RESET, tmp, 1);

	_delay_ms(50);

	READREG(RSCS_BMP280_REG_CALVAL_START, &(descr->calibration_values), sizeof(descr->calibration_values))
#ifdef RSCS_DEBUGMODE
	RSCS_DEBUG("BMP280: calvals: ");
	for(int i = 0; i < sizeof(descr->calibration_values); i++) {
		RSCS_DEBUG("%d ", *( ( (uint8_t *) &descr->calibration_values) + i ) );
	}
	RSCS_DEBUG("\n");
#endif

	tmp[0] = 	(params->temperature_oversampling << 5) |
				(params->pressure_oversampling << 2);
	tmp[1] = 	(params->standbytyme << 5) |
				(params->filter << 2);
	WRITEREG(RSCS_BMP280_REG_CTRL_MEAS, tmp, 2)

	descr->parameters = *params;

end:
	RSCS_DEBUG("BMP280: SETUP: returning %d\n", error);
	return error;
}

const rscs_bmp280_parameters_t * rscs_bmp280_get_config(rscs_bmp280_descriptor_t * descr){
	return &descr->parameters;
}

rscs_e rscs_bmp280_set_config(rscs_bmp280_descriptor_t * descr, rscs_bmp280_parameters_t * params) {
	rscs_e error = RSCS_E_NONE;

	uint8_t tmp[2];

	tmp[0] = 	(params->temperature_oversampling << 5) |
				(params->pressure_oversampling << 2);
	tmp[1] = 	(params->standbytyme << 5) |
				(params->filter << 2);
	WRITEREG(RSCS_BMP280_REG_CTRL_MEAS, tmp, 2)

	descr->parameters = *params;

end:
	return error;
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
	RSCS_DEBUG("BMP280: CHMODE: returning %d\n", error);
	return error;
}

rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * bmp, int32_t * rawpress, int32_t * rawtemp){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp[6];

	READREG(RSCS_BMP280_REG_PRESS_MSB, tmp, 6)
	*rawpress = ((uint32_t)tmp[0] << 12) | ((uint32_t)tmp[1] << 4) | ((uint32_t)tmp[2] >> 4);
	*rawtemp = ((uint32_t)tmp[3] << 12) | ((uint32_t)tmp[4] << 4) | ((uint32_t)tmp[5] >> 4);

end:
	RSCS_DEBUG("BMP280: READ: returning %d\n", error);
	return error;
}

uint8_t rscs_bmp280_read_status(rscs_bmp280_descriptor_t * bmp) {
	rscs_e error = RSCS_E_NONE;
	uint8_t status = 255;

	READREG(RSCS_BMP280_REG_STATUS, &status, 1)


end:
	RSCS_DEBUG("BMP280: READ: returning %d\n", error);
	return status;
}

rscs_e rscs_bmp280_calculate(const rscs_bmp280_calibration_values_t * calvals , int32_t rawpress, int32_t rawtemp, int32_t * press_p, int32_t * temp_p) {

	int32_t t_fine;
	{
		int32_t var1, var2;
		var1 = ((((rawtemp >> 3) - (((int32_t)calvals->T1) << 1))) * ((int32_t)calvals->T2)) >> 11;
		var2 = (((((rawtemp >> 4) - ((int32_t)calvals->T1)) * ((rawtemp>>4) - ((int32_t)calvals->T1))) >> 12) * ((int32_t)calvals->T3)) >> 14;
		t_fine = var1 + var2;
		*temp_p = (t_fine * 5 + 128) >> 8;
	}

	{
		int32_t var1_p, var2_p;
		uint32_t p;
		var1_p = (((int32_t)t_fine)>>1) - (int32_t)64000;
		var2_p = (((var1_p>>2) * (var1_p>>2)) >> 11 ) * ((int32_t)calvals->P6);
		var2_p = var2_p + ((var1_p*((int32_t)calvals->P5))<<1);
		var2_p = (var2_p>>2)+(((int32_t)calvals->P4)<<16);
		var1_p = (((calvals->P3 * (((var1_p>>2) * (var1_p>>2)) >> 13 )) >> 3) + ((((int32_t)calvals->P2) * var1_p)>>1))>>18;
		var1_p =((((32768+var1_p))*((int32_t)calvals->P1))>>15);
		if (var1_p == 0)
		{
		return RSCS_E_NULL; // чтобы не делить на ноль
		}
		p = (((uint32_t)(((int32_t)1048576)- rawpress)-(var2_p>>12)))*3125;
		if (p < 0x80000000)
		{
		p = (p << 1) / ((uint32_t)var1_p);
		}
		else
		{
		p = (p / (uint32_t)var1_p) * 2;
		}
		var1_p = (((int32_t)calvals->P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
		var2_p = (((int32_t)(p>>2)) * ((int32_t)calvals->P8))>>13;
		p = (uint32_t)((int32_t)p + ((var1_p + var2_p + calvals->P7) >> 4));
		*press_p = p;
	}

	return RSCS_E_NONE;
}

#undef OPERATION
#pragma GCC diagnostic pop

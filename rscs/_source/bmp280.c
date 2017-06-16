#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <util/delay.h>

#include "../bmp280.h"

#include "../error.h"
#include "../i2c.h"
#include "../spi.h"

#include "librscs_config.h"

/*Дескриптор датчика.
 *Поле mode заполняется rscs_bmp280_changemode()*/
struct rscs_bmp280_descriptor {
	rscs_bmp280_parameters_t parameters;
	rscs_bmp280_calibration_values_t calibration_values;
	// Режим работы - непрерывный, одиночный, ожидания
	rscs_bmp280_mode_t mode;
	rscs_e (*read_reg)(rscs_bmp280_descriptor_t * /*descr*/, uint8_t /*reg_addr*/, void * /*buffer*/, size_t /*buffer_size*/);
	rscs_e (*write_reg)(rscs_bmp280_descriptor_t * /*descr*/, uint8_t /*reg_addr*/, const void * /*buffer*/, size_t /*buffer_size*/);

	union
	{
		struct
		{
			rscs_bmp280_addr_t addr;
		} i2c;
		struct
		{
			volatile uint8_t * cs_port;
			uint8_t cs_pin_mask;
		} spi;
	} bus_params;
};

//Макрос для возможности обработки ошибок
#define OPERATION(OP) error = OP; if(error != RSCS_E_NONE) goto end;

static rscs_e _read_reg_spi(rscs_bmp280_descriptor_t * descr, uint8_t reg_addr,
		void * buffer, size_t buffer_size)
{
	*descr->bus_params.spi.cs_port &= ~(descr->bus_params.spi.cs_pin_mask);

	rscs_spi_write(&reg_addr,1);
	rscs_spi_read(buffer, buffer_size, 0xFF);

	*descr->bus_params.spi.cs_port |= (descr->bus_params.spi.cs_pin_mask);
	return RSCS_E_NONE;
}

static rscs_e _write_reg_spi(rscs_bmp280_descriptor_t * descr, uint8_t reg_addr,
		const void * buffer, size_t buffer_size)
{
	*descr->bus_params.spi.cs_port &= ~(descr->bus_params.spi.cs_pin_mask);

	rscs_spi_write(&reg_addr,1);
	rscs_spi_write(buffer,buffer_size);

	*descr->bus_params.spi.cs_port |= (descr->bus_params.spi.cs_pin_mask);
	return RSCS_E_NONE;
}


static rscs_e _read_reg_i2c(rscs_bmp280_descriptor_t * descr, uint8_t reg_addr, void * buffer, size_t buffer_size)
{
	rscs_e error = RSCS_E_NONE;

	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(descr->bus_params.i2c.addr, rscs_i2c_slaw_write));
	OPERATION(rscs_i2c_write_byte(reg_addr));
	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(descr->bus_params.i2c.addr, rscs_i2c_slaw_read));
	OPERATION(rscs_i2c_read(buffer, buffer_size, true));

end:
	rscs_i2c_stop();
	return error;
}


static rscs_e _write_reg_i2c(rscs_bmp280_descriptor_t * descr, uint8_t reg_addr, const void * buffer, size_t buffer_size)
{
	rscs_e error = RSCS_E_NONE;
	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(descr->bus_params.i2c.addr, rscs_i2c_slaw_write));
	OPERATION(rscs_i2c_write_byte(reg_addr));
	OPERATION(rscs_i2c_write(buffer, buffer_size));

end:
	rscs_i2c_stop();
	return error;
}



rscs_bmp280_descriptor_t * rscs_bmp280_initi2c(rscs_bmp280_addr_t addr){
	rscs_bmp280_descriptor_t * pointer = (rscs_bmp280_descriptor_t *) malloc(sizeof(rscs_bmp280_descriptor_t));
	if (pointer == NULL)
		return NULL;

	pointer->bus_params.i2c.addr = addr;
	pointer->read_reg = _read_reg_i2c;
	pointer->write_reg = _write_reg_i2c;
	return pointer;
}


rscs_bmp280_descriptor_t * rscs_bmp280_initspi(volatile uint8_t * cs_port,
		volatile uint8_t * cs_ddr, uint8_t pin_n)
{
	rscs_bmp280_descriptor_t * pointer = (rscs_bmp280_descriptor_t *) malloc(sizeof(rscs_bmp280_descriptor_t));
	if (pointer == NULL)
		return NULL;

	pointer->bus_params.spi.cs_port = cs_port;
	pointer->bus_params.spi.cs_pin_mask = (1 << pin_n);
	pointer->read_reg = _read_reg_spi;
	pointer->write_reg = _write_reg_spi;
	*cs_ddr |= pointer->bus_params.spi.cs_pin_mask;
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

	_delay_ms(50);

	RSCS_DEBUG("BMP280: SETUP: trying to read ID reg\n");
	OPERATION(descr->read_reg(descr, RSCS_BMP280_REG_ID, tmp, 1));
	RSCS_DEBUG("BMP280: returned IDCODE 0x%X\n", tmp[0]);
	if(tmp[0] != RSCS_BMP280_IDCODE) {
		return RSCS_E_INVRESP;
	}

	RSCS_DEBUG("BMP280: SETUP: trying to reset\n");
	tmp[0] = 0xb6;  // специальное значение, которое сбрасывает датчик
	OPERATION(descr->write_reg(descr, RSCS_BMP280_REG_RESET, tmp, 1));

	_delay_ms(50);

	OPERATION(descr->read_reg(descr, RSCS_BMP280_REG_CALVAL_START, &(descr->calibration_values), sizeof(descr->calibration_values)));
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
	OPERATION(descr->write_reg(descr, RSCS_BMP280_REG_CTRL_MEAS, tmp, 2));

	descr->parameters = *params;

end:
	RSCS_DEBUG("BMP280: SETUP: returning %d\n", error);
	return error;
}


const rscs_bmp280_parameters_t * rscs_bmp280_get_config(rscs_bmp280_descriptor_t * descr){
	return &descr->parameters;
}


rscs_e rscs_bmp280_set_config(rscs_bmp280_descriptor_t * descr, const rscs_bmp280_parameters_t * params) {
	rscs_e error = RSCS_E_NONE;

	uint8_t tmp[2];

	tmp[0] = 	(params->temperature_oversampling << 5) |
				(params->pressure_oversampling << 2);
	tmp[1] = 	(params->standbytyme << 5) |
				(params->filter << 2);
	OPERATION(descr->write_reg(descr, RSCS_BMP280_REG_CTRL_MEAS, tmp, 2));

	descr->parameters = *params;

end:
	return error;
}


const rscs_bmp280_calibration_values_t * rscs_bmp280_get_calibration_values(rscs_bmp280_descriptor_t * descr){
	return &descr->calibration_values;
}


rscs_e rscs_bmp280_changemode(rscs_bmp280_descriptor_t * descr, rscs_bmp280_mode_t mode){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp = (	descr->parameters.temperature_oversampling << 5) |
			(descr->parameters.pressure_oversampling << 2) |
			mode;

	OPERATION(descr->write_reg(descr, RSCS_BMP280_REG_CTRL_MEAS, &tmp, 1));

end:
	RSCS_DEBUG("BMP280: CHMODE: returning %d\n", error);
	return error;
}


rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * descr, int32_t * rawpress, int32_t * rawtemp){
	rscs_e error = RSCS_E_NONE;
	uint8_t tmp[6];

	OPERATION(descr->read_reg(descr, RSCS_BMP280_REG_PRESS_MSB, tmp, 6));
	*rawpress = ((uint32_t)tmp[0] << 12) | ((uint32_t)tmp[1] << 4) | ((uint32_t)tmp[2] >> 4);
	*rawtemp = ((uint32_t)tmp[3] << 12) | ((uint32_t)tmp[4] << 4) | ((uint32_t)tmp[5] >> 4);

end:
	RSCS_DEBUG("BMP280: READ: returning %d\n", error);
	return error;
}


uint8_t rscs_bmp280_read_status(rscs_bmp280_descriptor_t * descr) {
	rscs_e error = RSCS_E_NONE;
	uint8_t status = 255;
	OPERATION(descr->read_reg(descr, RSCS_BMP280_REG_STATUS, &status, 1));

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


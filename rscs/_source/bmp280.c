#include <stdlib.h>
#include <stdbool.h>

#include "../bmp280.h"

#include "../i2c.h"

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

rscs_bmp280_descriptor_t * rscs_bmp280_init(i2c_addr_t address){
	rscs_bmp280_descriptor_t * pointer = (rscs_bmp280_descriptor_t *) malloc(sizeof(rscs_bmp280_descriptor_t));
	pointer->address = address;
	return pointer;
}

void rscs_bmp280_deinit(rscs_bmp280_descriptor_t * descr){
	free(descr);
}

rscs_e rscs_bmp280_setup(rscs_bmp280_descriptor_t * descr, const rscs_bmp280_parameters_t * params){
	//TODO: реализовать обработку ошибок
	uint8_t tmp;
	rscs_i2c_init();
	rscs_i2c_start();
	rscs_i2c_send_slaw(descr->address, rscs_i2c_slaw_write);
	//TODO: задефайнить адреса регистров
	tmp = 0xD0;
	rscs_i2c_write(&tmp, 1);
	rscs_i2c_start();
	rscs_i2c_send_slaw(descr->address, rscs_i2c_slaw_read);
	rscs_i2c_read(&tmp, 1, true);
	rscs_i2c_stop();
	if(tmp != 0x58) return RSCS_E_INVRESP;

	rscs_i2c_start();
	rscs_i2c_send_slaw(descr->address, rscs_i2c_slaw_write);
	tmp = 0x88;
	rscs_i2c_write(&tmp, 1);
	rscs_i2c_start();
	rscs_i2c_send_slaw(descr->address, rscs_i2c_slaw_read);
	rscs_i2c_read(&descr->calibration_values, sizeof(descr->calibration_values), true);
	rscs_i2c_stop();

	rscs_i2c_start();
	rscs_i2c_send_slaw(descr->address, rscs_i2c_slaw_write);
	tmp = 0xF4;
	rscs_i2c_write(&tmp, 1);
	tmp = (params->temperature_oversampling << 5) | (params->pressure_oversampling << 2);
	rscs_i2c_write(&tmp, 1);
	tmp = (params->standbytyme << 5) | (params->filter << 2);
	rscs_i2c_write(&tmp, 1);
	rscs_i2c_stop();

	descr->parameters = *params;

	return RSCS_E_NONE;
}

const rscs_bmp280_parameters_t * rscs_bmp280_get_config(rscs_bmp280_descriptor_t * descr){
	return &descr->parameters;
}

const rscs_bmp280_calibration_values_t * rscs_bmp280_get_calibration_values(rscs_bmp280_descriptor_t * descr){
	return &descr->calibration_values;
}

rscs_e rscs_bmp280_changemode(rscs_bmp280_descriptor_t * bmp, rscs_bmp280_mode_t mode){
	uint8_t tmp;
	rscs_i2c_start();
	rscs_i2c_send_slaw(bmp->address, rscs_i2c_slaw_write);
	tmp = 0xF4;
	rscs_i2c_write(&tmp, 1);
	tmp = (bmp->parameters.temperature_oversampling << 5) |
			(bmp->parameters.pressure_oversampling << 2) |
			mode;
	rscs_i2c_write(&tmp, 1);
	rscs_i2c_stop();

	return RSCS_E_NONE;
}

rscs_e rscs_bmp280_read(rscs_bmp280_descriptor_t * bmp, uint32_t * rawpress, uint32_t * rawtemp){
	uint8_t tmp[6];
	rscs_i2c_start();
	rscs_i2c_send_slaw(bmp->address, rscs_i2c_slaw_write);
	tmp[0] = 0xF7;
	rscs_i2c_write(tmp, 1);
	rscs_i2c_start();
	rscs_i2c_send_slaw(bmp->address, rscs_i2c_slaw_read);
	rscs_i2c_read(tmp, 6, true);
	rscs_i2c_stop();
	*rawpress = (tmp[0] << 12) | (tmp[1] << 4) | (tmp[2] >> 4);
	*rawtemp = (tmp[3] << 12) | (tmp[4] << 4) | (tmp[5] >> 4);

	return RSCS_E_NONE;
}

/*
 * tsl2561.c
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */

#include "../tsl2561.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "../i2c.h"

uint8_t tsl2561_addr1 = 0x29; // 0b00101001 // пин ADDR SEL подтянут к GND
int tsl2561_ACK = 0;
int tsl2561_NACK = 1;

#define GOTO_END_IF_ERROR(expr) error = expr; if (error != RSCS_E_NONE) goto end;
#define CMD_PREAMBLE 0x90 // 0b10010000

#define CONTROL_REG_ADDR 0x00
#define TIMING_REG_ADDR 0x01
#define INTERRUPT_REG_ADDR 0x06
#define DATA0LOW_REG_ADDR 0x0C
#define DATA0HIGH_REG_ADDR 0x0D
#define DATA1LOW_REG_ADDR 0x0E
#define DATA1HIGH_REG_ADDR 0x0F

static rscs_e error;

rscs_e write_reg_value(uint8_t reg_addr, uint8_t reg_value)
{
	uint8_t control_c = CMD_PREAMBLE | reg_addr;
	GOTO_END_IF_ERROR(rscs_i2c_write(&control_c, 1));

	GOTO_END_IF_ERROR(rscs_i2c_write(&reg_value, 1));

end:
	return error;
}

rscs_e rscs_tsl2561_init(void)
{
	// начинаем i2c транзакцию
	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	// передаем адрес ведомого в режиме на запись
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));

	write_reg_value(CONTROL_REG_ADDR, 0x03); // 0b00000011 // младшие биты 11 означают, что питание ВКЛ
	write_reg_value(TIMING_REG_ADDR, 0x02); // 0b00000010 // младшие биты 10 означают, что время интеграции 402мс
    write_reg_value(INTERRUPT_REG_ADDR, 0x00); // 0b00000000 // 5 и 4 биты отключают прерывания
    rscs_i2c_stop();
end:
	rscs_i2c_stop();
	return error;
}

uint16_t rscs_tsl2561_read(uint16_t * sensor_data0, uint16_t * sensor_data1) // в массив PD0_ARRAY[4] записываются 4 байта,
// посылаемые датчиком. Первые два - младшие и старшие биты фотодиода 0, а последние два -
// младшие и старшие биты фотодиода 1
{
    rscs_e error = RSCS_E_NONE;

    error = rscs_i2c_start();
    if (error != RSCS_E_NONE)
    	return error;

    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));

    GOTO_END_IF_ERROR(rscs_i2c_write_byte(DATA0LOW_REG_ADDR));

    GOTO_END_IF_ERROR(rscs_i2c_start()); // создали restart

    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_read));

    GOTO_END_IF_ERROR(rscs_i2c_read(sensor_data0, 2, false));
    GOTO_END_IF_ERROR(rscs_i2c_read(sensor_data1, 2, true));

end:
    rscs_i2c_stop();
    return error;
}

uint16_t get_lux(uint16_t sensor_data0, uint16_t sensor_data1)
{
	int lux;
    float x = sensor_data1 / sensor_data0;
    if (0 < x && x <= 0.52) lux = 0.0315 * sensor_data0 - 0.0593 * sensor_data0 * (pow(x, 1.4));
    if (0.52 < x && x <= 0.65) lux = 0.0229 * sensor_data0 - 0.0291 * sensor_data1;
    if (0.65 < x && x <= 0.8) lux = 0.0157 * sensor_data0 - 0.018 * sensor_data1;
    if (0.8 < x && x <= 1.3) lux = 0.00338 * sensor_data0 - 0.00260 * sensor_data1;
    if (x > 1.3) lux = 0;

    return lux;
}

/*
 * tsl2561.c
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */

#include "../tsl2561.h"

#include <stdbool.h>
#include <stdint.h>

#include "../i2c.h"

// регистры установки порога измерений
#define threshlowlow_c 0b10010010;
#define threshlowhigh_c  0b10110011;
#define threshhighlow_c  0b10110100;
#define threshhighhigh_c 0b10110101;

#define interrupt_c  0b10010110;
#define crc_c  0b10011000;
#define id_c 0b10011010;
#define data0low_c 0b10011100;
#define data0high_c 0b10011101;
#define data1low_c 0b10011110;
#define data1high_c 0b10011111;

uint8_t tsl2561_addr1 = 0b0101001;
int tsl2561_ACK = 0;
int tsl2561_NACK = 1;

#define GOTO_END_IF_ERROR(expr) if (expr != RSCS_E_NONE) goto end;
#define CMD_PREAMBLE 0b10010000

#define CONTROL_REG_ADDR 0x00

rscs_e write_reg_value(uint8_t reg_addr, uint8_t reg_value)
{
	rscs_e error;

	uint8_t control_c = CMD_PREAMBLE | reg_addr;
	GOTO_END_IF_ERROR(rscs_i2c_write(&control_c, 1));

	// формируем содержимое регистра CONTROL (младшие биты - режим питания)
	// ставим питание на ВКЛ
	GOTO_END_IF_ERROR(rscs_i2c_write(&reg_value, 1));

end:
	return error;
}

rscs_e rscs_tsl2561_init(void)
{
	rscs_e error = RSCS_E_NONE;

	// начинаем i2c транзакцию
	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	// передаем адрес ведомого в режиме на запись
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));

	write_reg_value(CONTROL_REG_ADDR, 0b00000011); // младшие биты 11 означают, питание ВКЛ
	write_reg_value(CONTROL_REG_ADDR, 0b00000011); // младшие биты 11 означают, питание ВКЛ

end:
	rscs_i2c_stop();
}

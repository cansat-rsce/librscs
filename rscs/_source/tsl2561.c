/*
 * tsl2561.c
 *
 *  Created on: 11 февр. 2017 г.
 *      Author: developer
 */
#include <stdbool.h>
#include <stdint.h>
#include <i2c.h>
// регистры установки порога измерений
uint8_t threshlowlow_c = 0b10010010;
uint8_t threshlowhigh_c = 0b10110011;
uint8_t threshhighlow_c = 0b10110100;
uint8_t threshhighhigh_c = 0b10110101;

uint8_t interrupt_c = 0b10010110;
uint8_t crc_c = 0b10011000;
uint8_t id_c = 0b10011010;
uint8_t data0low_c = 0b10011100;
uint8_t data0high_c = 0b10011101;
uint8_t data1low_c = 0b10011110;
uint8_t data1high_c = 0b10011111;

uint8_t tsl2561_addr1 = 0b0101001;
int tsl2561_ACK = 0;
int tsl2561_NACK = 1;

void rscs_tsl2561_init(void)
{
	rscs_i2c_start();
	rscs_i2c_send_slaw(tsl2561_addr1, 0); // 0 -- запись
	rscs_i2c_write(&tsl2561_ACK, 1);

	uint8_t control_c = 0b10010000;
	rscs_i2c_write(&control_c, 1);
	rscs_i2c_write(&tsl2561_ACK, 1);
    uint8_t control = 0b00000011;
	rscs_i2c_write(& control, 1);
	rscs_i2c_write(&tsl2561_ACK, 1);

	rscs_i2c_stop();
}

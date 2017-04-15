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

#include <stdio.h>

#include "../i2c.h"

uint8_t tsl2561_addr1 = 0b00111001; // пин ADDR SEL летает
int tsl2561_ACK = 0;
int tsl2561_NACK = 1;

#define GOTO_END_IF_ERROR(expr) error = expr; if (error != RSCS_E_NONE) goto end;
#define CMD_PREAMBLE 0x80 // 0b10010000

#define CONTROL_REG_ADDR 0x00
#define TIMING_REG_ADDR 0x01
#define INTERRUPT_REG_ADDR 0x06
#define DATA0LOW_REG_ADDR 0x0C
#define DATA0HIGH_REG_ADDR 0x0D
#define DATA1LOW_REG_ADDR 0x0E
#define DATA1HIGH_REG_ADDR 0x0F

#define K1C  0x0043 // 0.130 * 2^RATIO_SCALE
	#define B1C  0x0204 // 0.0315 * 2^LUX_SCALE
	#define M1C  0x01ad // 0.0262 * 2^LUX_SCALE
	#define K2C  0x0085 // 0.260 * 2^RATIO_SCALE
	#define B2C  0x0228 // 0.0337 * 2^LUX_SCALE
	#define M2C  0x02c1 // 0.0430 * 2^LUX_SCALE
	#define K3C  0x00c8 // 0.390 * 2^RATIO_SCALE
	#define B3C  0x0253 // 0.0363 * 2^LUX_SCALE
	#define M3C  0x0363 // 0.0529 * 2^LUX_SCALE
	#define K4C  0x010a // 0.520 * 2^RATIO_SCALE
	#define B4C  0x0282 // 0.0392 * 2^LUX_SCALE
	#define M4C  0x03df // 0.0605 * 2^LUX_SCALE
	#define K5C  0x014d // 0.65 * 2^RATIO_SCALE
	#define B5C  0x0177 // 0.0229 * 2^LUX_SCALE
	#define M5C  0x01dd // 0.0291 * 2^LUX_SCALE
	#define K6C  0x019a // 0.80 * 2^RATIO_SCALE
	#define B6C  0x0101 // 0.0157 * 2^LUX_SCALE
	#define M6C  0x0127 // 0.0180 * 2^LUX_SCALE
	#define K7C  0x029a // 1.3 * 2^RATIO_SCALE
	#define B7C  0x0037 // 0.00338 * 2^LUX_SCALE
	#define M7C  0x002b // 0.00260 * 2^LUX_SCALE
	#define K8C  0x029a // 1.3 * 2^RATIO_SCALE
	#define B8C  0x0000 // 0.000 * 2^LUX_SCALE
	#define M8C  0x0000 // 0.000 * 2^LUX_SCALE
	#define K1T  0x0040 // 0.125 * 2^RATIO_SCALE
	#define B1T  0x01f2 // 0.0304 * 2^LUX_SCALE
	#define M1T  0x01be // 0.0272 * 2^LUX_SCALE
	#define K2T  0x0080 // 0.250 * 2^RATIO_SCALE
	#define B2T  0x0214 // 0.0325 * 2^LUX_SCALE
	#define M2T  0x02d1 // 0.0440 * 2^LUX_SCALE
	#define K3T  0x00c0 // 0.375 * 2^RATIO_SCALE
	#define B3T  0x023f // 0.0351 * 2^LUX_SCALE
	#define M3T  0x037b // 0.0544 * 2^LUX_SCALE
	#define K4T  0x0100 // 0.50 * 2^RATIO_SCALE
	#define B4T  0x0270 // 0.0381 * 2^LUX_SCALE
	#define M4T  0x03fe // 0.0624 * 2^LUX_SCALE
	#define K5T  0x0138 // 0.61 * 2^RATIO_SCALE
	#define B5T  0x016f // 0.0224 * 2^LUX_SCALE
	#define M5T  0x01fc // 0.0310 * 2^LUX_SCALE
	#define K6T  0x019a // 0.80 * 2^RATIO_SCALE
	#define B6T  0x00d2 // 0.0128 * 2^LUX_SCALE
	#define M6T  0x00fb // 0.0153 * 2^LUX_SCALE
	#define K7T  0x029a // 1.3 * 2^RATIO_SCALE
	#define B7T  0x0018 // 0.00146 * 2^LUX_SCALE
	#define M7T  0x0012 // 0.00112 * 2^LUX_SCALE
	#define K8T  0x029a // 1.3 * 2^RATIO_SCALE
	#define B8T  0x0000 // 0.000 * 2^LUX_SCALE
	#define M8T  0x0000 // 0.000 * 2^LUX_SCALE

	#define LUX_SCALE   14     // scale by 2^14
	#define RATIO_SCALE 9      // scale ratio by 2^9
	#define CH_SCALE           10     // scale channel values by 2^10
	#define CHSCALE_TINT0      0x7517 // 322/11 * 2^CH_SCALE
	#define CHSCALE_TINT1      0x0fe7 // 322/81 * 2^CH_SCALE

static rscs_e error;

static rscs_e _write_reg8(uint8_t reg_addr, uint8_t reg_value)
{
	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	// передаем адрес ведомого в режиме на запись
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));

	uint8_t control_c = CMD_PREAMBLE | reg_addr;
	GOTO_END_IF_ERROR(rscs_i2c_write(&control_c, 1));
	GOTO_END_IF_ERROR(rscs_i2c_write(&reg_value, 1));

end:
	rscs_i2c_stop();
	return error;
}


rscs_e rscs_tsl2561_init(void)
{
	// начинаем i2c транзакцию
	GOTO_END_IF_ERROR(_write_reg8(CONTROL_REG_ADDR, 0x03)); // 0b00000011 // младшие биты 11 означают, что питание ВКЛ
	GOTO_END_IF_ERROR(_write_reg8(TIMING_REG_ADDR, 0x02)); // 0b00000010 // младшие биты 10 означают, что время интеграции 402мс
	GOTO_END_IF_ERROR(_write_reg8(INTERRUPT_REG_ADDR, 0x00)); // 0b00000000 // 5 и 4 биты отключают прерывания


end:
	return error;

	/*
	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;

	// передаем адрес ведомого в режиме на запись
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));
	uint8_t writeValue = 0x08 | 0x0A;
	GOTO_END_IF_ERROR(rscs_i2c_write(&writeValue, 1)); // 0b00000011 // младшие биты 11 означают, что питание ВКЛ

	error = rscs_i2c_start();
	if (error != RSCS_E_NONE)
		return error;
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_read));
	uint8_t regValue;
	GOTO_END_IF_ERROR(rscs_i2c_read(&regValue, 1, true));

	printf("id_value = %d\n", regValue);
end:
	rscs_i2c_stop();
	return error;

	*/
}


rscs_e rscs_tsl2561_read(uint16_t * sensor_data0, uint16_t * sensor_data1)
{
    error = rscs_i2c_start();
    if (error != RSCS_E_NONE)
        return error;

    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));
    GOTO_END_IF_ERROR(rscs_i2c_write_byte(0x80 | 0x20 | 0x0E));
    GOTO_END_IF_ERROR(rscs_i2c_start()); // создали restart
    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_read));
    GOTO_END_IF_ERROR(rscs_i2c_read(sensor_data1, 2, true));
    rscs_i2c_stop();

    error = rscs_i2c_start();
    if (error != RSCS_E_NONE)
    	return error;
    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_write));
    GOTO_END_IF_ERROR(rscs_i2c_write_byte(0x80 | 0x20 | 0x0C));
    GOTO_END_IF_ERROR(rscs_i2c_start()); // создали restart
    GOTO_END_IF_ERROR(rscs_i2c_send_slaw(tsl2561_addr1, rscs_i2c_slaw_read));
    GOTO_END_IF_ERROR(rscs_i2c_read(sensor_data0, 2, true));

end:
    rscs_i2c_stop();
    return error;
}

unsigned int rscs_tsl2561_get_lux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1, int iType)
	{

	// iGain - масштаб (0 : 1X, 1 : 16X)
	// tInt - время интеграции, где 0 : 13мс, 1 : 100мс, 2 : 402мс, 3 : ручной
	// iType - тип корпуса (0 : T, 1 : CS)
	// масштабировать, если время интеграции НЕ 402мс

	unsigned long chScale;
	unsigned long channel1;
	unsigned long channel0;
	switch (tInt)
	{
	case 0:    // 13.7мс
		chScale = CHSCALE_TINT0;
		break;
	case 1:    // 101мс
		chScale = CHSCALE_TINT1;
		break;
	default:   // без масштабирования
		chScale = (1 << CH_SCALE);
		break;
	}
	// масштабировать, если gain НЕ 16X
	if (!iGain) chScale = chScale << 4;   // scale 1X to 16X
	printf("chscale = %lu\n", chScale);
	// scale the channel values
	channel0 = (ch0 * chScale) >> CH_SCALE;
	channel1 = (ch1 * chScale) >> CH_SCALE;
	printf("ch0 = %lu\n", channel0);
	printf("ch1 = %lu\n", channel1);
	//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	unsigned long ratio1 = 0;
	if (channel0 != 0) ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
	// round the ratio value
	printf("ratio1 = %lu\n", ratio1);
	unsigned long ratio = (ratio1 + 1) >> 1;
	// is ratio <= eachBreak ?
	unsigned int b, m;
	switch (iType)
	{
	case 0: // T, FN and CL package
		if ((ratio >= 0) && (ratio <= K1T))
		{b=B1T; m=M1T;}
		else if (ratio <= K2T)
		{b=B2T; m=M2T;}
		else if (ratio <= K3T)
		{b=B3T; m=M3T;}
		else if (ratio <= K4T)
		{b=B4T; m=M4T;}
		else if (ratio <= K5T)
		{b=B5T; m=M5T;}
		else if (ratio <= K6T)
		{b=B6T; m=M6T;}
		else if (ratio <= K7T)
		{b=B7T; m=M7T;}
		else if (ratio > K8T)
		{b=B8T; m=M8T;}
		break;
	case 1:// CS package
		if ((ratio >= 0) && (ratio <= K1C))
		{b=B1C; m=M1C;}
		else if (ratio <= K2C)
		{b=B2C; m=M2C;}
		else if (ratio <= K3C)
		{b=B3C; m=M3C;}
		else if (ratio <= K4C)
		{b=B4C; m=M4C;}
		else if (ratio <= K5C)
		{b=B5C; m=M5C;}
		else if (ratio <= K6C)
		{b=B6C; m=M6C;}
		else if (ratio <= K7C)
		{b=B7C; m=M7C;}
		else if (ratio > K8C)
		{b=B8C; m=M8C;}
		break;
	}
	printf("ratio = %lu\n", ratio);
	unsigned long temp;
	temp = ((channel0 * b) - (channel1 * m));
	// do not allow negative lux value
	if (temp < 0) temp = 0;
	// round lsb (2^(LUX_SCALE−1))
	temp += (1 << (LUX_SCALE - 1));
	// strip off fractional portion
	unsigned long lux = temp >> LUX_SCALE;

	return lux;
}



/* */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "../i2c.h"

#include "../ads1115.h"

#define RSCS_ADS1115_REG_DATA 		0x00
#define RSCS_ADS1115_REG_CONFIG 	0x01
#define RSCS_ADS1115_REG_LOTHRESH	0x02
#define RSCS_ADS1115_REG_HITHRESH	0x03


#define RSCS_ADS1115_MV_PER_PARROT_6DOT144 0.1875f
#define RSCS_ADS1115_MV_PER_PARROT_4DOT096 0.125f
#define RSCS_ADS1115_MV_PER_PARROT_2DOT048 0.0625f
#define RSCS_ADS1115_MV_PER_PARROT_1DOT024 0.03125f
#define RSCS_ADS1115_MV_PER_PARROT_0DOT512 0.015625f
#define RSCS_ADS1115_MV_PER_PARROT_0DOT256 0.007813f


#define CHBYTEORDER(CHAR) CHAR = (CHAR << 8) | (CHAR >> 8);
#define OPERATION(OP) error = OP; if(error != RSCS_E_NONE) goto end;


static rscs_e _i2c_readreg(uint8_t addr, uint8_t reg, void * data) {
	uint16_t * data_u16 = (uint16_t *) data;

	rscs_e error = RSCS_E_NONE;

	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(addr, rscs_i2c_slaw_write))
	OPERATION(rscs_i2c_write_byte(reg))
	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(addr, rscs_i2c_slaw_read))
	OPERATION(rscs_i2c_read(data_u16, 2, true))
	CHBYTEORDER(*data_u16)

end:
	rscs_i2c_stop();
	return error;
}


static rscs_e _i2c_writereg(uint8_t addr, uint8_t reg, const uint16_t * data) {
	rscs_e error = RSCS_E_NONE;

	OPERATION(rscs_i2c_start());
	OPERATION(rscs_i2c_send_slaw(addr, rscs_i2c_slaw_write))
	OPERATION(rscs_i2c_write_byte(reg))

	uint16_t dataswapped = *data;
	CHBYTEORDER(dataswapped);
	OPERATION(rscs_i2c_write(&dataswapped, 2))
end:
	rscs_i2c_stop();
	return error;
}


struct rscs_ads1115_t {
	rscs_i2c_addr_t address;
	rscs_ads1115_range_t range;
};


rscs_ads1115_t * rscs_ads1115_init(rscs_i2c_addr_t addr) {
	rscs_ads1115_t * adc = malloc(sizeof(rscs_ads1115_t));
	adc->address = addr;
	adc->range = RSCS_ADS1115_RANGE_2DOT048;

	return adc;
}


void rscs_ads1115_deinit(rscs_ads1115_t * device) {
	free(device);
}


static rscs_e _set_channel(rscs_ads1115_t * device, rscs_ads1115_channel_t channel) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config &= ~((1 << 14) | (1 << 13) | (1 << 12));
	config |= (channel << 12);

	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

end:
	return error;
}


rscs_e rscs_ads1115_set_range(rscs_ads1115_t * device, rscs_ads1115_range_t range) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config &= ~((1 << 11) | (1 << 10) | (1 << 9));
	config |= (range << 9);
	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	device->range = range;

end:
	return error;
}


rscs_e rscs_ads1115_set_datarate(rscs_ads1115_t * device, rscs_ads1115_datarate_t datarate) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config &= ~((1 << 7) | (1 << 6) | (1 << 5));
	config |= (datarate << 5);

	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

end:
	return error;
}


rscs_e rscs_ads1115_start_single(rscs_ads1115_t * device, rscs_ads1115_channel_t channel) {
	rscs_e error = RSCS_E_NONE;

	OPERATION(_set_channel(device, channel))

	uint16_t config = 0;
	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config |= (1 << 8);
	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config |= (1 << 15);
	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

end:
	return error;
}


rscs_e rscs_ads1115_start_continuous(rscs_ads1115_t * device, rscs_ads1115_channel_t channel) {
	rscs_e error = RSCS_E_NONE;

	OPERATION(_set_channel(device, channel))

	uint16_t config = 0;
	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config &= ~(1 << 8);
	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config |= (1 << 15);
	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

end:
	return error;
}


rscs_e rscs_ads1115_stop_continuous(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

	config |= (1 << 8);

	OPERATION(_i2c_writereg(device->address, RSCS_ADS1115_REG_CONFIG, &config))

end:
		return error;
}


rscs_e rscs_ads1115_read(rscs_ads1115_t * device, int16_t * value) {
	return _i2c_readreg(device->address, RSCS_ADS1115_REG_DATA, value);
}


rscs_e rscs_ads1115_wait_result(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	while(1) {
		// TODO: ADS1115: добавить таймаут
		OPERATION(_i2c_readreg(device->address, RSCS_ADS1115_REG_CONFIG, &config))
		if((config & (1 << 15))) break;
		if(!(config & (1 << 8))) break;
	}

end:
	return error;
}


rscs_e rscs_ads1115_take(rscs_ads1115_t * device, rscs_ads1115_channel_t channel, int16_t * value) {
	rscs_e error = RSCS_E_NONE;

	OPERATION(rscs_ads1115_start_single(device, channel))
	OPERATION(rscs_ads1115_wait_result(device))
	OPERATION(rscs_ads1115_read(device, value))

end:
	return error;
}


float rscs_ads1115_convert(rscs_ads1115_t * device, int16_t rawdata) {
	switch(device->range) {
	case RSCS_ADS1115_RANGE_6DOT144:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_6DOT144;

	case RSCS_ADS1115_RANGE_4DOT096:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_4DOT096;

	case RSCS_ADS1115_RANGE_2DOT048:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_2DOT048;

	case RSCS_ADS1115_RANGE_1DOT024:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_1DOT024;

	case RSCS_ADS1115_RANGE_0DOT512:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_0DOT512;

	case RSCS_ADS1115_RANGE_0DOT256:
			return rawdata * RSCS_ADS1115_MV_PER_PARROT_0DOT256;
	default: return 0;
	}
}

#undef OPERATION

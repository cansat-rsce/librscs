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

#define OPERATION(OP) error = OP; if(error != RSCS_E_NONE) goto end;

#define RSCS_I2C_READREG(ADDR, REG, DATA, COUNT) \
	rscs_i2c_start();\
	OPERATION(rscs_i2c_send_slaw(ADDR, rscs_i2c_slaw_write)) \
	OPERATION(rscs_i2c_write_byte(REG)) \
	rscs_i2c_start();\
	OPERATION(rscs_i2c_send_slaw(ADDR, rscs_i2c_slaw_read)) \
	/*printf("ADS1115: READREG: read\n");*/ \
	OPERATION(rscs_i2c_read(DATA, COUNT, true)) \
	rscs_i2c_stop();

#define RSCS_I2C_WRITEREG(ADDR, REG, DATA, COUNT) \
	rscs_i2c_start();\
	OPERATION(rscs_i2c_send_slaw(ADDR, rscs_i2c_slaw_write)) \
	OPERATION(rscs_i2c_write_byte(REG)) \
	OPERATION(rscs_i2c_write(DATA, COUNT)) \
	rscs_i2c_stop();


struct rscs_ads1115_t {
	i2c_addr_t address;
};

rscs_ads1115_t * rscs_ads1115_init(i2c_addr_t addr) {
	rscs_ads1115_t * adc = malloc(sizeof(rscs_ads1115_t));
	adc->address = addr;

	return adc;
}

void rscs_ads1115_deinit(rscs_ads1115_t * device) {
	free(device);
}

rscs_e rscs_ads1115_set_channel(rscs_ads1115_t * device, rscs_ads1115_channel_t channel) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	config &= ~((1 << 14) | (1 << 13) | (1 << 12));
	config |= (channel << 12);

	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_set_range(rscs_ads1115_t * device, rscs_ads1115_range_t range) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	printf("ADS1115: set_range: READREG\n");
	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	config &= ~((1 << 11) | (1 << 10) | (1 << 9));
	config |= (range << 9);
	printf("ADS1115: set_range: WRITEREG\n");
	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_set_datarate(rscs_ads1115_t * device, rscs_ads1115_datarate_t datarate) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	config &= ~((1 << 7) | (1 << 6) | (1 << 5));
	config |= (datarate << 5);

	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_start_single(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	if(!(config & (1 << 15))){
		error = RSCS_E_BUSY;
		goto end;
	}

	config |= (1 << 8);
	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	config |= (1 << 15);
	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_start_continuous(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	/*bool n_busy = (config & (1 << 15)) == (1 << 15);
	if(!n_busy){
		error = RSCS_E_BUSY;
		goto end;
	}*/

	config &= ~(1 << 8);

	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_stop_continuous(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

	config |= (1 << 8);

	RSCS_I2C_WRITEREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_read(rscs_ads1115_t * device, uint16_t * value) {
	rscs_e error = RSCS_E_NONE;

	RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, value, sizeof(*value))

end:
	rscs_i2c_stop();
	return error;
}

rscs_e rscs_ads1115_wait_result(rscs_ads1115_t * device) {
	rscs_e error = RSCS_E_NONE;

	uint16_t config = 0;

	while(1) {
		RSCS_I2C_READREG(device->address, RSCS_ADS1115_REG_CONFIG, &config, sizeof(config))
		if((config & (1 << 15))) break;
	}

end:
	rscs_i2c_stop();
	return error;
}

#undef OPERATION

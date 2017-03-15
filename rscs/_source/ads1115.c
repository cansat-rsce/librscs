/* */


#include "../i2c.h"

#include "../ads1115.h"

struct rscs_ads1115_t {
	i2c_addr_t address;
};

rscs_ads1115_t * rscs_ads1115_init(i2c_addr_t addr){
	rscs_ads1115_t * adc = malloc(sizeof(rscs_ads1115_t));

	return adc;
}

// Все написать ...

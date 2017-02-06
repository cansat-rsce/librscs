#include "../ds18b20.h"

#include <stdlib.h>

// дескриптор датчика
struct rscs_ds18b20_t
{
	uint64_t uid; // его уникальный идентификатор
};


rscs_ds18b20_t * ds18b20_init(uint64_t sensor_uid)
{
	// создаем дескриптор
	rscs_ds18b20_t * retval = malloc(sizeof(rscs_ds18b20_t));
	if (!retval)
		return NULL;

	// ... заполняем его поля и проверяем

	// возвращаем пользователю
	return retval;
}



void ds18b20_deinit(rscs_ds18b20_t * sensor)
{
	// просто освобождаем память дескриптора
	free(sensor);
}


rscs_e ds18b20_start_conversion(rscs_ds18b20_t * sensor)
{

	return RSCS_E_NONE;
}


rscs_e ds18b20_read_temperature(rscs_ds18b20_t * sensor, int16_t * value_buffer)
{

	return RSCS_E_NONE;
}


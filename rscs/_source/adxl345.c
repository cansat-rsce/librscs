#include "../adxl345.h"

#include <stdlib.h>

#include "../i2c.h"
#include "../error.h"


// описание стурктуры дескриптора
struct rscs_adxl345_t
{
	// TODO: ADXL: Определить дескриптор
};


rscs_adxl345_t * rscs_adxl345_init(rscs_adxl345_addr_t addr)
{
	// создаем дескриптор
	rscs_adxl345_t * retval = (rscs_adxl345_t *)malloc(sizeof(rscs_adxl345_t));
	if (!retval)
		return retval;


	// и инициализируем
	// TODO: ADXL:
	// заодно тут можно проверить не спит ли акселерометр и включить его
	// и настроить всякие параметры по-умолчанию или к которым модуль не предоставляет
	// интерфейса

	return retval;
}



void rscs_adxl345_deinit(rscs_adxl345_t * device)
{
	// TODO: ADXL: напимер усыпляем акселерометр, чтобы не потреблял электричество
	// если не лень это писать конечно

	// освобождем память, занимаемую дескриптором
	free(device);
}



rscs_e rscs_adxl345_set_range(rscs_adxl345_t * device, rscs_adxl345_range_t range)
{
	// TODO: ADXL: написать реализацию функции

	return RSCS_E_NONE;
}


rscs_e rscs_adxl345_set_rate(rscs_adxl345_t * device, rscs_adxl345_rate_t rate)
{
	// TODO: ADXL: написать реализацию функции

	return RSCS_E_NONE;
}


rscs_e rscs_adxl345_read(rscs_adxl345_t * device, int16_t * x, int16_t * y, int16_t * z)
{
	// TODO: ADXL: написать реализацию функции

	return RSCS_E_NONE;
}

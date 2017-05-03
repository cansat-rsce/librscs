#include "../ds18b20.h"

#include <stdlib.h>
#include <util/atomic.h>

#include <rscs/onewire.h>
#include <rscs/crc.h>

/* Эта команда начинает единственное температурное преобразование.
 * По окончании данные сохраняются 2-байтовом температурном регистре,
 * а датчик возвращается в неактивное состояние с низким электропотреблением.
 */
#define RSCS_DS18B20_CONVERT_T 0x44

/* Этак оманда позволяет главному устройству читать содержание памяти.
 * Передача данных начинается с наименьшего значащего байта бита 0 и продолжается до 9-ого байта.
 * Главное устройство может выполнить сброс, чтобы закончить чтение в любое время, если необходима только часть данных.
 */
#define RSCS_DS18B20_READ_SCRATCHPAD 0xBE

// дескриптор датчика
struct rscs_ds18b20_t
{
	uint64_t uid; // его уникальный идентификатор
};

rscs_ds18b20_t * rscs_ds18b20_init(uint64_t sensor_uid)
{
	// создаем дескриптор
	rscs_ds18b20_t * retval = malloc(sizeof(rscs_ds18b20_t));
	if (!retval)
		return NULL;

	// ... заполняем его поля и проверяем
	retval->uid = sensor_uid;

	// возвращаем пользователю
	return retval;
}

void rscs_ds18b20_deinit(rscs_ds18b20_t * sensor)
{
	// просто освобождаем память дескриптора
	free(sensor);
}

rscs_e rscs_ds18b20_start_conversion(rscs_ds18b20_t * sensor)
{
	rscs_e error = RSCS_E_NONE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Посылаем импульс сброса. Его необходимо посылать при каждом обращении к DS18B20.
		if (!rscs_ow_reset())
			error = RSCS_E_INVRESP;
		else {
			// Пропускаем этап адресации.
			rscs_ow_write(RSCS_OW_CMD_SKIPADDR);
			// Начинаем температурное преобразование.
			rscs_ow_write(RSCS_DS18B20_CONVERT_T);
		}
	}

	return error;
}


bool rscs_ds18b20_check_ready(void)
{
	bool retval;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		retval = rscs_ow_read_bit();
	}
	return retval;
}


rscs_e rscs_ds18b20_read_temperature(rscs_ds18b20_t * sensor, int16_t * value_buffer)
{
	rscs_e error = RSCS_E_NONE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Посылаем импульс сброса. Его необходимо посылать при каждом обращении к DS18B20.
		if (!rscs_ow_reset())
			error = RSCS_E_INVRESP;
		else {
			// Пропускаем этап адресации.
			rscs_ow_write(RSCS_OW_CMD_SKIPADDR);
			// Читаем содержимое памяти.
			rscs_ow_write(RSCS_DS18B20_READ_SCRATCHPAD);
			uint8_t str [9];
			for(int i = 0; i < 9; i++) {
				str[i] = rscs_ow_read();
			}
			//Проверяем контрольную сумму
			if (str[8] != rscs_crc8(&str[0],sizeof str - 1))
				error = RSCS_E_CHKSUM;
			// Вычисляем значение
			else (*value_buffer) = str[0] | (str[1]<<8);
		}
	}
	return error;
}

float rscs_ds18b20_count_temperature(rscs_ds18b20_t * sensor, int16_t raw_value){
	return raw_value / 16.0f;
}

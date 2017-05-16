
#include <stdlib.h>
#include <math.h>
#include <util/delay.h>

#include "../i2c.h"
#include "../spi.h"
#include "../error.h"
#include "../adxl345.h"


//TODO: ADXL: заготовка для неосновных параметров
/*typedef struct {
	int8_t offset_x, offset_y, offset_z; / * Смещение, добавляемое к резульатам
											измерения (для калибровки, например)
											(15.6 mg/LSB)* /



	// Настройки событий TAP и DOUBLE_TAP (подробнее в даташите)
	uint8_t tap_threshold, 	/ * Значение ускорение, при котором сработает
							 * событие TAP (если включено)
							 * (62.5 mg/LSB)* /
	tap_duration, / *Сколько времени ускорение должо быть выше tap_threshold,
					чтобы вызвать событие TAP (625μs/LSB)* /
	tap_double_latent, 	/ * Время между событием TAP и началом окна, во время
						   которого можно сгенерировать DOUBLE_TAP (1.25 ms/LSB)* /
	tap_double_window; / * Продолжительность окна, в которое событие DOUBLE_TAP
						  может быть сгенерировано (1.25 ms/LSB)* /

} rscs_adxl345_settings_t;*/


/* Команды на чтение и запись */
#define RSCS_ADXL345_SPI_READ        (1 << 7)	//бит на чтение
#define RSCS_ADXL345_SPI_WRITE       (0 << 7)	//бит на запись
#define RSCS_ADXL345_SPI_MB          (1 << 6)	//бит чтение/запись нескольких байт (в противном случае только одного байта)

/* ADXL345 Адреса регистров */
#define RSCS_ADXL345_DEVID			0x00 // R	Device ID
#define RSCS_ADXL345_OFSX            0x1E // R/W X-axis offset.
#define RSCS_ADXL345_OFSY            0x1F // R/W Y-axis offset.
#define RSCS_ADXL345_OFSZ            0x20 // R/W Z-axis offset.
#define RSCS_ADXL345_BW_RATE         0x2C // R/W Data rate and power mode control.
#define RSCS_ADXL345_POWER_CTL       0x2D // R/W Power saving features control.
#define RSCS_ADXL345_INT_ENABLE      0x2E // R/W Interrupt enable control.
#define RSCS_ADXL345_INT_MAP         0x2F // R/W Interrupt mapping control.
#define RSCS_ADXL345_INT_SOURCE      0x30 // R   Source of interrupts.
#define RSCS_ADXL345_DATA_FORMAT     0x31 // R/W Data format control.
#define RSCS_ADXL345_DATAX0          0x32 // R   X-Axis Data 0.
#define RSCS_ADXL345_DATAX1          0x33 // R   X-Axis Data 1.
#define RSCS_ADXL345_DATAY0          0x34 // R   Y-Axis Data 0.
#define RSCS_ADXL345_DATAY1          0x35 // R   Y-Axis Data 1.
#define RSCS_ADXL345_DATAZ0          0x36 // R   Z-Axis Data 0.
#define RSCS_ADXL345_DATAZ1          0x37 // R   Z-Axis Data 1.
#define RSCS_ADXL345_FIFO_CTL        0x38 // R/W FIFO control.
#define RSCS_ADXL345_FIFO_STATUS     0x39 // R   FIFO status.

/* ADXL345_POWER_CTL Определение регистра */
#define RSCS_ADXL345_PCTL_LINK       (1 << 5)
#define RSCS_ADXL345_PCTL_AUTO_SLEEP (1 << 4)
#define RSCS_ADXL345_PCTL_MEASURE    (1 << 3)
#define RSCS_ADXL345_PCTL_SLEEP      (1 << 2)
#define RSCS_ADXL345_PCTL_WAKEUP(x)  ((x) & 0x3)

/* ADXL345_INT_ENABLE / ADXL345_INT_MAP / ADXL345_INT_SOURCE Определение регистра */
#define RSCS_ADXL345_DATA_READY      (1 << 7)
#define RSCS_ADXL345_SINGLE_TAP      (1 << 6)
#define RSCS_ADXL345_DOUBLE_TAP      (1 << 5)
#define RSCS_ADXL345_ACTIVITY        (1 << 4)
#define RSCS_ADXL345_INACTIVITY      (1 << 3)
#define RSCS_ADXL345_FREE_FALL       (1 << 2)
#define RSCS_ADXL345_WATERMARK       (1 << 1)
#define RSCS_ADXL345_OVERRUN         (1 << 0)

/* ADXL345_DATA_FORMAT Определение регистра */
#define RSCS_ADXL345_SELF_TEST       (1 << 7)
#define RSCS_ADXL345_SPI_BIT             (1 << 6)
#define RSCS_ADXL345_INT_INVERT      (1 << 5)
#define RSCS_ADXL345_FULL_RES        (1 << 3)
#define RSCS_ADXL345_JUSTIFY         (1 << 2)
#define RSCS_ADXL345_RANGE(x)        ((x) & 0x3)

/* ADXL345_BW_RATE Определение регистра */
#define RSCS_ADXL345_LOW_POWER       (1 << 4)
#define RSCS_ADXL345_RATE(x)         ((x) & 0xF)


/* ADXL345 Full Resolution Scale Factor */
#define RSCS_ADXL345_SCALE_FACTOR    		0.0039
#define RSCS_ADXL345_OFFSET_SCALE_FACTOR		15.6

#define GOTO_END_IF_ERROR(X) if ((error = (X)) != RSCS_E_NONE) goto end;


// описание стурктуры дескриптора
struct rscs_adxl345_t
{
	rscs_adxl345_range_t range;
	rscs_adxl345_addr_t addr;
};


/*******СЛУЖЕБНЫЕ ФУНКЦИИ*******/

#define SSPI_PORT PORTB
#define SSPI_DDR DDRB
#define SSPI_PIN PINB
#define SSPI_MISO (3)
#define SSPI_MOSI (2)
#define SSPI_CLK (1)
#define SSPI_CS (0)

//#define RSCS_SPI_PORTX	(PORTB)
//#define RSCS_SPI_DDRX	(DDRB)
//#define RSCS_SPI_MISO	(3)
//#define RSCS_SPI_MOSI	(2)
//#define RSCS_SPI_SCK	(1)
//#define RSCS_SPI_SS		(0)

/*ЧТЕНИЕ ЗНАЧЕНИЯ ИЗ РЕГИСТРА*/
rscs_e rscs_adxl345_getRegisterValue(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t * read_data)
{
	rscs_e error = RSCS_E_NONE;
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_read));
	GOTO_END_IF_ERROR(rscs_i2c_read(read_data, 1, 1));
end:
	rscs_i2c_stop();
    return error;
}


/*ЗАПИСЬ ЗНАЧЕНИЯ В РЕГИСТР*/
rscs_e rscs_adxl345_setRegisterValue(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t registerValue)
{
	rscs_e error = RSCS_E_NONE;
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerValue));
end:
	rscs_i2c_stop();
    return error;
}


/*******ФУНКЦИИ УПРАВЛЕНИЯ*******/

rscs_adxl345_t * rscs_adxl345_initi2c(rscs_i2c_addr_t addr) {
 	// создаем дескриптор
	rscs_adxl345_t * retval = (rscs_adxl345_t *)malloc(sizeof(rscs_adxl345_t));
	if (!retval)
		return retval;

	// и инициализируем
	// TODO: ADXL: Вынести первичную инициализицию в функцию startup, чтобы оттуда можно было
	// вернуть код ошибки
	retval->addr = addr;
	retval->range = RSCS_ADXL345_RANGE_2G;		//диапазон 2g (по умолчанию)

	uint8_t devid = 0;
	rscs_e error = RSCS_E_NONE;
	error = rscs_adxl345_getRegisterValue(retval, 0x00, &devid);

	//смещение по осям XYZ равно 0 (по умолчанию)
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_OFSX, 0));
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_OFSY, 0));
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_OFSZ, 0));
	//LOW_POWER off, 100Гц (по умолчанию)
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_BW_RATE,
			RSCS_ADXL345_RATE_100HZ));

	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_DATA_FORMAT,
			retval->range			// диапазон
			| RSCS_ADXL345_FULL_RES	// FULL_RES = 1 (для всех диапазонов использовать максимальное разрешение 4 mg/lsb)
	));

	//rscs_adxl345_setRegisterValue(retval, ADXL345_INT_ENABLE,		ADXL345_INT_ENABLE_DATA);	//смотри librscs_config.h
	//rscs_adxl345_setRegisterValue(retval, ADXL345_INT_MAP,		ADXL345_INT_MAP_DA(retval->interface << 6)TA);		//смотри librscs_config.h
	//rscs_adxl345_setRegisterValue(retval, ADXL345_FIFO_CTL,		ADXL345_FIFO_CTL_DATA);		//смотри librscs_config.h

	//переводит акселерометр из режима ожидания в режим измерения
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(retval, RSCS_ADXL345_POWER_CTL,	RSCS_ADXL345_PCTL_MEASURE));

end:
	if (error != RSCS_E_NONE)
	{
		if (retval)
			free(retval);
		return NULL;
	}

	return retval;
}


void rscs_adxl345_deinit(rscs_adxl345_t * device)
{
	// TODO: ADXL: напимер усыпляем акселерометр, чтобы не потреблял электричество
	// если не лень это писать конечно

	// освобождем память, занимаемую дескриптором
	free(device);
}


/* УСТАНОВКА ПРЕДЕЛОВ ИЗМЕРЕНИЙ*/
rscs_e rscs_adxl345_set_range(rscs_adxl345_t * device, rscs_adxl345_range_t range)
{
	rscs_e error = RSCS_E_NONE;
	uint8_t data = 0;

	GOTO_END_IF_ERROR(rscs_adxl345_getRegisterValue(device, RSCS_ADXL345_DATA_FORMAT, &data));
	data &= ~( (1 << 1) | 1 );			//очищаем 2 младших бита регистра BW_RATE
	data |= RSCS_ADXL345_RANGE(range);	//и записываем новое значение
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, RSCS_ADXL345_DATA_FORMAT, data));

	device->range = range;

end:
	return error;
}


/* УСТАНОВКА ЧАСТОТЫ ИЗМЕРЕНИЙ*/
rscs_e rscs_adxl345_set_rate(rscs_adxl345_t * device, rscs_adxl345_rate_t rate)
{
	rscs_e error = RSCS_E_NONE;
	uint8_t data = 0;

	GOTO_END_IF_ERROR(rscs_adxl345_getRegisterValue(device, RSCS_ADXL345_BW_RATE, &data));
	data &= ~(0xF);						//очищаем 4 младших бита регистра BW_RATE
	data |= RSCS_ADXL345_RATE(rate);	//и записываем новое значение
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, RSCS_ADXL345_BW_RATE, data));

end:
	return error;
}


/* УСТАНОВКА СМЕЩЕНИЯ РЕЗУЛЬТАТОВ ПО ОСЯМ X, Y, Z*/
rscs_e rscs_adxl345_set_offset(rscs_adxl345_t * device, float mg_x, float mg_y, float mg_z)
{
	rscs_e error = RSCS_E_NONE;

	int8_t ofs_x;
	int8_t ofs_y;
	int8_t ofs_z;

	ofs_x = (int8_t) round(mg_x / RSCS_ADXL345_OFFSET_SCALE_FACTOR);
	ofs_y = (int8_t) round(mg_y / RSCS_ADXL345_OFFSET_SCALE_FACTOR);
	ofs_z = (int8_t) round(mg_z / RSCS_ADXL345_OFFSET_SCALE_FACTOR);

	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, RSCS_ADXL345_OFSX, ofs_x));
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, RSCS_ADXL345_OFSX, ofs_y));
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, RSCS_ADXL345_OFSX, ofs_z));

end:
	return error;
}


/* ЧТЕНИЕ ДАННЫХ ADXL345 В БИНАРНОМ ВИДЕ*/
rscs_e rscs_adxl345_read(rscs_adxl345_t * device, int16_t * x, int16_t * y, int16_t * z)
{
	rscs_e error = RSCS_E_NONE;
	uint8_t readBuffer[6]   = {0};

	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(RSCS_ADXL345_DATAX0));
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_read));
	GOTO_END_IF_ERROR(rscs_i2c_read(readBuffer, sizeof(readBuffer), 1));

	*x = (readBuffer[1] << 8) | readBuffer[0];
	*y = (readBuffer[3] << 8) | readBuffer[2];
	*z = (readBuffer[5] << 8) | readBuffer[4];

end:
	rscs_i2c_stop();
	return error;
}


void rscs_adxl345_cast_to_G(rscs_adxl345_t * device, int16_t x, int16_t y, int16_t z, float * x_g, float * y_g, float * z_g) {
	/*uint8_t  range = 1;

	range = (1 << device->range);

	if(x >> 9) *x_g = -(float)(!x + 1);	//если 10-й бит равен 1, то число отрицательное
	if(x >> 9) *y_g = -(float)(!y + 1);
	if(x >> 9) *z_g = -(float)(!z + 1);

	*x_g = (*x_g) * RSCS_ADXL345_SCALE_FACTOR * range;
	*y_g = (*y_g) * RSCS_ADXL345_SCALE_FACTOR * range;
	*z_g = (*z_g) * RSCS_ADXL345_SCALE_FACTOR * range;*/

	// FIXME: ADXL: это код не похож на правильный
	*x_g = x * 0.004f;
	*y_g = y * 0.004f;
	*z_g = z * 0.004f;
}


rscs_e rscs_adxl345_GetGXYZ(rscs_adxl345_t * device, int16_t* x, int16_t* y, int16_t* z, float* x_g, float* y_g, float* z_g)
{
	*x = 0;
	*y = 0;
	*z = 0;

	rscs_e error = rscs_adxl345_read(device, x, y, z);
	if(error != RSCS_E_NONE) return error;

	rscs_adxl345_cast_to_G(device, *x, *y, *z, x_g, y_g, z_g);

	return error;
}

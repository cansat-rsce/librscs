#include "../adxl345.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../i2c.h"
#include "../spi.h"
#include "../error.h"


/* Команды на чтение и запись */
#define ADXL345_SPI_READ        (1 << 7)	//бит на чтение
#define ADXL345_SPI_WRITE       (0 << 7)	//бит на запись
#define ADXL345_SPI_MB          (1 << 6)	//бит чтение/запись нескольких байт (в противном случае только одного байта)

/* ADXL345 Адреса регистров */
#define ADXL345_DEVID			0x00 // R	Device ID
#define ADXL345_OFSX            0x1E // R/W X-axis offset.
#define ADXL345_OFSY            0x1F // R/W Y-axis offset.
#define ADXL345_OFSZ            0x20 // R/W Z-axis offset.
#define ADXL345_BW_RATE         0x2C // R/W Data rate and power mode control.
#define ADXL345_POWER_CTL       0x2D // R/W Power saving features control.
#define ADXL345_INT_ENABLE      0x2E // R/W Interrupt enable control.
#define ADXL345_INT_MAP         0x2F // R/W Interrupt mapping control.
#define ADXL345_INT_SOURCE      0x30 // R   Source of interrupts.
#define ADXL345_DATA_FORMAT     0x31 // R/W Data format control.
#define ADXL345_DATAX0          0x32 // R   X-Axis Data 0.
#define ADXL345_DATAX1          0x33 // R   X-Axis Data 1.
#define ADXL345_DATAY0          0x34 // R   Y-Axis Data 0.
#define ADXL345_DATAY1          0x35 // R   Y-Axis Data 1.
#define ADXL345_DATAZ0          0x36 // R   Z-Axis Data 0.
#define ADXL345_DATAZ1          0x37 // R   Z-Axis Data 1.
#define ADXL345_FIFO_CTL        0x38 // R/W FIFO control.
#define ADXL345_FIFO_STATUS     0x39 // R   FIFO status.

/* ADXL345_POWER_CTL Определение регистра */
#define ADXL345_PCTL_LINK       (1 << 5)
#define ADXL345_PCTL_AUTO_SLEEP (1 << 4)
#define ADXL345_PCTL_MEASURE    (1 << 3)
#define ADXL345_PCTL_SLEEP      (1 << 2)
#define ADXL345_PCTL_WAKEUP(x)  ((x) & 0x3)
/* ADXL345_INT_ENABLE / ADXL345_INT_MAP / ADXL345_INT_SOURCE Определение регистра */
#define ADXL345_DATA_READY      (1 << 7)
#define ADXL345_SINGLE_TAP      (1 << 6)
#define ADXL345_DOUBLE_TAP      (1 << 5)
#define ADXL345_ACTIVITY        (1 << 4)
#define ADXL345_INACTIVITY      (1 << 3)
#define ADXL345_FREE_FALL       (1 << 2)
#define ADXL345_WATERMARK       (1 << 1)
#define ADXL345_OVERRUN         (1 << 0)
/* ADXL345_DATA_FORMAT Определение регистра */
#define ADXL345_SELF_TEST       (1 << 7)
#define ADXL345_SPI             (1 << 6)
#define ADXL345_INT_INVERT      (1 << 5)
#define ADXL345_FULL_RES        (1 << 3)
#define ADXL345_JUSTIFY         (1 << 2)
#define ADXL345_RANGE(x)        ((x) & 0x3)

/* ADXL345 Full Resolution Scale Factor */
#define ADXL345_SCALE_FACTOR    		0.0039
#define ADXL345_OFFSET_SCALE_FACTOR		15.6

#define GOTO_END_IF_ERROR(X) if ((error = (X)) != RSCS_E_NONE) goto end;


// описание стурктуры дескриптора
struct rscs_adxl345_t
{
	rscs_adxl345_inteface_t interface;
	rscs_adxl345_addr_t addr;
	rscs_adxl345_range_t range;
	volatile uint8_t * CS_PORT;
	uint8_t CS_PIN;
};


/*******СЛУЖЕБНЫЕ ФУНКЦИИ*******/

/*Управление линией CS ADXL345 для SPI*/
void rscs_adxl345_CS_State(rscs_adxl345_t * device, bool state)
{
	if (state)
		*device->CS_PORT |= (1 << device->CS_PIN);
	else
		*device->CS_PORT &= ~(1 << device->CS_PIN);
}


/*ЧТЕНИЕ ЗНАЧЕНИЯ ИЗ РЕГИСТРА*/
rscs_e rscs_adxl345_getRegisterValue(rscs_adxl345_t * device, uint8_t registerAddress, uint16_t/*!!!*/ * read_data)
{
	rscs_e error = RSCS_E_NONE;

	switch (device->interface) {
		case RSCS_ADXL345_SPI:
			registerAddress = ADXL345_SPI_READ | registerAddress;	//дописываем в адрес бит чтения

			rscs_adxl345_CS_State(device, 0);
			rscs_spi_do(0x00);
			rscs_spi_write(&registerAddress, 1);
			rscs_spi_read(read_data, 2/*!!!*/, 0xFF);
			rscs_adxl345_CS_State(device, 1);

			break;
		case RSCS_ADXL345_I2C:
			GOTO_END_IF_ERROR(rscs_i2c_start());
			GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
			GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
			GOTO_END_IF_ERROR(rscs_i2c_start());
			GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_read));
			GOTO_END_IF_ERROR(rscs_i2c_read(read_data, 1, 1));
			end:
			rscs_i2c_stop();
			break;
		default:
			return RSCS_E_INVARG;
			break;
	}

    return error;
}


/*ЗАПИСЬ ЗНАЧЕНИЯ В РЕГИСТР*/
rscs_e rscs_adxl345_setRegisterValue(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t registerValue)
{
	rscs_e error = RSCS_E_NONE;

	switch (device->interface) {
		case RSCS_ADXL345_SPI:
			registerAddress = ADXL345_SPI_WRITE | registerAddress;	//дописываем в адрес бит записи

			rscs_adxl345_CS_State(device, 0);
			rscs_spi_write(&registerAddress, 1);
			rscs_spi_write(&registerValue, 1);
			rscs_adxl345_CS_State(device, 1);

			break;
		case RSCS_ADXL345_I2C:
			GOTO_END_IF_ERROR(rscs_i2c_start());
			GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
			GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
			GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerValue));
			end:
			rscs_i2c_stop();
			break;
		default:
			break;
	}

    return error;
}


/*******ФУНКЦИИ УПРАВЛЕНИЯ*******/

rscs_adxl345_t * rscs_adxl345_init(rscs_adxl345_inteface_t interface, rscs_adxl345_addr_t addr,
		volatile uint8_t * CS_DDR, volatile uint8_t * CS_PORT, uint8_t CS_PIN)
{
	// создаем дескриптор
	rscs_adxl345_t * retval = (rscs_adxl345_t *)malloc(sizeof(rscs_adxl345_t));
	if (!retval)
		return retval;

	// и инициализируем
	retval->interface = interface;
	retval->addr = addr;
	retval->range = RSCS_ADXL345_RANGE_2G;		//диапазон 2g (по умолчанию)
	retval->CS_PORT = CS_PORT;
	retval->CS_PIN = CS_PIN;

	*CS_DDR |= (1 << CS_PIN);		//устанавливаем пин CS на ЗАПИСЬ
	*retval->CS_PORT |= (1 << retval->CS_PIN);

	//rscs_adxl345_setRegisterValue(retval, ADXL345_OFSX,			0);		//смещение по оси X равно 0 (по умолчанию)
	//rscs_adxl345_setRegisterValue(retval, ADXL345_OFSY,			0);		//смещение по оси Y равно 0 (по умолчанию)
	//rscs_adxl345_setRegisterValue(retval, ADXL345_OFSZ,			0);		//смещение по оси Z равно 0 (по умолчанию)
	//rscs_adxl345_setRegisterValue(retval, ADXL345_BW_RATE,		RSCS_ADXL345_RATE_100HZ);	//LOW_POWER off, 100Гц (по умолчанию)
	/*rscs_adxl345_setRegisterValue(retval, ADXL345_DATA_FORMAT,	retval->range |				//диапазон 2g (по умолчанию)
																							//FULL_RES = 0 (разрешение 10 бит для любого диапазона)
																							//JUSTIFY = 0 (выравнивание бит данных по правому краю)
																						);	//интерфейс SPI-3pin или SPI-4pin
	*/
	//rscs_adxl345_setRegisterValue(retval, ADXL345_INT_ENABLE,	ADXL345_INT_ENABLE_DATA);	//смотри librscs_config.h
	//rscs_adxl345_setRegisterValue(retval, ADXL345_INT_MAP,		ADXL345_INT_MAP_DA(retval->interface << 6)TA);		//смотри librscs_config.h
	//rscs_adxl345_setRegisterValue(retval, ADXL345_FIFO_CTL,		ADXL345_FIFO_CTL_DATA);		//смотри librscs_config.h

	//rscs_adxl345_setRegisterValue(retval, ADXL345_POWER_CTL,	ADXL345_PCTL_MEASURE); //переводит акселерометр из режима ожидания в режим измерения

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

	GOTO_END_IF_ERROR(rscs_adxl345_getRegisterValue(device, ADXL345_DATA_FORMAT, &data));
	data = (data & 0x3) | ADXL345_RANGE(range);	//очищаем 2 младших бита регистра BW_RATE и записываем новое значение
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, ADXL345_DATA_FORMAT, data));

	device->range = range;

	end:
	return error;
}


/* УСТАНОВКА ЧАСТОТЫ ИЗМЕРЕНИЙ*/
rscs_e rscs_adxl345_set_rate(rscs_adxl345_t * device, rscs_adxl345_rate_t rate)
{
	rscs_e error = RSCS_E_NONE;
	uint8_t data = 0;

	GOTO_END_IF_ERROR(rscs_adxl345_getRegisterValue(device, ADXL345_BW_RATE, &data));
	data = (data & 0xF) | ADXL345_RATE(rate);	//очищаем 4 младших бита регистра BW_RATE и записываем новое значение
	GOTO_END_IF_ERROR(rscs_adxl345_setRegisterValue(device, ADXL345_BW_RATE, data));

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

	if (mg_x < 0) ofs_x = !(round(- mg_x / ADXL345_OFFSET_SCALE_FACTOR) - 1);
	if (mg_y < 0) ofs_y = !(round(- mg_y / ADXL345_OFFSET_SCALE_FACTOR) - 1);
	if (mg_z < 0) ofs_z = !(round(- mg_z / ADXL345_OFFSET_SCALE_FACTOR) - 1);

	ofs_x = round(- mg_x / ADXL345_OFFSET_SCALE_FACTOR);
	ofs_y = round(- mg_y / ADXL345_OFFSET_SCALE_FACTOR);
	ofs_z = round(- mg_z / ADXL345_OFFSET_SCALE_FACTOR);

	rscs_adxl345_setRegisterValue(device, ADXL345_OFSX, ofs_x);
	rscs_adxl345_setRegisterValue(device, ADXL345_OFSX, ofs_y);
	rscs_adxl345_setRegisterValue(device, ADXL345_OFSX, ofs_z);

	return error;
}


/* ЧТЕНИЕ ДАННЫХ ADXL345 В БИНАРНОМ ВИДЕ*/
rscs_e rscs_adxl345_read(rscs_adxl345_t * device, int16_t * x, int16_t * y, int16_t * z)
{
	rscs_e error = RSCS_E_NONE;
	uint8_t readBuffer[6]   = {0};
	uint8_t firstRegAddress = ADXL345_SPI_READ | ADXL345_SPI_MB | ADXL345_DATAX0;

	switch (device->interface) {
		case RSCS_ADXL345_SPI:
			rscs_adxl345_CS_State(device, 0);
			//_delay_ms(20);
			rscs_spi_write(&firstRegAddress, 1);
			rscs_spi_read(readBuffer, sizeof(readBuffer), 0xFF);
			rscs_adxl345_CS_State(device, 1);

			break;
		case RSCS_ADXL345_I2C:
			GOTO_END_IF_ERROR(rscs_i2c_start());
			GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_write));
			GOTO_END_IF_ERROR(rscs_i2c_write_byte(ADXL345_DATAX0));
			GOTO_END_IF_ERROR(rscs_i2c_start());
			GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->addr, rscs_i2c_slaw_read));
			GOTO_END_IF_ERROR(rscs_i2c_read(readBuffer, sizeof(readBuffer), 1));
			end:
			rscs_i2c_stop();
			break;
		default:
			break;
	}

	for (int i = 0; i < sizeof(readBuffer); i++)
	{
		printf("[%d] = %d; ", i, readBuffer[i]);
	}
	printf("\n");

	*x = (readBuffer[1] << 8) | readBuffer[0];
	*y = (readBuffer[3] << 8) | readBuffer[2];
	*z = (readBuffer[5] << 8) | readBuffer[4];

	return error;
}


/* ЧТЕНИЕ ДАННЫХ ADXL345 В БИНАРНОМ ВИДЕ И ПРЕОБРАЗОВАНИЕ В ЕДИНИЦЫ g */
void ADXL345_GetGXYZ(rscs_adxl345_t * device, int16_t* x, int16_t* y, int16_t* z, float* x_g, float* y_g, float* z_g)
{
	*x = 0;
	*y = 0;
	*z = 0;
	uint8_t  range = 1;

	for (size_t i = 0; i < (device->range); i++) range = range * 2;

	rscs_adxl345_read(device, x, y, z);

    if(*x >> 9) *x_g = -(float)(!*x + 1);	//если 10-й бит равен 1, то число отрицательное
    if(*x >> 9) *y_g = -(float)(!*y + 1);
    if(*x >> 9) *z_g = -(float)(!*z + 1);

    *x_g = (*x_g) * ADXL345_SCALE_FACTOR * range;
    *y_g = (*y_g) * ADXL345_SCALE_FACTOR * range;
    *z_g = (*z_g) * ADXL345_SCALE_FACTOR * range;
}

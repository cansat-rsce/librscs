
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <util/delay.h>

#include "../adxl345.h"
#include "../error.h"
#include "../spi.h"

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

// описание стурктуры дескриптора

struct rscs_adxl345_t
{
	rscs_adxl345_range_t range;
	union{
		struct{
			rscs_adxl345_addr_t addr;
		}i2c;
		struct{
			volatile uint8_t * CSPORT;
			uint8_t CSMASK;
		}spi;
	};

	rscs_e (*get)(rscs_adxl345_t *, uint8_t, void *, size_t);
	rscs_e (*set)(rscs_adxl345_t *, uint8_t, uint8_t);

};

#define spi_start(bus) (*bus->spi.CSPORT &= ~bus->spi.CSMASK)
#define spi_stop(bus) (*bus->spi.CSPORT |= bus->spi.CSMASK)

#define GOTO_END_IF_ERROR(X) if ((error = (X)) != RSCS_E_NONE) goto end;

rscs_e getRegisterValueSPI(rscs_adxl345_t * device, uint8_t registerAddress, void * buffer_, size_t buffer_size)
{
	uint8_t * buffer = (uint8_t *)buffer_;
	if(buffer_size - 1) registerAddress |= RSCS_ADXL345_SPI_READ | RSCS_ADXL345_SPI_MB;
	else registerAddress |= RSCS_ADXL345_SPI_READ;

	spi_start(device);

	rscs_spi_write(&registerAddress,1);
	rscs_spi_read(buffer, buffer_size, 0xFF);

	spi_stop(device);

	return RSCS_E_NONE;
}

rscs_e setRegisterValueSPI(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t registerValue)
{
	registerAddress &= ~(1<<7);

	spi_start(device);

	rscs_spi_write(&registerAddress, 1);
	rscs_spi_write(&registerValue, 1);

	spi_stop(device);

	return RSCS_E_NONE;
}

rscs_e getRegisterValueI2C(rscs_adxl345_t * device, uint8_t registerAddress, void * buffer_, size_t buffer_size)
{
	rscs_e error = RSCS_E_NONE;

	uint8_t* buffer = (uint8_t *)buffer_;

	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->i2c.addr, rscs_i2c_slaw_write));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->i2c.addr, rscs_i2c_slaw_read));
	GOTO_END_IF_ERROR(rscs_i2c_read(buffer, buffer_size, 1));
end:
	rscs_i2c_stop();
    return error;
}

rscs_e setRegisterValueI2C(rscs_adxl345_t * device, uint8_t registerAddress, uint8_t registerValue)
{
	rscs_e error = RSCS_E_NONE;
	GOTO_END_IF_ERROR(rscs_i2c_start());
	GOTO_END_IF_ERROR(rscs_i2c_send_slaw(device->i2c.addr, rscs_i2c_slaw_write));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerAddress));
	GOTO_END_IF_ERROR(rscs_i2c_write_byte(registerValue));
end:
	rscs_i2c_stop();
    return error;
}

rscs_adxl345_t* rscs_adxl345_initspi(volatile uint8_t * CSPORT, uint8_t CSPIN)
{
	rscs_adxl345_t* retval = (rscs_adxl345_t*)malloc(sizeof(rscs_adxl345_t));
	if(!retval) return NULL;

	retval->spi.CSPORT = CSPORT;
	retval->spi.CSMASK = (1 << CSPIN);

	retval->set = setRegisterValueSPI;
	retval->get = getRegisterValueSPI;

	spi_start(retval);
	_delay_us(100);
	spi_stop(retval);

	return retval;
}

rscs_adxl345_t * rscs_adxl345_initi2c(rscs_i2c_addr_t addr)
{
	rscs_adxl345_t * retval = (rscs_adxl345_t *)malloc(sizeof(rscs_adxl345_t));
	if(!retval) return NULL;

	retval->i2c.addr = addr;

	retval->set = setRegisterValueI2C;
	retval->get = getRegisterValueI2C;

	return retval;
}

rscs_e rscs_adxl345_startup(rscs_adxl345_t * adxl) {
	uint8_t devid = 0;
	adxl->get(adxl, 0x00, &devid,1);

	if(devid !=  229)  {
		return RSCS_E_INVRESP;
	}

	//смещение по осям XYZ равно 0 (по умолчанию)
	adxl->set(adxl, RSCS_ADXL345_OFSX, 0);
	adxl->set(adxl, RSCS_ADXL345_OFSY, 0);
	adxl->set(adxl, RSCS_ADXL345_OFSZ, 0);

	//LOW_POWER off, 100Гц (по умолчанию)
	adxl->set(adxl, RSCS_ADXL345_BW_RATE, RSCS_ADXL345_RATE_100HZ);

	adxl->set(adxl, RSCS_ADXL345_DATA_FORMAT,
			adxl->range			// диапазон
			| RSCS_ADXL345_FULL_RES	// FULL_RES = 1 (для всех диапазонов использовать максимальное разрешение 4 mg/lsb)
	);
	//переводит акселерометр из режима ожидания в режим измерения
	adxl->set(adxl, RSCS_ADXL345_POWER_CTL,	RSCS_ADXL345_PCTL_MEASURE);

	return RSCS_E_NONE;
}


void rscs_adxl345_deinit(rscs_adxl345_t * device)
{
	free(device);
}


/* УСТАНОВКА ПРЕДЕЛОВ ИЗМЕРЕНИЙ*/
void rscs_adxl345_set_range(rscs_adxl345_t * device, rscs_adxl345_range_t range)
{
	uint8_t data = 0;

	device->get(device, RSCS_ADXL345_DATA_FORMAT, &data, 1);
	data &= ~( (1 << 1) | 1 );			//очищаем 2 младших бита регистра BW_RATE
	data |= RSCS_ADXL345_RANGE(range);	//и записываем новое значение
	device->set(device, RSCS_ADXL345_DATA_FORMAT, data);

	device->range = range;
}


/* УСТАНОВКА ЧАСТОТЫ ИЗМЕРЕНИЙ*/
void rscs_adxl345_set_rate(rscs_adxl345_t * device, rscs_adxl345_rate_t rate)
{
	uint8_t data = 0;

	device->get(device, RSCS_ADXL345_BW_RATE, &data, 1);
	data &= ~(0xF);						//очищаем 4 младших бита регистра BW_RATE
	data |= RSCS_ADXL345_RATE(rate);	//и записываем новое значение
	device->set(device, RSCS_ADXL345_BW_RATE, data);
}


/* УСТАНОВКА СМЕЩЕНИЯ РЕЗУЛЬТАТОВ ПО ОСЯМ X, Y, Z*/
void rscs_adxl345_set_offset(rscs_adxl345_t * device, float mg_x, float mg_y, float mg_z)
{
	int8_t ofs_x;
	int8_t ofs_y;
	int8_t ofs_z;

	ofs_x = (int8_t) round(mg_x / RSCS_ADXL345_OFFSET_SCALE_FACTOR);
	ofs_y = (int8_t) round(mg_y / RSCS_ADXL345_OFFSET_SCALE_FACTOR);
	ofs_z = (int8_t) round(mg_z / RSCS_ADXL345_OFFSET_SCALE_FACTOR);

	device->set(device, RSCS_ADXL345_OFSX, ofs_x);
	device->set(device, RSCS_ADXL345_OFSX, ofs_y);
	device->set(device, RSCS_ADXL345_OFSX, ofs_z);

}


/* ЧТЕНИЕ ДАННЫХ ADXL345 В БИНАРНОМ ВИДЕ*/
void rscs_adxl345_read(rscs_adxl345_t * device, int16_t * x, int16_t * y, int16_t * z)
{
	uint8_t readBuffer[6]   = {0};

	device->get(device, RSCS_ADXL345_DATAX0, readBuffer, 6);

	*x = (readBuffer[1] << 8) | readBuffer[0];
	*y = (readBuffer[3] << 8) | readBuffer[2];
	*z = (readBuffer[5] << 8) | readBuffer[4];
}


void rscs_adxl345_cast_to_G(rscs_adxl345_t * device, int16_t x, int16_t y, int16_t z,
		float * x_g, float * y_g, float * z_g) {
	*x_g = x * 0.004f;
	*y_g = y * 0.004f;
	*z_g = z * 0.004f;
}


void rscs_adxl345_GetGXYZ(rscs_adxl345_t * device, int16_t* x, int16_t* y, int16_t* z, float* x_g, float* y_g, float* z_g)
{
	*x = 0;
	*y = 0;
	*z = 0;

	rscs_adxl345_read(device, x, y, z);
	rscs_adxl345_cast_to_G(device, *x, *y, *z, x_g, y_g, z_g);
}

// Ошибки встречающиеся в библиотеке

#ifndef RSCS_ERROR_H_
#define RSCS_ERROR_H_

typedef enum
{
	RSCS_E_NONE			=  0,  // все в порядке

	RSCS_E_INVARG	= -1,  // недопустимый аргумент передан функции
	RSCS_E_INVRESP	= -2,  // получен неверный ответ от устройства - наприимер I2C N-ACK
	RSCS_E_TIMEOUT	= -3,  // истек таймаут даннный операции
	RSCS_E_IO		= -4,  // что-то не так с передачей данных. Например потеря арбитража I2C шины
	RSCS_E_BUSY		= -5,  // вызываемое устройство занято
	RSCS_E_NODEVICE = -6,  // нет такого устройства
	RSCS_E_CHKSUM	= -7,  // ошибка контрольной суммы
	RSCS_E_NULL 	= -8,  // ошибка, связанная с нулём (NPE, divide by zero и т.д.)
} rscs_e ;


#endif /* RSCS_ERROR_H_ */

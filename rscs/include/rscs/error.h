// Ошибки встречающиеся в библиотеке

#ifndef ERROR_H_
#define ERROR_H_

typedef enum
{
	RSCS_E_NONE			=  0,  // все в порядке

	RSCS_E_INVARG	= -1,  // недопустимый аргумент передан функции
	RSCS_E_INVRESP	= -2,  // получен неверный ответ от устройства - наприимер I2C N-ACK
	RSCS_E_TIMEOUT	= -3,  // истек таймаут даннный операции
	RSCS_E_IO		= -3,  // что-то не так с передачей данных. Например потеря арбитража I2C шины


} rscs_error_t;


#endif /* ERROR_H_ */

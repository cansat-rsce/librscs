#ifndef ADC_H_
#define ADC_H_

#include "error.h"


/*Идентификаторы каналов
 * Волшебные числа _ то, что надо записать в ADMUX для включения канала*/
typedef enum {
	//Одиночные каналы
	ADC_SINGLE_0 		= 0,
	ADC_SINGLE_1 		= 1,
	ADC_SINGLE_2 		= 2,
	ADC_SINGLE_3 		= 3,
	ADC_SINGLE_4 		= 4,
	ADC_SINGLE_5 		= 5,
	ADC_SINGLE_6 		= 6,
	ADC_SINGLE_7 		= 7,
#ifdef __AVR_ATmega128__
	/*Дифференциальны каналы
	 * (названия вида ADC_DIFF_положительныйканал_отрицательныйканал_множитель)*/
	ADC_DIFF_0_0_10X	= 8,
	ADC_DIFF_1_0_10X	= 9,
	ADC_DIFF_0_0_200X	= 10,
	ADC_DIFF_1_0_200X	= 11,
	ADC_DIFF_2_2_10X	= 12,
	ADC_DIFF_3_2_10X	= 13,
	ADC_DIFF_2_2_200X	= 14,
	ADC_DIFF_3_2_200X	= 15,
	ADC_DIFF_0_1_1X		= 16,
	ADC_DIFF_1_1_1X		= 17,
	ADC_DIFF_2_1_1X		= 18,
	ADC_DIFF_3_1_1X		= 19,
	ADC_DIFF_4_1_1X		= 20,
	ADC_DIFF_5_1_1X		= 21,
	ADC_DIFF_6_1_1X		= 22,
	ADC_DIFF_7_1_1X		= 23,
	ADC_DIFF_0_2_1X		= 24,
	ADC_DIFF_1_2_1X		= 25,
	ADC_DIFF_2_2_1X		= 26,
	ADC_DIFF_3_2_1X		= 27,
	ADC_DIFF_4_2_1X		= 28

#endif

} adc_channel;


//Определение верхней границы индексов каналов внутреннего АЦП
#ifdef __AVR_ATmega128__

#define ADC_INTERNAL_HIGHEND ADC_DIFF_4_2_1X

#elif defined __AVR_ATmega328P__

#define ADC_INTERNAL_HIGHEND ADC_SINGLE_7

#endif

//Перечисление предделителей АЦП
typedef enum {
	ADC_PRESCALER_2 = 1,
	ADC_PRESCALER_4 = 2,
	ADC_PRESCALER_8 = 3,
	ADC_PRESCALER_16 = 4,
	ADC_PRESCALER_32 = 5,
	ADC_PRESCALER_64 = 6,
	ADC_PRESCALER_128 = 7
} adc_prescaler;

//Дескриптор АЦП
typedef struct adc_descriptor_t adc_descriptor_t;
struct adc_descriptor_t{
	rscs_e (*init) (adc_descriptor_t *);//Указатель на функцию инициализации АЦП
	rscs_e (*startConversion) (adc_descriptor_t *);//Указатель на функцию начала измерения
	int32_t (*getResult) (adc_descriptor_t *);//Указатель на функцию начала измерения
	adc_prescaler prescaler;//Предделитель
	adc_channel channel;//Канал измерения

};

/*Служит для заполнения функций в дескрипторе, перед вызовом функции нужно в
 * передаваемом дескрипторе заполнить поле канала.
 * Для внутреннего АЦП подберёт adc_internal_init(), adc_internal_startConversion
 * и adc_internal_getResult*/
void adc_descriptor_init(adc_descriptor_t * descriptor_p);

/*Функция для инициализации внутреннего АЦП. Нужно вызвать минимум один раз,
 * неоднократный вызов не запрещён*/
rscs_e adc_internal_init(adc_descriptor_t * descriptor_p);

/*Функция для начала измерения на внутреннем ацп. Результат можно получить с помощью
 * getResult() в дескрипторе*/
rscs_e adc_internal_startConversion(adc_descriptor_t * descriptor_p);

/*Функция для получения результатов с внутреннего АЦП. Возвращает результат или
 * код ошибки:
 * Вернёт RSCS_E_BUSY, если результат измерения не готов
 * Вернёт RSCS_E_INVARG, если канал, для которого есть результат, не соответствует
 * каналу, указанному в дескрипторе, а также если нет нового результата*/
int32_t adc_internal_getResult(adc_descriptor_t * descriptor_p);

#endif /* ADC_H_ */

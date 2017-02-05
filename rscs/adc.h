#ifndef ADC_H_
#define ADC_H_

#include "error.h"
#include "stdint.h"

/*! Идентификаторы каналов
    Волшебные числа _ то, что надо записать в ADMUX для включения канала*/
typedef enum {
	//Одиночные каналы
	RSCS_ADC_SINGLE_0 		= 0,
	RSCS_ADC_SINGLE_1 		= 1,
	RSCS_ADC_SINGLE_2 		= 2,
	RSCS_ADC_SINGLE_3 		= 3,
	RSCS_ADC_SINGLE_4 		= 4,
	RSCS_ADC_SINGLE_5 		= 5,
	RSCS_ADC_SINGLE_6 		= 6,
	RSCS_ADC_SINGLE_7 		= 7,
#ifdef __AVR_ATmega128__
	/*Дифференциальны каналы
	 * (названия вида ADC_DIFF_положительныйканал_отрицательныйканал_множитель)*/
	RSCS_ADC_DIFF_0_0_10X		= 8,
	RSCS_ADC_DIFF_1_0_10X		= 9,
	RSCS_ADC_DIFF_0_0_200X		= 10,
	RSCS_ADC_DIFF_1_0_200X		= 11,
	RSCS_ADC_DIFF_2_2_10X		= 12,
	RSCS_ADC_DIFF_3_2_10X		= 13,
	RSCS_ADC_DIFF_2_2_200X		= 14,
	RSCS_ADC_DIFF_3_2_200X		= 15,
	RSCS_ADC_DIFF_0_1_1X		= 16,
	RSCS_ADC_DIFF_1_1_1X		= 17,
	RSCS_ADC_DIFF_2_1_1X		= 18,
	RSCS_ADC_DIFF_3_1_1X		= 19,
	RSCS_ADC_DIFF_4_1_1X		= 20,
	RSCS_ADC_DIFF_5_1_1X		= 21,
	RSCS_ADC_DIFF_6_1_1X		= 22,
	RSCS_ADC_DIFF_7_1_1X		= 23,
	RSCS_ADC_DIFF_0_2_1X		= 24,
	RSCS_ADC_DIFF_1_2_1X		= 25,
	RSCS_ADC_DIFF_2_2_1X		= 26,
	RSCS_ADC_DIFF_3_2_1X		= 27,
	RSCS_ADC_DIFF_4_2_1X		= 28
#endif

} rscs_adc_channel_t;

//! Перечисление предделителей АЦП
typedef enum {
	RSCS_ADC_PRESCALER_2 	= 1,
	RSCS_ADC_PRESCALER_4 	= 2,
	RSCS_ADC_PRESCALER_8 	= 3,
	RSCS_ADC_PRESCALER_16 	= 4,
	RSCS_ADC_PRESCALER_32 	= 5,
	RSCS_ADC_PRESCALER_64 	= 6,
	RSCS_ADC_PRESCALER_128 	= 7
} rscs_adc_prescaler_t;

//! Опорные напряжения АЦП
typedef enum {
	//! Внешний пин AREF (нельзя использовать на конструкторе)
	RSCS_ADC_REF_EXTERNAL_AREF	= 0,
	//! Напряжение питания, требуется конденсатор на AREF (есть на конструкторе)
	RSCS_ADC_REF_EXTERNAL_VCC	= 1,
	//! Внутренний источник на 2.56 вольта. Требуется конденсатор на AREF (есть на конструкторе)
	RSCS_ADC_REF_INTERNAL_2DOT56= 3,
} rscs_adc_ref_t;

//! Режим работы АЦП - непрерывный или одиночный
typedef enum {
	RSCS_ADC_MODE_SINGLE 	= 0, 		//!< Одиночный
	RSCS_ADC_MODE_CONTINIOUS= 1,	//!< Непрерывный
} rscs_adc_mode_t;

//! Инициализация АЦП
/*! Параметры по-умолчанию:
    - опорное напряжеие - RSCS_ADC_REF_EXTERNAL_VCC
    - предделитель - 64 на 8МГц, 128 на 16МГц
    - режим - одиночный
    */
void rscs_adc_init();

//! Установка опорного напряжения
// 	от изменения значений до их применения может пройти некоторое время(см. даташит)
void rscs_adc_set_refrence(rscs_adc_ref_t ref);

//! Установка предделителя.
/*! На частоте в 16МГц для использования полного разрешения АЦП
	необходимо использователь предделитель не менее чем 128
	на частоте 8 МГц - не менее чем 64
	от изменения значений до их применения может пройти некоторое время(см. даташит)*/
void rscs_adc_set_prescaler(rscs_adc_prescaler_t presc);

#ifdef __AVR_ATmega128__
//! Установка режима (только ATmega128)
//  от изменения значений до их применения может пройти некоторое время(см. даташит)
void rscs_adc_set_mode(rscs_adc_mode_t mode);
#endif

//! Начало измерения. Вернет RSCS_E_BUSY если, уже есть измерение в процессе.
rscs_e rscs_adc_start_conversion(rscs_adc_channel_t channel);

//! Получение результата измерения.
/*! Если нет свежих данных, вернет RSCS_E_BUSY*/
rscs_e rscs_adc_get_result(int32_t * value_ptr, rscs_adc_channel_t channel);

#endif /* ADC_H_ */

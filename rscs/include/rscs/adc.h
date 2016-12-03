#ifndef ADC_H_
#define ADC_H_

#include "error.h"


//Идентификатор каналов
//Волшебные числа _ то, что надо записать в ADMUX для включения канала
typedef enum {

	ADC_SINGLE_0 		= 0,
	ADC_SINGLE_1 		= 1,
	ADC_SINGLE_2 		= 2,
	ADC_SINGLE_3 		= 3,
	ADC_SINGLE_4 		= 4,
	ADC_SINGLE_5 		= 5,
	ADC_SINGLE_6 		= 6,
	ADC_SINGLE_7 		= 7,
#ifdef __AVR_ATmega128__
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

#ifdef __AVR_ATmega128__

#define ADC_INTERNAL_HIGHEND ADC_DIFF_4_2_1X

#elif defined __AVR_ATmega328P__

#define ADC_INTERNAL_HIGHEND

#endif

typedef enum {
	ADC_PRESCALER_2 = 1,
	ADC_PRESCALER_4 = 2,
	ADC_PRESCALER_8 = 3,
	ADC_PRESCALER_16 = 4,
	ADC_PRESCALER_32 = 5,
	ADC_PRESCALER_64 = 6,
	ADC_PRESCALER_128 = 7
} adc_prescaler;


typedef struct adc_descriptor_t adc_descriptor_t;
struct adc_descriptor_t{
	rscs_e (*init) (adc_descriptor_t *);
	rscs_e (*startConversion) (adc_descriptor_t *);
	int32_t (*getResult) (adc_descriptor_t *);
	adc_prescaler prescaler;
	adc_channel channel;

};

void adc_descriptor_init(adc_descriptor_t * descriptor_p);

rscs_e adc_internal_init(adc_descriptor_t * descriptor_p);
rscs_e adc_internal_startConversion(adc_descriptor_t * descriptor_p);
int32_t adc_internal_getResult(adc_descriptor_t * descriptor_p);

#endif /* ADC_H_ */

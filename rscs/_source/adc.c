#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "../adc.h"
#include "../error.h"

static bool _adc_internal_needinit = true;
static rscs_adc_channel_t _current_channel = -1;

void rscs_adc_init(){

	if(_adc_internal_needinit){
			ADMUX = (RSCS_ADC_REF_EXTERNAL_VCC << REFS0) // опорное напряжение на AVCC
				| (0 << ADLAR) // Левосторонний формат результата
			;
			ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) |
#ifdef __AVR_ATmega128__
					RSCS_ADC_PRESCALER_64;
#elif defined __AVR_ATmega328P__
					RSCS_ADC_PRESCALER_128;
#endif
			_adc_internal_needinit = false;
	}
}

rscs_e rscs_adc_start_single_conversion(rscs_adc_channel_t channel){

	if(!(ADCSRA & (1 << ADSC))) {

#ifdef __AVR_ATmega128__
		ADCSRA &= ~(1 << ADFR);
#elif defined __AVR_ATmega328P__
		ADCSRA &= ~(1 << ADATE);
#endif

		ADMUX &= ~((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3));

		ADMUX |= channel;

		ADCSRA |= (1 << ADSC);

		_current_channel = channel;

		return RSCS_E_NONE;
	}

	else {
		return RSCS_E_BUSY;
	}
}

rscs_e rscs_adc_start_continuous_conversion(rscs_adc_channel_t channel) {

	if(!(ADCSRA & (1 << ADSC))) {

#ifdef __AVR_ATmega128__
		ADCSRA |= (1 << ADFR);
#elif defined __AVR_ATmega328P__
		ADCSRA |= (1 << ADATE);
#endif

		ADMUX &= ~((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3));

		ADMUX |= channel;

		ADCSRA |= (1 << ADSC);

		_current_channel = channel;

		return RSCS_E_NONE;
	}

	else {
		return RSCS_E_BUSY;
	}
}

void rscs_adc_stop_continuous_conversion() {
#ifdef __AVR_ATmega128__
		ADCSRA &= ~(1 << ADFR);
#elif defined __AVR_ATmega328P__
		ADCSRA &= ~(1 << ADATE);
#endif
}

void rscs_adc_set_refrence(rscs_adc_ref_t ref){
	ADMUX &= ~(1 << REFS0) | (1 << REFS1);

	ADMUX |= (ref << REFS0);
}

void rscs_adc_set_prescaler(rscs_adc_prescaler_t presc){
	ADCSRA &= ~((1 << ADPS0)|(1 << ADPS1)|(1 << ADPS2));

	ADCSRA |= presc;
}

rscs_e rscs_adc_get_result(int32_t * value_ptr) {

	if(!(ADCSRA & (1 << ADIF))) return RSCS_E_BUSY;

	*value_ptr = ADC;

	return RSCS_E_NONE;
}

void rscs_adc_wait_result() {
	while(!(ADCSRA & (1 << ADIF))){}
}

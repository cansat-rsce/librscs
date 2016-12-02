#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "../include/rscs/adc.h"
#include "../include/rscs/error.h"

adc_descriptor_t current_channel;

ISR (ADC_vect) {
	* (current_channel.result_pointer) = ADC;
}

bool adc_internal_needinit = true;

rscs_e adc_internal_init(adc_descriptor_t descriptor){

	if(adc_internal_needinit){
			ADMUX = (1 << REFS0) | (0 << REFS1) // опорное напряжение на AVCC
				| (0 << ADLAR) // Левосторонний формат результата
			;
			ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADIE) |
					descriptor.prescaler;
			adc_internal_needinit = false;
	}

	return RSCS_E_NONE;
}

//Результат измерения будет помещён в переменную, указанную в дескрипторе
//Вернёт RSCS_E_TIMEOUT если ADC занят
rscs_e adc_internal_read(adc_descriptor_t descriptor){

	if(!(ADCSRA & (1 << ADSC))) {

		ADMUX &= ~((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3));

		ADMUX |= descriptor.channel;

		ADCSRA |= (1 << ADSC);

		current_channel = descriptor;

		return RSCS_E_NONE;
	}

	else {
		return RSCS_E_TIMEOUT;
	}
}

void adc_descriptor_init(adc_descriptor_t * descriptor_p){

	if(!(descriptor_p->channel > ADC_INTERNAL_HIGHEND)) {

		descriptor_p->init = adc_internal_init;
		descriptor_p->read = adc_internal_read;
	}
}

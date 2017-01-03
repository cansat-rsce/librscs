#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "../include/rscs/adc.h"
#include "../include/rscs/error.h"

adc_descriptor_t current_channel;

bool convertion_started = false;

bool adc_internal_needinit = true;

rscs_e adc_internal_init(adc_descriptor_t * descriptor_p){

	if(adc_internal_needinit){
			ADMUX = (1 << REFS0) | (0 << REFS1) // опорное напряжение на AVCC
				| (0 << ADLAR) // Левосторонний формат результата
			;
			ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADIE) |
					descriptor_p->prescaler;
			adc_internal_needinit = false;
	}

	return RSCS_E_NONE;
}

rscs_e adc_internal_startConversion(adc_descriptor_t * descriptor_p){

	if(!(ADCSRA & (1 << ADSC))) {

		ADMUX &= ~((1 << MUX0) | (1 << MUX1) | (1 << MUX2) | (1 << MUX3));

		ADMUX |= descriptor_p->channel;

		ADCSRA |= (1 << ADSC);

		convertion_started = true;

		current_channel = *descriptor_p;

		return RSCS_E_NONE;
	}

	else {
		return RSCS_E_BUSY;
	}
}

int32_t adc_internal_getResult(adc_descriptor_t * descriptor_p) {

	if(!convertion_started) return RSCS_E_INVARG;

	if(!(ADCSRA & (1 << ADIF))) return RSCS_E_BUSY;

	if(descriptor_p->channel != current_channel.channel) return RSCS_E_INVARG;

	convertion_started = false;
	return ADC;
}

void adc_descriptor_init(adc_descriptor_t * descriptor_p){

	if(!(descriptor_p->channel > ADC_INTERNAL_HIGHEND)) {

		descriptor_p->init = adc_internal_init;
		descriptor_p->startConversion = adc_internal_startConversion;
		descriptor_p->getResult = adc_internal_getResult;
	}
}

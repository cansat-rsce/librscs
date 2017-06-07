#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "librscs_config.h"

#include "../timeservice.h"

bool _rscs_time_needinit = true;

volatile uint32_t _m_seconds = 0;

#ifdef __AVR_ATmega128__
ISR (TIMER3_COMPA_vect) {
#elif defined __AVR_ATmega328P__
ISR (TIMER0_COMPA_vect) {
#endif
	_m_seconds++;
}

void rscs_time_init() {
	if(_rscs_time_needinit) {
		_m_seconds = 0;
#ifdef __AVR_ATmega128__
		TCCR3A = 0;
		TCCR3B = (1 << WGM32)|(0 << WGM33);
		OCR3A = 125;
		ETIMSK = (1 << OCIE3A);
		TCCR3B |= (1 << CS31) | (1 << CS30);
#elif defined __AVR_ATmega328P__
		TCCR0A = (1 << WGM01);
		OCR0A = 250;
		TIMSK0 = (1 << OCIE0A);
		TCCR0B |= (1 << CS01) | (1 << CS00);
#endif
		_rscs_time_needinit = false;
	}
}

void rscs_time_deinit() {
	if(!_rscs_time_needinit) {
#ifdef __AVR_ATmega128__
		TCCR3A = 0;
		TCCR3B = 0;
		OCR3A = 0;
		TIMSK &= ~(1 << OCIE3A);
#elif defined __AVR_ATmega328P__
		TCCR0A = 0;
		TCCR0B = 0;
		OCR0A = 0;
		TIMSK0 &= ~(1 << OCIE0A);
#endif
		_rscs_time_needinit = true;
	}
}

uint32_t rscs_time_get() {

	uint32_t temp_ms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		temp_ms = _m_seconds;
	}
	return temp_ms;
}

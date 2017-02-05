#include "../time.h"

#include <stdint.h>
#include <util/delay.h>
#include <stdbool.h>

#include "avr/io.h"
#include "avr/interrupt.h"

#include "librscs_config.h"


bool _rscs_time_needinit = true;

volatile uint16_t _seconds = 0;

ISR (TIMER3_COMPA_vect) {
	 _seconds++;
}

void rscs_time_init() {
	if(_rscs_time_needinit) {
		_seconds = 0;
		TCCR3A= 0;
		TCCR3B = (1 << WGM32)|(0 << WGM33);
		OCR3A = RSCS_TIME_MAXSUBSECONDS;
		TIMSK = (1 << OCIE3A);
		TCCR3B |= (1 << CS32);
		_rscs_time_needinit = false;
	}
}

void rscs_time_deinit() {
	if(!_rscs_time_needinit) {
		TCCR3A = 0;
		TCCR3B = 0;
		OCR3A = 0;
		TIMSK &= ~(1 << OCIE3A);
		_rscs_time_needinit = true;
	}
}

rscs_time_t rscs_time_get() {
	rscs_time_t _time;
	cli();

	_time.subseconds = TCNT3;
	_time.seconds = _seconds ;

	sei();

	return _time;
}

rscs_time_t rscs_time_summ(rscs_time_t a, rscs_time_t b) {
	rscs_time_t _summ;
	_summ.seconds = a.seconds + b.seconds;
	_summ.subseconds = a.subseconds + b.subseconds;
	if (_summ.subseconds >= RSCS_TIME_MAXSUBSECONDS) {
		_summ.seconds = _summ.seconds + 1;
		_summ.subseconds = _summ.subseconds - RSCS_TIME_MAXSUBSECONDS;
	}
	return _summ;
}

bool rscs_time_compare(rscs_time_t a, rscs_time_t b) {
	if(a.seconds > b.seconds) return true;

	else if(a.seconds == b.seconds && a.subseconds > b.subseconds) return true;

	return false;
}

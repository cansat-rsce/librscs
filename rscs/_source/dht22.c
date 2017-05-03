#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "librscs_config.h"
#include "../error.h"

#include "../dht22.h"

struct rscs_dht22_t {
	volatile uint8_t * PORTREG, * PINREG, * DDRREG;
	uint8_t PIN;
	uint8_t signal_time; //У каждого DHT22 очень сильно изменяется время, обозначающее передачу нуля или единицы. Этот коэффициент показывает, во сколько раз реальные длительности отличаются от предполагаемых (28us для '0' и 70us для '1').
};

rscs_dht22_t *  rscs_dht22_init(volatile uint8_t * PORTREG, volatile uint8_t * PINREG, volatile uint8_t * DDRREG, uint8_t PIN, float signal_time_divisor)
{
	rscs_dht22_t * dht = malloc(sizeof(rscs_dht22_t));
	dht->PORTREG = PORTREG;
	dht->PINREG = PINREG;
	dht->DDRREG = DDRREG;
	dht->PIN = PIN;
	dht->signal_time = (uint8_t)(35 / signal_time_divisor);
	*(dht->PORTREG) &= ~(1 << dht->PIN);
	*(dht->DDRREG) &= ~(1 << dht->PIN);
	return dht;
}


inline static void _set_bus_zero(rscs_dht22_t * dht)
{
	*(dht->DDRREG) |= (1 << (dht->PIN));
}


inline static void _set_bus_one(rscs_dht22_t * dht)
{
	*(dht->DDRREG) &= ~(1 << (dht->PIN));
}


inline static int _read_bus(rscs_dht22_t * dht)
{
	if ((*(dht->PINREG) & (1 << (dht->PIN))) != 0)
		return 1;
	else
		return 0;
}


// начало связи с dht
// если все хорошо возвращает ноль
// если устроиство не отвечает то возвращает RSCS_E_NODEVICE
// если линия не поднялась после ответа за заданное время возвращает RSCS_E_TIMEOUT
inline static rscs_e _reset(rscs_dht22_t * dht)
{
	_set_bus_zero(dht);
	_delay_us(1500);
	_set_bus_one(dht);
	_delay_us(10);

 	bool isSomeoneHere = false;
	for (int i = 0; i < 30+80; i++)
	{
		if (_read_bus(dht) == 0)
		{
			isSomeoneHere = true;
			break;
		}

		_delay_us(1);
	}

	if (!isSomeoneHere)
		return RSCS_E_NODEVICE;

	for (int i = 0; i < 120; i++)
	{
		if(_read_bus(dht) != 0 )
			return RSCS_E_NONE;
	}

	return RSCS_E_TIMEOUT;
}


inline static rscs_e _wait_start_bit(rscs_dht22_t * dht){

	int value = 0;

	for(int i = 0; i < 100; i++){
		if(_read_bus(dht) == 0){
			value = 1;
			break;
		}
		_delay_us(1);
	}

	if(!value) return RSCS_E_TIMEOUT;
	return RSCS_E_NONE;
}


// если значение отрицательное, то это код ошибки:
// 		RSCS_E_TIMEOUT - истечение времени ожидания опускания или подъёма линии
// из положительных значений возвращает 0 или 1
inline static int _read_bit(rscs_dht22_t * dht)
{
	register uint8_t bitStartedOrEnded = false;
	for (uint8_t i = 0; i < 50; i++)
	{
		_delay_us(1);
		if (_read_bus(dht) != 0)
		{
			bitStartedOrEnded = true;
			break;
		}
	}

	if (!bitStartedOrEnded) return RSCS_E_TIMEOUT;

	bitStartedOrEnded = false;
	uint8_t i;
	for (i = 0; i < 100; i++){
		_delay_us(1);
		if (_read_bus(dht) == 0)
		{
			bitStartedOrEnded = true;
			break;
		}
	}

	if (!bitStartedOrEnded)
		return RSCS_E_TIMEOUT;

	if(i > dht->signal_time) {
		//printf("111111111`11111\n");
		return 1;
	}
	else return 0;
}


inline static rscs_e _read_byte(rscs_dht22_t * dht){
	int num[8];
	for(int i = 0; i < 8; i++){
		int value = _read_bit(dht);
		if (value < 0)
			return value;

		num[i] = value;
	}

	uint8_t retval = 0;
	retval = num[0] << 7;


	for(int i = 1, j = 6; j >= 0; i++ , j--){
		retval = retval | num[i] << j;
	}

	return retval;
}


rscs_e rscs_dht22_read(rscs_dht22_t * dht, uint16_t * humidity, int16_t * temp)
{
	rscs_e error = RSCS_E_NONE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		rscs_e reset_status = _reset(dht);
		if (reset_status != RSCS_E_NONE)
			error = reset_status;

		else {
			rscs_e wait_start_status = _wait_start_bit(dht);
			if (wait_start_status != RSCS_E_NONE)
				error = wait_start_status;

			else {
				uint8_t sum[5];
				bool needcontinue = true;
				for( int i = 0; i<5; i++){
					sum[i] = _read_byte(dht);
					if (sum[i] < 0){
						error = sum[i];
						needcontinue = false;
						break;
					}
				}

				if(needcontinue) {
					uint8_t* tempptr = (uint8_t*)temp;
					*(tempptr + 0) = sum[2];
					*(tempptr + 1) = sum[3];

					*humidity = (sum[0] << 8) | sum[1];
					*temp = (sum[2] << 8) | sum[3];

					if (((sum[0] + sum[1] + sum[2] + sum[3]) & 0xFF) != sum[4])
						error = RSCS_E_CHKSUM;
				}
			}
		}
	}

	return error;
}

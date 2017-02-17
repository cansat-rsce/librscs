#include <stdlib.h>

#include <avr/io.h>

#include "librscs_config.h"

#include "../servo.h"

struct rscs_servo;
typedef struct rscs_servo rscs_servo;


struct rscs_servo{
	uint8_t id;
	uint8_t mask;
	int ocr;
	rscs_servo * next;
};

rscs_servo * head;
rscs_servo * current;
uint8_t update_servo;
uint8_t new_angle;
uint8_t new_angle_n;

static inline void _init_servo(int id, rscs_servo * servo)
{
	servo->id = 0;
	servo->mask = (1 << id);
	servo->ocr = 0;
}

void _include_servo(rscs_servo *in)
{
	if(in->ocr <= head->ocr)
	{
		in->next = head;
		head = in;
	}
	else
	{
		rscs_servo *tmp = head;
		while(tmp->next != NULL && tmp->next->ocr < in->ocr) //находим серву с наибольшим ocr < in->ocr
		{
			tmp = tmp->next;
		}
		if(tmp->next == NULL)
		{
			tmp->next = in;
		}
		else
		{
			rscs_servo *buf = tmp->next;
			tmp->next = in;
			in->next = buf;
		}
	}
}
static rscs_servo * _exclude_servo_by_id(int id)
{
	rscs_servo *tmp = head;
	if(tmp->id == id)
	{
		head = tmp->next;
		return tmp;
	}
	while(tmp != NULL && tmp->id != id)
	{
		tmp = tmp->next;
	}
	if(tmp != NULL)
	{
		rscs_servo *res = tmp->next;
		tmp->next = tmp->next->next;
		return res;
	}
	return NULL;
}


void rscs_servo_init(int n)
{
	head = malloc(sizeof(rscs_servo));
	_init_servo(0, head);
	rscs_servo * temp = head;
	for(int i = 1; i < n; i++)
	{
		temp->next = malloc(sizeof(rscs_servo));
		temp = temp->next;
		_init_servo(i, temp);
	}
	current = head;
    OCR0 = current->ocr;
}

void rscs_set_angle(int n, int angle)
{
	new_angle_n = n;
	new_angle = angle;
	update_servo = 1;
}

void _set_angle(int n, int ocr)
{
	rscs_servo *tmp = _exclude_servo_by_id(n);
	tmp->ocr = ocr;
	_include_servo(tmp);
}


ISR(TIMER0_COMP_vect)
{
		PORTA &= ~current->mask;
		current = current->next;

		if(current == NULL)
		{
			if(update_servo)
			{
				update_servo = 0;
				_set_angle(new_angle_n, new_angle);
			}
			current = head;
		}

        OCR0 = current->ocr;
}

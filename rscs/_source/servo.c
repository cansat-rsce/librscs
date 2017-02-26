#include <stdlib.h>

#include <util/atomic.h>

#include <avr/io.h>

#include "librscs_config.h"

#include "../servo.h"

struct rscs_servo;
typedef struct rscs_servo rscs_servo;


struct rscs_servo{
	uint8_t id;
	uint8_t mask;
	int ocr;
	int new_ocr;
	rscs_servo * next;
};

rscs_servo * head;
rscs_servo * current;

void _timer_int();

static inline void _init_servo(int id, rscs_servo * servo)
{
	servo->id = 0;
	servo->mask = (1 << id);
	servo->ocr = 0;
	servo->new_ocr = -1;
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
static void _exclude_servo(rscs_servo *tmp)
{
	if(tmp == head)
	{
		head = tmp->next;
		return;
	}
	rscs_servo *previous = head;
	while(previous->next != tmp)
	{
		previous = previous->next;
	}
	previous->next = tmp->next;
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

void rscs_servo_set_angle(int n, int angle)
{
	rscs_servo *t = head;
	while(t != NULL && t->id != n)
	{
		t = t->next;
	}
	if(t == NULL) { return;}
	t->new_ocr = angle;
}

int _set_angle(rscs_servo *servo)
{
	if(servo->new_ocr >= 0)
	{
		_exclude_servo(servo);
		servo->ocr = servo->new_ocr;
		servo->new_ocr = -1;
		_include_servo(servo);
		return 1;
	}
	else
	{
		return 0;
	}
}


ISR(TIMER0_COMP_vect)
{
        do
        {
		PORTA &= ~current->mask;
		current = current->next;
        }while(current->next != NULL && 
                current->next->ocr == current->ocr);

		if(current == NULL)
		{
			current = head;
			while(current != NULL)
			{
				if(_set_angle(current))
				{
					current = head;
				}
				current = current->next;
			}
			current = head;
		}

        OCR0 = current->ocr;
}

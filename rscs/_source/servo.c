#include <stdlib.h>

#include <avr/io.h>

#include "librscs_config.h"

#include "../servo.h"


#define SERVO_TIM1_A_PWM_MASK ((1<<COM1A1) | (0<<COM1A0))
#define SERVO_TIM1_B_PWM_MASK ((1<<COM1B1) | (0<<COM1B0))


struct rscs_servo {
	volatile uint16_t *OCR;
	int min_angle_ms;
	int max_angle_ms;
};


rscs_servo_t * rscs_servo_init(rscs_servo_id_t id)
{
	volatile uint16_t * target_ocr_reg = NULL;

	if (id == RSCS_SERVO_ID_TIM1_A)
	{
		// а вдруг мы уже создавали дескриптор этой сервы?
		if (TCCR1A & SERVO_TIM1_A_PWM_MASK)
			return NULL;

		DDRB |= (1 << 1);
		target_ocr_reg = &OCR1A;
		TCCR1A |= SERVO_TIM1_A_PWM_MASK;
	}
	else if (id == RSCS_SERVO_ID_TIM1_B)
	{
		// а вдруг мы уже создавали дескриптор этой сервы?
		if (TCCR1A & SERVO_TIM1_B_PWM_MASK)
			return NULL;

		DDRB |= (1 << 2);
		target_ocr_reg = &OCR1B;
		TCCR1A |= SERVO_TIM1_B_PWM_MASK;
	}
	else
	{
		// мы пока не поддерживаем серв, расположенных не на таймере 1
		return NULL;
	}


	// так, ну вроде определились с настройками
	// запускаем таймер 1
#if F_CPU == 8000000
	ICR1 = 80000; // FIXME: Это очевидно не верно, так как поле принимает максимум 65535. Исправить!
#elif F_CPU == 16000000
	ICR1 = 40000;
#else
#error "Wrong F_CPU value"
#endif
	TCCR1A |= (0<<WGM10) | (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13)
			| (0<<CS12) | (1<<CS11) | (0<<CS10);

	// создаем дескриптор сервы и настраиваем
	rscs_servo_t * servo = (rscs_servo_t * )malloc(sizeof(rscs_servo_t));
	if (NULL == servo)
			return servo;

	servo->OCR = target_ocr_reg;

	// возвращаем
	return servo;
}


void rscs_servo_deinit(rscs_servo_t * servo)
{
	// смотрим какой это канал и выключаем ШИП на соответствующем пине
	if (servo->OCR == &OCR1A)
		TCCR1A &= ~((1<<COM1A1) | (0<<COM1A0));
	else if (servo->OCR == &OCR1B)
		TCCR1A &= ~((1<<COM1B1) | (0<<COM1B0));

	// если обе сервы с таймера 1 ушли - можно его вообще остановить
	if (0 == (TCCR1A & (SERVO_TIM1_A_PWM_MASK | SERVO_TIM1_B_PWM_MASK)))
	{
		TCCR1B &= ((1<<CS12) | (1<<CS11) | (1<<CS10));
	}

	free(servo);
}


void rscs_servo_calibrate(rscs_servo_t * servo, uint16_t min_angle_ms, uint16_t max_angle_ms)
{
	servo->min_angle_ms = min_angle_ms;
	servo->max_angle_ms = max_angle_ms;
}


void rscs_servi_set_degrees(rscs_servo_t * servo, uint8_t angle)
{
	*servo->OCR =
			(float)(servo->max_angle_ms - servo->min_angle_ms)
			*(float)angle / 180.0f + servo->min_angle_ms;
}

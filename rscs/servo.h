#ifndef SERVOFUNCTIONS_H_
#define SERVOFUNCTIONS_H_

#include <stdint.h>

struct rscs_servo;
typedef struct rscs_servo rscs_servo_t; //Структура ещё не тип

// идентификатор пина, на который можно подключить сервомашинку
typedef enum
{
	RSCS_SERVO_ID_TIM1_A,
	RSCS_SERVO_ID_TIM1_B
} rscs_servo_id_t;

// Инициализация сервомашинки, принимающая перечесление ключей rscs_servo_id_t
// задающих возможную к запуску сервомашинку
rscs_servo_t * rscs_servo_init(rscs_servo_id_t id);

// Деинициализация и освобождение ресурсов.
void rscs_servo_deinit(rscs_servo_t * servo);

// Калибровка сервомашинки, принимающая дескриптор сервомашинки, продолжительности
// импульсов, соответствующих 0 и 180 градусам
void rscs_servo_calibrate(rscs_servo_t * servo, uint16_t min_angle_ms, uint16_t max_angle_ms);

// Поворот сервомашинки, принимающий дескриптор сервомашинки и абсолютный угол в градусах
void rscs_servi_set_degrees(rscs_servo_t * servo, uint8_t angle);




#endif /* SERVOFUNCTIONS_H_ */

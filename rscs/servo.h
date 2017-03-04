#ifndef SERVOFUNCTIONS_H_
#define SERVOFUNCTIONS_H_

#include <stdint.h>

struct rscs_servo;
typedef struct rscs_servo rscs_servo; //Структура ещё не тип

//Дескриптор сервомашинки

//Инициализация n сервомашинок
void rscs_servo_init(int n);

void rscs_servo_timer_init(void);

//Установка угла n-ой сервомашинке
void rscs_servo_set_angle(int n, int angle);

#endif /* SERVOFUNCTIONS_H_ */

#ifndef RSCS_SERVO_H_
#define RSCS_SERVO_H_

#include <stdint.h>

//Инициализация n сервомашинок
void rscs_servo_init(int n);

//Инициализация таймера, !!!ВАЖНО!!! запускать после инициализации сервомашинок
void rscs_servo_timer_init(void);

//Установка угла n-ой сервомашинке
void rscs_servo_set_angle(int n, int angle);

//Установка минимальной и максимальной длинны импульса для n-ой сервомашинки - калибровка
void rscs_servo_calibrate(int n, float min_ms, float max_ms);

//Функция для калибровки: устанавливает конкретную длину импульса для n-ой сервомашинки
// TODO: SERVO: Привести название функции к стандартам кодирования
void _servo_set_mcs(int n,int mcs);

#endif /* RSCS_SERVO_H_ */

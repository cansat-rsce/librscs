#ifndef TIME_H_
#define TIME_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __AVR_ATmega128__
#define RSCS_TIME_MAXSUBSECONDS 31250
#elif defined __AVR_ATmega328P__
#define RSCS_TIME_MAXSUBSECONDS (255 * 32)
#endif

/*!!ПРИ ИСПОЛЬЗОВАНИИ ATMEGA328P ТАЙМЕР НАСЧИТЫВАЕТ ЛИШНИЕ 16 МС ЗА СЕКУНДУ!!*/

/*Инициализация службы времени (настройка таймера)
 * Служба занимает таймер 3 */
void rscs_time_init();

/*Выключение службы времени
 * Останавливает таймер 3*/
void rscs_time_deinit();

//Получение времени в миллисекундах с момента старта аппарата
uint32_t rscs_time_get();

#endif /* TIME_H_ */

#ifndef TIME_H_
#define TIME_H_

#include <stdbool.h>
#include <stdint.h>


//Тип, описывающий время (временная метка)
typedef struct {
	uint16_t seconds; //Секунды
	uint16_t subseconds; //!!НЕ МИКРОСЕКУНДЫ!!, а подсекунды (сек/MaxSubSeconds)
} rscs_time_t ;

/*Инициализация службы времени (настройка таймера)
 * Служба занимает таймер 3 */
void rscs_time_init();

/*Выключение службы времени
 * Останавливает таймер 3*/
void rscs_time_deinit();

//Получение текущего времени
rscs_time_t rscs_time_get();

//Суммирование двух временных меток
rscs_time_t rscs_time_summ(rscs_time_t a, rscs_time_t b);

/*Сравнение двух временных меток
 * выдаст true, если a > b */
bool rscs_time_compare(rscs_time_t a, rscs_time_t b);

#endif /* TIME_H_ */

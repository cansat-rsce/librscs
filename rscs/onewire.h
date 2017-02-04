/*
	Модуль программной реализации ведущего устройства проткола OneWire.
	Модуль обеспечивает возможность работы с несколькими шинами посредством структуры rscs_ow_bus_t
	которая является дескриптором шины.

	Модуль создан опираясь на публикацию http://aterlux.ru/index.php?page=article&art=1wire
*/

#ifndef RSCS_ONEWIRE_H_
#define RSCS_ONEWIRE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// Инициализация шины - настройка железа МК для работы с OW шиной.
// пины на которыех работает шина, настраиваются в config.h
void rscs_ow_init_bus(void);

// Команда OW - RESET. Возвращает true, если был получен импульс присутсвия от ведомых на шине
bool rscs_ow_reset(void);

// Передача байта на OW шину
void rscs_ow_write(uint8_t byte);
// Чтение байта с OW шины
uint8_t rscs_ow_read(void);

// Передача n байт на OW шину
void rscs_ow_write_n(const void * buffer, size_t buffersize);
// чтение n байт с OW шины
void rscs_ow_read_n(void * buffer, size_t buffersize);


#endif /* RSCS_ONEWIRE_H_ */

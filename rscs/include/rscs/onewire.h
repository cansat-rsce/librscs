/*
	Модуль программной реализации ведущего устройства проткола OneWire.
	Модуль обеспечивает возможность работы с несколькими шинами посредством структуры rscs_ow_bus_t
	которая является дескриптором шины.

	Модуль создан опираясь на публикацию http://aterlux.ru/index.php?page=article&art=1wire

	Для работы с модулем необходимо

	1. определить дескриптор шины - объект структуры  rscs_onewire_bus_t
	и инициализировать его поля адресами регистров порта и маской пина которые подключены к OW шине.

	2. Инициализировать пин для работы с OW шиной вызовом функции rscs_ow_init_bus с определенным ранее
	дескриптором

	3. Использовать функции, предоставляемые этим компонентом для работы с конкретными ведомыми
	устройствами на шине.


	#include <avr/io.h>
	#include <rscs/onewire.h>

	rscs_ow_bus_t ow = {
			.ddr_reg = &DDRB,
			.port_reg = &PORTB,
			.pin_reg = &PINB,
			.pin_mask = 1 << 2,
	};

	int main()
	{
		rscs_ow_init_bus(&ow);

		while(1)
		{
			// начинаем OW транзакцию.
			rscs_ow_reset(&ow);

			// передаем команды ведомым
			rscs_ow_write(&ow, 0xCC);
			rscs_ow_write(&ow, 0x44);

			// получаем от ведомых ответ
			uint8_t buffer[10];
			rscs_ow_read_n(&ow, buffer, sizeof(buffer));
		}

		return 0;
	}
*/

#ifndef RSCS_ONEWIRE_H_
#define RSCS_ONEWIRE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// Структура, определяющая onewire шину.
// Используется как дескритор шины и определятся в пользовательском приложении.
typedef struct
{
	// Указатель на DDR регистр порта на котором расположен пин, работающий с шиной.
	// например - DDRB
	volatile uint8_t * ddr_reg;
	// Указатель на PORT регистр порта на котором расположен пин, работающий с шиной.
	// например - PORTB
	volatile uint8_t * port_reg;
	// Указатель на PIN регистр порта на котором расположен пин, работающий с шиной.
	// например - PINB
	volatile uint8_t * pin_reg;
	// 8-ми битная маска, задающая пин, связанный с OW шиной. Например (1 << 2)
	volatile uint8_t pin_mask;

} rscs_ow_bus_t;


// Инициализация шины - настройка железа МК для работы с OW шиной.
void rscs_ow_init_bus(rscs_ow_bus_t * bus);

// Команда OW - RESET. Возвращает true, если был получен импульс присутсвия от ведомых на шине
bool rscs_ow_reset(rscs_ow_bus_t * bus);

// Передача байта на OW шину
void rscs_ow_write(rscs_ow_bus_t * bus, uint8_t byte);
// Чтение байта с OW шины
uint8_t rscs_ow_read(rscs_ow_bus_t * bus);

// Передача n байт на OW шину
void rscs_ow_write_n(rscs_ow_bus_t * bus, const void * buffer, size_t buffersize);
// чтение n байт с OW шины
void rscs_ow_read_n(rscs_ow_bus_t * bus, void * buffer, size_t buffersize);

// TODO: Реализовать функции

#endif /* RSCS_ONEWIRE_H_ */

#ifndef RSCS_UART_H_
#define RSCS_UART_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct rscs_uart_bus;
typedef struct rscs_uart_bus rscs_uart_bus_t;


// варианты значения параметра бита четности
typedef enum
{
	RSCS_UART_PARITY_NONE,
	RSCS_UART_PARITY_EVEN,
	RSCS_UART_PARITY_ODD,
} rscs_uart_parity_t;


// флаги
typedef enum
{
	RSCS_UART_FLAG_ENABLE_TX = 0x01,	// использовать TX для этого уарта
	RSCS_UART_FLAG_ENABLE_RX = 0x02,	// использовать RX для этого уарта
} rscs_uart_mode_t;


// идентификатор UART модуля
typedef enum
{
	RSCS_UART_ID_UART0,	// UART0
#ifdef __AVR_ATmega128__
	RSCS_UART_ID_UART1	// UART1
#endif
} rscs_uart_id_t;


// количество стоповых бит
typedef enum
{
	RSCS_UART_STOP_BITS_ONE, 	// один
	RSCS_UART_STOP_BITS_TWO,	// два
} rscs_uart_stopbits_t;

// Инициализация модуля. flags - битовая композиция значений rscs_uart_mode_t
// id - одно из значений rscs_uart_id_t - выбор модуля, с которым будем работать
rscs_uart_bus_t * rscs_uart_init(rscs_uart_id_t id, int flags);
// Отключение программного модуля UART и особождение всех ресурсов, занимаемых для его обслуживания
void rscs_uart_deinit(rscs_uart_bus_t * bus);

// Установка размера символа.
/* Для Atmega 328 допустимые значения: 5 6 7 8 9. При этом 9 не поддерживается этой библиотекой */
void rscs_uart_set_character_size(rscs_uart_bus_t * bus, int character_size);
// Установка baudrate. Типовые значения: 2400, 4800, 9600, 14400, 19200, 28800, 38400 и.т.д.
void rscs_uart_set_baudrate(rscs_uart_bus_t * bus, uint32_t baudrate);
// Установка параметров бита четности
void rscs_uart_set_parity(rscs_uart_bus_t * bus, rscs_uart_parity_t parity);
// установка количества стоповых битов. Для atmega128 допустимы значения 1, 2
void rscs_uart_set_stop_bits(rscs_uart_bus_t * bus, rscs_uart_stopbits_t stopbits);

// запись на UART TX линию
void rscs_uart_write(rscs_uart_bus_t * bus, const void * dataptr, size_t dataisize);

// чтение с UART RX линии
/* следует отметить, что данная версия библиотеки не использует буферизацию и поэтому использует исключительно аппаратный
 * буффер атмеги (1 байт). Поэтому используя эту фунцию очень легко пропустить значения */
void rscs_uart_read(rscs_uart_bus_t * bus, void * dataptr, size_t datasize);


#endif /* RSCS_UART_H_ */

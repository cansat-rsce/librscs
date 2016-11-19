#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <stddef.h>

// дескриптор spi шины
typedef struct
{
	// на атмеге пустой
} rscs_spi_bus_t;

// полярность и фаза CLK сигнала
typedef enum
{
	// на первом подъеме CLK - анализ MISO, на первом падении CLK - установка MOSI
	RSCS_SPI_POL_SAMPLE_RISE_SETUP_FALL,
	// на первом подъеме CLK - установка MOSI, на первом падении CLK - анализ MISO
	RSCS_SPI_POL_SETUP_RISE_SAMPLE_FALL,
	// на первом падении CLK - анализ MISO, на первом подъеме CLK - установка MOSI
	RSCS_SPI_POL_SAMPLE_FALL_SETUP_RISE,
	// на первом падении CLK - установка MOSI, на первом подъеме CLK - анализ MISO
	RSCS_SPI_POL_SETUP_FALL_SAMPLE_RISE,
} rscs_spi_polarity_t;

// очередность передачи битов по шине SPI
typedef enum
{
	RSCS_SPI_ORDER_LSB_FIRST, // сперва передается/принимается младший бит
	RSCS_SPI_ORDER_MSB_FIRST, // сперва передается/принимается старший бит
} rscs_spi_order_t;


// инициализация шины
void rscs_spi_init(rscs_spi_bus_t * bus);

// установка частоты линии CLK (кГц)
/* К сожалению модуль SPI не может обеспечивать произвольную частоту.
   Он способен лишь задавать несколько фиксированных частот, задаваемых набором
   делителей частоты процессора МК: 2, 4, 8, 16, 32, 64, 128

   Эта функция рассчитывает делитель необходимый для достижения заданной частоты и "округляет" его
   до ближайшего большего из доступного диапазона обеспечивая максимально возможную частоту SPI не превышающую заданную.
   Если такая частота недоступна - будет вызван abort() */
void rscs_spi_set_clk(rscs_spi_bus_t * bus, uint32_t clock_kHz);
// установка полярности CLK линии
void rscs_spi_set_pol(rscs_spi_bus_t * bus, rscs_spi_polarity_t polarity);
// установка порядка передачи бит по шине
void rscs_spi_set_order(rscs_spi_bus_t * bus, rscs_spi_order_t order);


// простейший обмен одним байтом по шине
/* параметр value - байт, отравляемый на шину.
   полученный байт функция возвращает */
uint8_t rscs_spi_do(rscs_spi_bus_t * bus, uint8_t value);

// чтение последовательности байт с шины.
/* Параметры:
    - buffer - буффер для читаемых данных
    - buffer_size - размер буффера в байтах и заодно количество читаемых байт
    - dummy - значение, которое будет удерживаться на шине во время чтения  */
void rscs_spi_read(rscs_spi_bus_t * bus, void * read_buffer, size_t buffer_size, uint8_t dummy);

// Запись последовательности на шину. Все прочитанные данные с шины при этом выбрасыватся
/* Параметры:
     - buffer - буффер, в котором хранятся данные для записи
     - buffer_size - размер буффера в байтах а заодно и количество записываемых байт */
void rscs_spi_write(rscs_spi_bus_t * bus, const void * write_buffer, size_t buffer_size);

// Обмен данными по шине
/* Параметры:
    - write_buffer - буффер, из которого будут переданы данные на шину
    - read_buffer - буффер, в который будут записаны данные, полученные с шины
    - buffers_size - объем транзакции в байтах */
void rscs_spi_exchange(rscs_spi_bus_t * bus, const void * write_buffer, void * read_buffer, size_t buffers_size);






#endif /* SPI_H_ */

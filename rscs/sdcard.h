#ifndef RSCS_SDCARD_H_
#define RSCS_SDCARD_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "spi.h"
#include "error.h"

struct rscs_sdcard;
typedef struct rscs_sdcard rscs_sdcard_t;

// типы sd карт
typedef enum
{
	RSCS_SD_TYPE_SD1,
	RSCS_SD_TYPE_SD2,
	RSCS_SD_TYPE_SDHC,
} rscs_sdcard_type_t;

// Команды SD карты
typedef enum
{
	RSCS_SD_CMD0 = 0,   // сброс
	RSCS_SD_CMD1 = 1,   // инициализация
	RSCS_SD_CMD8 = 8,   // проверка напряжения SD картой
	RSCS_SD_CMD12 = 12, // остановка многоблочного чтения
	RSCS_SD_CMD17 = 17, // чтение блока
	RSCS_SD_CMD18 = 18, // чтение множества блоков
	RSCS_SD_CMD55 = 55, // преамбула для ACMD команды
	RSCS_SD_CMD41 = 41, // команда на включение SDHC карты
	RSCS_SD_CMD24 = 24, // запись блока
	RSCS_SD_CMD25 = 25, // запись множества блоков
} rscs_sd_cmd_t;

// Типы ответов SD карты
typedef enum
{
	RSCS_SD_R1,
	RSCS_SD_R3,
	RSCS_SD_R7,
} rscs_sd_resp_t;


// настройка SPI для работы с картой (полярность, частота и прочее)
/* полезно, когда перед sd картой работает какое-то другое SPI устройство, использующее другие настройки */
void rscs_sd_spi_setup(void);
// настройка SPI для работы с картой в медленном режиме.
/* Такой режим необходим при вызове операции sd_startup */
void rscs_sd_spi_setup_slow(void);

// Инициализация дескриптора SD карты (не её самой, только переферии МК)
/* по факту настраивает порт на вывод, SPI не трогает ибо для них есть отдельные функции
	Параметры:
	cs_ddr_reg - указатель на DDR регистр порта на котором расположен chip select пин SD карты (например &DDRB).
	cs_port_reg - Указатель на PORT регистр порта на котором расположен chip select пин SD карты (например - &PORTB).
	cs_pin_mask - 8-ми битная маска, задающая пин chip select SD карты (Например 1 << 2).
 */
rscs_sdcard_t * rscs_sd_init(volatile uint8_t * cs_ddr_reg,volatile uint8_t * cs_port_reg, uint8_t cs_pin_mask);

// Деинициализация SD карты и особождение всех исользуемых модулем для этой карты ресурсов.
void rscs_sd_deinit(rscs_sdcard_t * self);

// Установка таймаута на операции SD карты в микросекундах
void rscs_sd_set_timeout(rscs_sdcard_t * self, uint32_t timeout_us);

// =========================================================
// Транспортный уровень (реализуется модулем SPI)
// =========================================================
// Управление линией CS SD карты. True - активна. False - пассивна
void rscs_sd_cs(rscs_sdcard_t * self, bool state);
// передача данных SD карте по SPI
void rscs_sd_write(rscs_sdcard_t * self, const void * buffer, size_t buffer_size);
// прием данных от SD карты по SPI
void rscs_sd_read(rscs_sdcard_t * self, void * buffer, size_t buffer_size);

// =========================================================
// Уровень команд
// =========================================================
// тип ответа SD карты в зависимости от типа команды
rscs_sd_resp_t rscs_sd_response_type(rscs_sd_cmd_t cmd);
// длинна ответа в зависимости от его типа
size_t rscs_sd_response_length(rscs_sd_resp_t resp);

// Команда SD карте. Аргумент 4 байта, зависит от команды. Размер ответа зависит от типа команды
// Не управляет линией CS самостоятельно
/* По типам ответа к командам - смотри функции sd_response_type и sd_response_length */
rscs_e rscs_sd_cmd(rscs_sdcard_t  * self, rscs_sd_cmd_t cmd, uint32_t argument, void * response);
// Ожидание снятия флага занятости SD карты с возможным таймаутом.
// Не управляет линией CS самостоятельно
rscs_e rscs_sd_wait_busy(rscs_sdcard_t  * self);

// =========================================================
// Уровень операций
// =========================================================
// Запуск и нициализация самой SD карты
// управляет линией CS самостоятельно.
// Перед вызовом этой функции SPI должен быть настроен на особый медленный режим не более 400 кГц.
// смотри фунцию rscs_sd_spi_setup_slow
rscs_e rscs_sd_startup(rscs_sdcard_t  * self);
// Запись нескольких блоков по 512 байт на SD карту
// управляет линией CS самостоятельно
rscs_e rscs_sd_block_write(rscs_sdcard_t  * self, size_t offset, const void * block_start, size_t block_count);
// Чтение нескольких блоков по 512 байт на SD карту
// управляет линией CS самостоятельно
rscs_e rscs_sd_block_read(rscs_sdcard_t * self, size_t offset, void * block_start, size_t block_count);


#endif /* RSCS_SDCARD_H_ */

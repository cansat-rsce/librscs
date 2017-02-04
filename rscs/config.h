#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>


// ========================================================
// Настройки SPI
// ========================================================
// Настройки пинов зависят от микроконтроллера - пины SPI модуля нужно конфигурировать руками...
#ifdef __AVR_ATmega328P__

#define RSCS_SPI_PORTX	(PORTB) // регистр PORT порта на котором висят все пины SPI
#define RSCS_SPI_DDRX	(DDRB)	// регистр DDR порта на котором висят все пины SPI
#define RSCS_SPI_MISO	(4)		// номер MISO пина
#define RSCS_SPI_MOSI	(3)		// номер MOSI пина
#define RSCS_SPI_SCK	(5)		// номер SCK пина
#define RSCS_SPI_SS 	(2)		// номер аппаратного CHIP_SELECT пина

#elif defined __AVR_ATmega128__

#define RSCS_SPI_PORTX	(PORTB)
#define RSCS_SPI_DDRX	(DDRB)
#define RSCS_SPI_MISO	(3)
#define RSCS_SPI_MOSI	(2)
#define RSCS_SPI_SCK	(1)
#define RSCS_SPI_SS		(0)

#endif

// ========================================================
// Настройки I2C
// ========================================================
// Таймаут на I2C формируется исходя из двух параметров: длительность такта ожидания и количество этих тактов.
// Когда I2C модуль получает команду на создание какого-либо события
// его драйвер ожидает (delay) один такт ожидания, а затем проверяет -
// было ли нужное событие таки создано?. Если да - драйвер сообщает об этом пользователю.
// Если нет - модуль уходит на ожидание на еще один такт и процесс повторяется пока не будет
// исчерпано заданное количество тактов ожидания
// Длительность такта ожидания Таймаута на I2C операции (в микросекундах)
#define RSCS_I2C_TIMEOUT_US		(100)
// Количетво тактов ожидания таймаута на I2C операции
#define RSCS_I2C_TIMEOUT_CYCLES	(5)


// ========================================================
// Настройки ONE_WIRE
// ========================================================
// Регистр PORT на котором расположен пин onewire шины
#define RSCS_ONEWIRE_REG_PORT (PORTB)
// Регистр PIN на котором расположен пин onewire шины
#define RSCS_ONEWIRE_REG_PIN  (PINB)
// Регистр DDR на котором расположен пин onewire шины
#define RSCS_ONEWIRE_REG_DDR  (DDRB)
// Битовая маска, задающая тот самый пин на порту
#define RSCS_ONEWIRE_PIN_MASK (1 << 0)



// ========================================================
// Настройки ONE_WIRE
// ========================================================
// Частота SPI модуля SD карты, которую он устанавливает при вызове rscs_sd_spi_setup
#define RSCS_SDCARD_SPI_CLK_SLOW (16000)
// Частота SPI модуля SD карты, которую он устанавливает при вызове rscs_sd_spi_setup_slow
#define RSCS_SDCARD_SPI_CLK_FAST (400)



// ========================================================
// Настройки модуля SERVO
// ========================================================
// Настройки зависят от микроконтроллера - это пины, на которые выведены каналы захвата-сравнения таймера 1
#ifdef __AVR_ATmega328P__

#define RSCS_SERVO_TIM1_A_REG_DDR	(DDRB)
#define RSCS_SERVO_TIM1_A_PIN_MASK	(1 << 1)

#define RSCS_SERVO_TIM1_B_REG_DDR	(DDRB)
#define RSCS_SERVO_TIM1_B_PIN_MASK	(1 << 2)

#elif defined __AVR_ATmega128__
// TODO: Посмотреть в даташите
#endif







#endif /* CONFIG_H_ */
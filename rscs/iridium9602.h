#ifndef RSCS_IRIDIUM9602_H_
#define RSCS_IRIDIUM9602_H_

typedef struct rscs_iridium_t rscs_iridium_t;

#define RSCS_IRIDIUM9602_BUFFER_SIZE 64
#define RSCS_IRIDIUM9602_TIMEOUT_MS 30000

/* Инициализация иридиума. */
rscs_iridium_t* rscs_iridium9602_init(rscs_uart_id_t uid);

/* Проверка ответа от иридиума.
 * Посылает "AT" и ждёт "OK"
 * Ожидание неблокирующие, но ВАЖНО: данная функция будет возвращать
 * RSCS_E_BUSY пока работает. Возврат RSCS_E_NONE или другой ошибки -
 * окончание работы функции и до того момента вызов других функций данного модуля -
 * на свой страх и риск.
 */
rscs_e rscs_iridium_check(rscs_iridium_t* iridium);

/* Посылка байтов через иридиум.
 * Последовательно посылает команды AT+SBDWB и AT+SBDIX.
 * Ожидание ответов неблокирующие, но ВАЖНО: данная функция будет возвращать
 * RSCS_E_BUSY пока работает. Возврат RSCS_E_NONE или другой ошибки -
 * окончание работы функции и до того момента вызов других функций данного модуля -
 * на свой страх и риск.
 */
rscs_e rscs_iridium9602_write_bytes(rscs_iridium_t* iridium, void* data, size_t datasize);

/* Послыка текста через иридиум.
 * Последовательно посылает команды AT+SBDWT и AT+SBDIX.
 * Ожидание неблокирующие, но ВАЖНО: данная функция будет возвращать
 * RSCS_E_BUSY пока работает. Возврат RSCS_E_NONE или другой ошибки -
 * окончание работы функции и до того момента вызов других функций данного модуля -
 * на свой страх и риск.
 */
rscs_e rscs_iridium9602_write_text(rscs_iridium_t* iridium, char* str);

/* Деинициализация иридиума */
void rscs_iridium9602_deinit(rscs_iridium_t* iridium);

#endif /* RSCS_IRIDIUM9602_H_ */

/*
 * ringbuf.h
 *
 *  Created on: 12 февр. 2017 г.
 *      Author: developer
 */

#ifndef RINGBUF_H_
#define RINGBUF_H_

//Структура, описывающая кольцевой буфер
struct rscs_ringbuf;
typedef struct rscs_ringbuf rscs_ringbuf_t;

//Инициализаци буфера заданного размера
//Принимает указатель на дескриптор буфера (нужно создать самостоятельно) и его размер
rscs_ringbuf_t * rscs_ringbuf_init(size_t bufsyze);

//Освобождение памяти буфера
void rscs_ringbuf_deinit(rscs_ringbuf_t * buf);

/*Добавлени значения в голову буфера
 * Вернёт RSCS_E_BUSY если буфер полон*/
rscs_e rscs_ringbuf_push(rscs_ringbuf_t * buf, uint8_t value);

/*Взятие значения из хвоста буфера
 * Вернёт RSCS_E_BUSY если буфер пуст*/
rscs_e rscs_ringbuf_pop(rscs_ringbuf_t * buf, uint8_t * value);

//Узнать размер записанных данных
size_t rscs_ringbuf_getsize(rscs_ringbuf_t * buf);

#endif /* RINGBUF_H_ */

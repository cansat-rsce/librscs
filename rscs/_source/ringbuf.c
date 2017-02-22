#include <stdint.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "../error.h"

#include "../ringbuf.h"

struct rscs_ringbuf{
	size_t fullsize, //Полный размер буфера
	size, //Размер записанных данных
	head, //Смещение головы
	tail; //Смещение хвоста
	uint8_t buffer[]; 	//Адрес буфера в памяти
};

rscs_ringbuf_t * rscs_ringbuf_init(size_t bufsyze){
	rscs_ringbuf_t * buf = (rscs_ringbuf_t *) malloc(sizeof(rscs_ringbuf_t) + bufsyze);
	buf->fullsize = bufsyze;
	buf->head = 0;
	buf->tail = 0;
	buf->size = 0;
	return buf;
}

void rscs_ringbuf_deinit(rscs_ringbuf_t * buf){
	free(buf);
}

rscs_e rscs_ringbuf_push(rscs_ringbuf_t * buf, uint8_t value){

	rscs_e error = RSCS_E_NONE;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		//Проверяем, есть ли место
		if(buf->size == buf->fullsize) {
			error = RSCS_E_BUSY;
		} else {
			//Пишем значение в голову
			buf->buffer[buf->head] = value;
			//Двигаем голову
			buf->head++;
			if(buf->head == buf->fullsize) buf->head = 0;
			//Увеличиваем размер записанного
			buf->size++;
		}
	}

	return error;
}

rscs_e rscs_ringbuf_pop(rscs_ringbuf_t * buf, uint8_t * value){
	rscs_e error = RSCS_E_NONE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		//Проверяем, есть ли данные в буфере
		if(buf->size == 0) error =  RSCS_E_BUSY;
		else {
			//Читаем значение из кольцевого буфера
			*value = buf->buffer[buf->tail];
			//Двигаем хвост
			buf->tail++;
			if(buf->tail == buf->fullsize) buf->tail = 0;
			//Уменьшаем размер записанного
			buf->size--;
		}
	}

	return error;
}

size_t rscs_ringbuf_getsize(rscs_ringbuf_t * buf){
	size_t size;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		size = buf->size;
	}
	return size;
}

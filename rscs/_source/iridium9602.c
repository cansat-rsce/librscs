#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stdext/stdio.h"

#include "error.h"
#include "timeservice.h"

#include "uart.h"
#include "iridium9602.h"

typedef enum{
	NOP,
	AT,
	SBDIX,
	SBDWBS,
	SBDWBD,
	SBDWT
} command;

struct rscs_iridium_t{
	rscs_uart_bus_t* uart;
	char buffer[RSCS_IRIDIUM9602_BUFFER_SIZE];
	size_t carret;
	command last;
	uint32_t time;
};

static void _accumulate(rscs_iridium_t* iridium){
	uint8_t readed;
	while(0 != (readed = rscs_uart_read_some(iridium->uart, iridium->buffer + iridium->carret, sizeof(iridium->buffer) - iridium->carret - 1))){
		iridium->carret += readed;
	}
	iridium->buffer[iridium->carret] = 0;
}

static rscs_e _await(rscs_iridium_t* iridium, const char* trigger, const char* delim, char* previos){
	if(previos == NULL){
		_accumulate(iridium);

		char str[iridium->carret + 1];
		strcpy(str, iridium->buffer);

		for(char* lex = strtok(str, delim); lex != NULL; lex = strtok(NULL, delim)){
			if(strcmp(lex, trigger) == 0) {
				iridium->carret = 0;

				return RSCS_E_NONE;
			}
			if(strcmp(lex, "ERROR") == 0){
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_INVRESP;
			}
		}
	}
	else{

	}
	return RSCS_E_BUSY;
}

#define RETIFE(OP) rscs_e error = OP; if(error) return error

rscs_e rscs_iridium_check(rscs_iridium_t* iridium){
	switch(iridium->last){
	case SBDIX: case SBDWBD: case SBDWBS: return RSCS_E_INVARG;
	case NOP:
	{
		char buf[] = "AT\r\n";
		rscs_uart_write(iridium->uart, buf, strlen(buf));
		iridium->last = AT;
	}
		break;
	case AT:
	{
		RETIFE(_await(iridium, "OK", "\r\n", NULL));
		iridium->last = NOP;

		return RSCS_E_NONE;
	}
		break;
	}
	return RSCS_E_BUSY;
}


rscs_e rscs_iridium9602_write_bytes(rscs_iridium_t* iridium, void* data, size_t datasize){
	switch(iridium->last){
	case AT: return RSCS_E_INVARG;
	case NOP:
	{
		iridium->last = SBDWBS;

		char buf[20];
		sprintf(buf, "AT+SBDWB=%d\r\n", datasize);
		rscs_uart_write(iridium->uart, buf, strlen(buf));

		iridium->time = rscs_time_get();
	}
		//break;
	case SBDWBS:
	{
		RETIFE(_await(iridium, "READY", "\r\n", NULL));
		iridium->last = SBDWBD;

		rscs_uart_write(iridium->uart, data, datasize);
		char buf[] = "\r\n";
		rscs_uart_write(iridium->uart, buf, strlen(buf));

		iridium->time = rscs_time_get();
	}
		break;
	case SBDWBD:
	{
		RETIFE(_await(iridium, "OK", "\r\n", NULL));
		iridium->last = SBDIX;

		char buf[] = "AT+SBDIX\r\n";
		rscs_uart_write(iridium->uart, buf, strlen(buf));

		iridium->time = rscs_time_get();
	}
		//break;
	case SBDIX:
	{
		RETIFE(_await(iridium, "OK", "\r\n", NULL));
		iridium->last = NOP;

		return RSCS_E_NONE;
	}
		break;
	}

	if(rscs_time_get() - iridium->time > RSCS_IRIDIUM9602_TIMEOUT_MS){
		iridium->carret = 0;
		iridium->last = NOP;

		return RSCS_E_TIMEOUT;
	}

	return RSCS_E_BUSY;
}

rscs_e rscs_iridium9602_write_text(rscs_iridium_t* iridium, char* str){
	switch(iridium->last){
	case AT: case SBDWBS: case SBDWBD: return RSCS_E_INVARG;
	case NOP:
	{
		iridium->last = SBDWT;

		char buf[strlen(str) + 14];
		sprintf(buf, "AT+SBDWT=%s\r\n", str);
		rscs_uart_write(iridium->uart, buf, strlen(buf));

		iridium->time = rscs_time_get();
	}
		//break;
	case SBDWT:
	{
		RETIFE(_await(iridium, "OK", "\r\n", NULL));
		iridium->last = SBDIX;

		char buf[] = "AT+SBDIX\r\n";
		rscs_uart_write(iridium->uart, buf, strlen(buf));

		iridium->time = rscs_time_get();
	}
		//break;
	case SBDIX:
	{
		RETIFE(_await(iridium, "OK", "\r\n", NULL));
		iridium->last = NOP;

		return RSCS_E_NONE;
	}
		break;
	}

	if(rscs_time_get() - iridium->time > RSCS_IRIDIUM9602_TIMEOUT_MS){
		iridium->carret = 0;
		iridium->last = NOP;

		return RSCS_E_TIMEOUT;
	}

	return RSCS_E_BUSY;
}

rscs_iridium_t* rscs_iridium9602_init(rscs_uart_id_t uid){

	rscs_uart_bus_t *uart = rscs_uart_init(uid,
											RSCS_UART_FLAG_ENABLE_TX | RSCS_UART_FLAG_BUFFER_TX |
											RSCS_UART_FLAG_ENABLE_RX | RSCS_UART_FLAG_BUFFER_RX);
	if(uart == NULL) return NULL;

	rscs_iridium_t* retval = (rscs_iridium_t*)malloc(sizeof(rscs_iridium_t));

	if(retval == NULL){
		rscs_uart_deinit(uart);
		return NULL;
	}

	rscs_uart_set_character_size(uart, 8);
	rscs_uart_set_baudrate(uart, 19200);
	rscs_uart_set_parity(uart, RSCS_UART_PARITY_NONE);
	rscs_uart_set_stop_bits(uart, RSCS_UART_STOP_BITS_ONE);

	retval->uart = uart;
	retval->carret = 0;
	retval->last = NOP;

	rscs_time_init();

	return retval;
}

void rscs_iridium9602_deinit(rscs_iridium_t* iridium){
	rscs_uart_deinit(iridium->uart);

	free(iridium);
}





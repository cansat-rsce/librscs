#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stdext/stdio.h"

#include "error.h"
#include "uart.h"
#include "iridium9602.h"

typedef enum{
	NOP,
	AT,
	SBDIX,
	SBDWBS,
	SBDWBD
} command;

struct rscs_iridium_t{
	rscs_uart_bus_t* uart;
	char buffer[RSCS_IRIDIUM9602_BUFFER_SIZE];
	size_t carret;
	command last;
};

static void accumulate(rscs_iridium_t* iridium){
	uint8_t readed;
	while(0 != (readed = rscs_uart_read_some(iridium->uart, iridium->buffer + iridium->carret, sizeof(iridium->buffer) - iridium->carret - 1))){
		iridium->carret += readed;
	}
	iridium->buffer[iridium->carret] = 0;
}

rscs_e rscs_iridium9602_write(rscs_iridium_t* iridium, void* data, size_t datasize){
	switch(iridium->last){
	case NOP:
	{
		char buf[5];
		sprintf(buf, "AT\r\n");
		rscs_uart_write(iridium->uart, buf, strlen(buf));
		iridium->last = AT;
	}
		break;
	case AT:
	{
		accumulate(iridium);

		char str[iridium->carret + 1];
		strcpy(str, iridium->buffer);

		for(char* lex = strtok(str, "\r\n"); lex != NULL; lex = strtok(NULL, "\r\n")){
			if(strcmp(lex, "OK") == 0) {
				char buf[18];
				sprintf(buf, "AT+SBDWB=%d\r\n", datasize);
				rscs_uart_write(iridium->uart, buf, strlen(buf));

				iridium->carret = 0;
				iridium->last = SBDWBS;
			}
			if(strcmp(lex, "ERROR") == 0){
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_INVARG;
			}
		}
	}
		break;
	case SBDWBS:
	{
		accumulate(iridium);

		char str[iridium->carret + 1];
		strcpy(str, iridium->buffer);

		for(char* lex = strtok(str, "\r\n"); lex != NULL; lex = strtok(NULL, "\r\n")){
			if(strcmp(lex, "READY") == 0) {
				rscs_uart_write(iridium->uart, data, datasize);
				rscs_uart_write(iridium->uart, "\r\n", 2);

				iridium->carret = 0;
				iridium->last = SBDWBD;
			}
			if(strcmp(lex, "ERROR") == 0){
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_INVARG;
			}
		}
	}
		break;
	case SBDWBD:
	{
		accumulate(iridium);

		char str[iridium->carret + 1];
		strcpy(str, iridium->buffer);

		for(char* lex = strtok(str, "\r\n"); lex != NULL; lex = strtok(NULL, "\r\n")){
			if(strcmp(lex, "OK") == 0) {
				char buf[] = "AT+SBDIX\r\n";
				rscs_uart_write(iridium->uart, buf, strlen(buf));

				iridium->carret = 0;
				iridium->last = SBDIX;
			}
			if(strcmp(lex, "ERROR") == 0){
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_INVARG;
			}
		}
	}
		break;
	case SBDIX:
		accumulate(iridium);

		char str[iridium->carret + 1];
		strcpy(str, iridium->buffer);

		for(char* lex = strtok(str, "\r\n"); lex != NULL; lex = strtok(NULL, "\r\n")){
			if(strcmp(lex, "OK") == 0) {
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_NONE;
			}
			if(strcmp(lex, "ERROR") == 0){
				iridium->carret = 0;
				iridium->last = NOP;

				return RSCS_E_INVARG;
			}
		}
		break;
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
	//stdout = stdin = rscs_make_uart_stream(retval->uart);

	return retval;
}





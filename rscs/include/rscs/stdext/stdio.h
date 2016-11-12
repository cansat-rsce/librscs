#ifndef STDIO_H_
#define STDIO_H_

#include "../uart.h"

#include <stdio.h>

// Создает стандартный поток на UART
FILE * rscs_make_uart_stream(rscs_uart_bus_t * bus);


#endif /* STDIO_H_ */

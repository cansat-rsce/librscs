#include "../../stdext/stdio.h"

static int rscs_uart_stream_read(FILE * stream)
{
	void * userdata = fdev_get_udata(stream);
	rscs_uart_bus_t * bus = (rscs_uart_bus_t *)userdata;

	char symbol;
	rscs_uart_read(bus, &symbol, 1);
	return symbol;
}


static int rscs_uart_stream_write(char symbol, FILE * stream)
{
	void * userdata = fdev_get_udata(stream);
	rscs_uart_bus_t * bus = (rscs_uart_bus_t *)userdata;

	rscs_uart_write(bus, &symbol, 1);
	return 0;
}


FILE * rscs_make_uart_stream(rscs_uart_bus_t * bus)
{
	FILE * retval = fdevopen(rscs_uart_stream_write, rscs_uart_stream_read);;
	fdev_set_udata(retval, bus);

	return retval;
}


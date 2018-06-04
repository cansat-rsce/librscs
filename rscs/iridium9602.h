typedef struct rscs_iridium_t rscs_iridium_t;

#define RSCS_IRIDIUM9602_BUFFER_SIZE 64

rscs_e rscs_iridium9602_write(rscs_iridium_t* iridium, void* data, size_t datasize);

rscs_iridium_t* rscs_iridium9602_init(rscs_uart_id_t uid);

#ifndef RSCS_CRC_H_
#define RSCS_CRC_H_

#include <stddef.h>
#include <stdint.h>

// Подсчет контрольной суммы алгоритмом CRC-8
uint8_t rscs_crc8(const void * data_ptr, size_t data_size);

// Подсчет контрольной суммы алгоритмом CRC-7
// Обнаружилась целая куча алгоритмов, называемых CRC-7. Конркретно этот используется на SD картах
uint8_t rscs_crc7(const void * data_ptr, size_t data_size);

#endif /* RSCS_CRC_H_ */

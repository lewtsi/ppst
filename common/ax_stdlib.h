#ifndef AX_STDLIB_H
#define AX_STDLIB_H

#include "ax_common.h"

void ax_buffer_copy(char *src, char *dest, uint8_t len);
void ax_data_to_ascii(uint8_t *data, char *buf, uint8_t datalen);

#endif


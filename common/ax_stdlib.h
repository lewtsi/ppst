#ifndef AX_STDLIB_H
#define AX_STDLIB_H

#include "ax_common.h"

void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
uint8_t ax_stdlib_generate_xor(uint8_t *buf, uint8_t cnt);
void ax_stdlib_buffer_copy(char *src, char *dest, uint8_t len);
opt_result_t ax_stdlib_buffer_compare(char *bufA, char *bufB, uint8_t len);
void ax_stdlib_data_to_ascii(uint8_t *data, char *buf, uint8_t datalen);

opt_result_t ax_stdlib_decimal_string_to_ushort(char *str, uint16_t *value);
opt_result_t ax_stdlib_decimal_string_to_ushort_II(char *str, 
											uint16_t *value, uint8_t *len);
opt_result_t ax_stdlib_decimal_string_to_ushort_III(char * str, 
											uint8_t len, uint16_t *value);
uint32_t fx_stdlib_mktime (uint16_t year, uint8_t mon, uint8_t day, 
								uint8_t hour, uint8_t min, uint8_t sec);
opt_result_t ax_stdlib_time_string_to_utc(char *str, uint32_t *utc);
opt_result_t ax_stdlib_data_is_power_of_two(void data);

#endif


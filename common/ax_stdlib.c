


#include "ax_stdlib.h"

static uint8_t asciidata[16] = "0123456789ABCDEF";


void ax_buffer_copy(char *src, char *dest, uint8_t len)
{
	while((len --) != 0){
		*dest = *src;
		dest ++;
		src ++;
	}
}

/// retval 0x00 OPT_SUCCESS equal
/// retval 0xFF OPT_FAILURE	not_equal
opt_result_t ax_buffer_compare(char *bufA, char *bufB, uint8_t len)
{
	while(len != 0){
		if(*bufA != *bufB){
			return OPT_FAILURE;
		}
		bufA ++;
		bufB ++;
		len --;
	}
	return OPT_SUCCESS;
}

void ax_data_to_ascii(uint8_t *data, char *buf, uint8_t datalen)
{
	buf += (datalen << 1) - 1;	// 反序操作，正序输出
	
	while(datalen != 0){
		*buf -- = asciidata[*data & 0x0F];
		*buf -- = asciidata[(*data >> 4) & 0x0F];
		data ++;
		datalen --;
	}
}




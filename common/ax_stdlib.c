


#include "ax_stdlib.h"

static uint8_t asciidata[16] = {'0', '1', '2', '3',
								'4', '5', '6', '7',
								'8', '9', 'A', 'B',
								'C', 'D', 'E', 'F' };


void ax_buffer_copy(char *src, char *dest, uint8_t len)
{
	while((len --) != 0){
		*dest = *src;
		dest ++;
		src ++;
	}
}

void ax_data_to_ascii(uint8_t *data, char *buf, uint8_t datalen)
{
	uint8_t tmp;
	buf += datalen << 1;	// ·´Ðò²Ù×÷
	buf --;
	while(datalen != 0){
		tmp = *data & 0x0F;
		*buf -- = asciidata[tmp];
		tmp = (*data >> 4) & 0x0F;
		*buf -- = asciidata[tmp];
		data ++;
		datalen --;
	}
}





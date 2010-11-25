


#include "ax_stdlib.h"

void ax_buffer_copy(char *src, char *dest, uint8_t len)
{
	while((len --) != 0){
		*dest = *src;
		dest ++;
		src ++;
	}
}





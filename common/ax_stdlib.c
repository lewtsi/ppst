


#include "ax_stdlib.h"
#include "ctype.h"

static uint8_t asciidata[] = "0123456789ABCDEF";

void delay_us(uint16_t us)
{
	uint8_t i;
	while(us-- > 0){
		i = 10;
		while(i--);
	}
}

void delay_ms(uint16_t ms)
{
	while(ms-- > 0){
		delay_us(1000);
	}
}

uint8_t ax_stdlib_generate_xor(uint8_t *buf, uint8_t cnt)
{
	uint8_t xor_tmp = 0;
	while(cnt -- > 0){
		xor_tmp ^= *buf ++;
	}
	return xor_tmp;
}

void ax_stdlib_buffer_copy(char *src, char *dest, uint8_t len)
{
	while((len --) != 0){
		*dest = *src;
		dest ++;
		src ++;
	}
}

/// retval 0x00 OPT_SUCCESS equal
/// retval 0xFF OPT_FAILURE	not_equal
opt_result_t ax_stdlib_buffer_compare(char *bufA, char *bufB, uint8_t len)
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

// 0x3ABC8765 datalen=4 -> (char *)"3ABC8765" 
void ax_stdlib_data_to_ascii(uint8_t *data, char *buf, uint8_t datalen)
{
	buf += (datalen << 1) - 1;	// 反序操作，正序输出
	
	while(datalen != 0){
		*buf -- = asciidata[*data & 0x0F];
		*buf -- = asciidata[(*data >> 4) & 0x0F];
		data ++;
		datalen --;
	}
}

// 0x81BB = 33211 => (char *)"33211", *len = 5
void ax_stdlib_ushort_to_decimal_string(uint16_t usrt, char *buf, uint8_t *len)
{
	uint8_t bg = 0, cnt = 0;
	
	*buf = (usrt / 10000) + '0';
	if(*buf != '0'){
		cnt ++;	buf ++; bg = 1;
	}
	usrt %= 10000;
	*buf = (usrt / 1000) + '0';
	if(*buf != '0' || bg){
		cnt ++;	buf ++;	bg = 1;
	}
	usrt %= 1000;
	*buf = (usrt / 100) + '0';
	if(*buf != '0' || bg){
		cnt ++;	buf ++;	bg = 1;
	}
	usrt %= 100;
	*buf = (usrt / 10) + '0';
	if(*buf != '0' || bg){
		cnt ++;	buf ++;	bg = 1;
	}
	usrt %= 10;
	*buf = usrt + '0';
	cnt ++;
	*len = cnt;
}

/// decimal number string to unsigned short ("33211" -> 0x81BB)
opt_result_t ax_stdlib_decimal_string_to_ushort(char *str, uint16_t *value)
{
	uint32_t value_tmp = 0;
	uint8_t len = 0;
	
	while((*str >= '0') && (*str <= '9')){
		*str -= '0';
		value_tmp *= 10;
		value_tmp += *str;
		str ++;
		len ++;
		if(len > 5) return OPT_ERR_CODE_A;
	}
	if(value_tmp > 0xFFFF) return OPT_ERR_CODE_B;
	
	*value = (uint16_t)value_tmp;
	return OPT_SUCCESS;
}

/// decimal number string to unsigned short ("33211" -> 0x81BB, *len = 5)
opt_result_t ax_stdlib_decimal_string_to_ushort_II(char *str, 
											uint16_t *value, uint8_t *len)
{
	uint32_t value_tmp = 0;
	uint8_t len_tmp = 0;
	
	while((*str >= '0') && (*str <= '9')){
		*str -= '0';
		value_tmp *= 10;
		value_tmp += *str;
		str ++;
		len_tmp ++;
		if(len_tmp > 5) return OPT_ERR_CODE_A;
	}
	if(value_tmp > 0xFFFF) return OPT_ERR_CODE_B;

	*value = (uint16_t)value_tmp;
	*len = len_tmp;
	return OPT_SUCCESS;
}

/// decimal number string to unsigned short ("33211" len=3 -> 0x014C)
opt_result_t ax_stdlib_decimal_string_to_ushort_III(char * str, 
											uint8_t len, uint16_t *value)
{
	uint32_t value_tmp = 0;
	uint8_t len_tmp = 0;
	if(len > 5) return OPT_FAILURE;
	
	while((*str >= '0') && (*str <= '9')){
		*str -= '0';
		value_tmp *= 10;
		value_tmp += *str;
		str ++;
		len_tmp ++;
		if(len_tmp >= len) break;
	}
	if(len_tmp < len) return OPT_ERR_CODE_A;
	if(value_tmp > 0xFFFF) return OPT_ERR_CODE_B;
	
	*value = (uint16_t)value_tmp;
	return OPT_SUCCESS;
}

/// "mktime" function from linux source code [modify]
uint32_t fx_stdlib_mktime (uint16_t year, uint8_t mon, uint8_t day, 
								uint8_t hour, uint8_t min, uint8_t sec)
{
	if (0 >= (char) (mon -= 2)){		// 1..12 -> 11,12,1..10
		mon += 12;		// Puts Feb last since it has leap day
		year -= 1;
	}
	return (((
		(uint32_t) (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		year*365 - 719499
	)*24 + hour		// now have hours
	)*60 + min		// now have minutes
	)*60 + sec;		// finally seconds
}

/// time string to 32 bits format utc time ("20110210-123013" -> 0x04000000)
opt_result_t ax_stdlib_time_string_to_utc(char *str, uint32_t *utc)
{
	uint16_t year = 0, value_tmp;
	uint8_t mon, day, hour, min, sec;
	opt_result_t opt_result = OPT_SUCCESS;

	opt_result = ax_stdlib_decimal_string_to_ushort_III(str, 4, &year);
	opt_result |= ax_stdlib_decimal_string_to_ushort_III((str += 4), 2, &value_tmp);
	mon = (uint8_t)value_tmp;
	opt_result |= ax_stdlib_decimal_string_to_ushort_III((str += 2), 2, &value_tmp);
	day = (uint8_t)value_tmp;
	opt_result |= ax_stdlib_decimal_string_to_ushort_III((str += 3), 2, &value_tmp);
	hour = (uint8_t)value_tmp;
	opt_result |= ax_stdlib_decimal_string_to_ushort_III((str += 2), 2, &value_tmp);
	min = (uint8_t)value_tmp;
	opt_result |= ax_stdlib_decimal_string_to_ushort_III((str += 2), 2, &value_tmp);
	sec = (uint8_t)value_tmp;

	if(opt_result == OPT_SUCCESS){
		*utc = fx_stdlib_mktime(year, mon, day, hour, min, sec);
	}
	
	return opt_result;
}

/// 查看数据是否是2的N次幂
opt_result_t ax_stdlib_data_is_power_of_two(void data)
{
	if((data & (data - 1)) == 0) return OPT_RESULT_TRUE;
	else OPT_RESULT_FALSE;
}




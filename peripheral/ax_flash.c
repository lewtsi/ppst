/**
 * Use inner flash to save user parameters

 * 内部flash从92k到128k为大容量数据存储区，每2*2k空间为一配置区域zone
 * 每条配置为30字节+1字节使用标志(低)+1字节校验字(高)，2048/32=64条
 * 总共36k空间可存储9条不同数据项，本程序可经简单修改兼容16条数据项。

 * 当写入一条配置信息时，第一字节写入长度，之后是校验值，最后是数据
 * 当查找位置时，第一字节不为FF则为已用，查找时使用分段查找
 * 64条分8段，每次查找段首不为ff则查找下一段
 **/
#include "ax_flash.h"


#define ST_FLASH_CFG_BASE_ADDR	((uint32_t)0x08017000)	// 92K

#define ST_FLASH_CFG_LEN_MAX	((uint8_t)30)	// 最大的配置字长度
#define ST_FLASH_SEG_LEN		((uint8_t)32)	// 每条配置占用空间长度
#define ST_FLASH_SEG_CNT_MAX	((uint8_t)64)	// 2k/32=64
#define ST_FLASH_CFG_CNT_MAX	((uint8_t)9)	// 最多可有多少条配置信息
#define ST_FLASH_CFG_CNT_USED	((uint8_t)3)	// 现在使用的配置条数

#define ST_FLASH_USER_DATA_ADD	((uint8_t)0x30)	// 用户读取长于实际的补充数据

static uint16_t st_flash_zone_flag;	// zone区段选择，16条兼容
static uint8_t st_flash_seg_cnt[ST_FLASH_CFG_CNT_USED];	// 各条配置的segment cnt

static uint16_t st_flash_read_buf[ST_FLASH_SEG_LEN >> 1];

opt_result_t ax_flash_config_info_init(void)
{
	uint8_t i_zone = 0;
	uint8_t seek1;
	uint32_t addr, addr_ass;

	addr_ass = ST_FLASH_CFG_BASE_ADDR;
	//seek1_intv = ST_FLASH_SEG_LEN << 3;
	while(i_zone < ST_FLASH_CFG_CNT_USED){	// 必须从0开始
		addr = addr_ass;
		if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
			st_flash_zone_flag |= (1<<i_zone);
			addr += ST_FLASH_PAGE_SIZE;
			if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
				// 没有数据，初始化写入
				addr -= ST_FLASH_PAGE_SIZE;
				//st_flash_zone_flag &= ~(1<<i_zone);
				st_flash_seg_cnt[i_zone] = ST_FLASH_SEG_CNT_MAX - 1;
				ax_flash_config_info_write((st_flash_type_t)i_zone, 
												"abcdefghi", 10);
				continue;
			}
		}
		for(seek1=1; seek1<ST_FLASH_SEG_CNT_MAX; seek1++){
			addr += ST_FLASH_SEG_LEN;
			if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
				break;
			}
		}
		st_flash_seg_cnt[i_zone] = seek1 - 1;
		i_zone ++;
		addr_ass += (ST_FLASH_PAGE_SIZE << 1);
	}
	return OPT_SUCCESS;
}

opt_result_t ax_flash_config_info_write(st_flash_type_t type, 
										char *buf, uint8_t len)
{
	uint32_t addr;
	uint16_t *hwbuf = (uint16_t *)buf;
	uint8_t hwlen, i;
	uint16_t data;

	if(len > ST_FLASH_CFG_LEN_MAX) return OPT_FAILUR;
	if(type > ST_FLASH_CFG_CNT_USED) return OPT_ERR_CODE_SP;

	hwlen = len;	// hwlen <= xor (len ^ data ... )
	for(i=0; i<len; i++, buf++){
		hwlen ^= *buf;
	}
	data = ((uint16_t)hwlen << 8) | len;
	st_flash_seg_cnt[type] ++;
	if(st_flash_seg_cnt[type] == ST_FLASH_SEG_CNT_MAX){
		addr = ST_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
		if(st_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
		FLASH_ErasePage(addr);
		st_flash_zone_flag ^= (1<<type);
		st_flash_seg_cnt[type] = 0;
	}
	addr = ST_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
	if(st_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
	addr += st_flash_seg_cnt[type] * ST_FLASH_SEG_LEN;

	if(FLASH_ProgramHalfWord(addr, data)!= FLASH_COMPLETE)
		return OPT_ERR_CODE_A;
	if((*(__IO uint16_t*)addr) != data)
		return OPT_ERR_CODE_B;
	addr += 2;
	hwlen = len >> 1;
	while(hwlen != 0){
		if(FLASH_ProgramHalfWord(addr, *hwbuf)!= FLASH_COMPLETE)
			return OPT_ERR_CODE_C;
		if((*(__IO uint16_t*)addr) != *hwbuf){
			if(FLASH_ProgramHalfWord(addr, *hwbuf)!= FLASH_COMPLETE)
				return OPT_ERR_CODE_D;
			if((*(__IO uint16_t*)addr) != *hwbuf){
				return OPT_ERR_CODE_E;
			}
		}
		hwbuf ++;
		addr += 2;
		hwlen --;
	}
	if(len & 0x01){
		data = (*hwbuf) & 0x00FF;
		if(FLASH_ProgramHalfWord(addr, data)!= FLASH_COMPLETE)
			return OPT_ERR_CODE_F;
		if((*(__IO uint16_t*)addr) != data)
			return OPT_ERR_CODE_G;
	}
	return OPT_SUCCESS;
}

// flash read : read all of the config info one time, 
// even thought user maybe do not want all the things.
// 如果用户读取的长度大于实际有的数据长度，则多出部分以零补充，并返回出错
opt_result_t ax_flash_config_info_read(st_flash_type_t type, 
											char *buf, uint8_t len)
{
	static uint32_t addr;
	uint16_t *hwbuf = st_flash_read_buf;
	uint8_t *rd_buf = (uint8_t *)st_flash_read_buf;
	uint8_t ixor = 0, hwlen, truelen;
	static uint16_t ass_data;	// len + xor
	
	addr = ST_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
	if(st_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
	addr += st_flash_seg_cnt[type] * ST_FLASH_SEG_LEN;

	ass_data = *(__IO uint16_t*)addr;
	truelen = (uint8_t)ass_data;
	if(truelen > ST_FLASH_CFG_LEN_MAX) return OPT_ERR_CODE_A;
	hwlen = (truelen >> 1);
	if(truelen & 0x01) hwlen ++;

	*hwbuf = ass_data;
	while(hwlen != 0){
		addr += 2;
		hwbuf ++;
		*hwbuf = *(__IO uint16_t*)addr;
		hwlen --;
	}
	hwlen = truelen+2;
	while((hwlen --) != 0){
		ixor ^= *rd_buf;
		rd_buf ++;
	}
	if(ixor != 0) return OPT_ERR_CODE_B;
	ax_buffer_copy((char *)&st_flash_read_buf[1], buf, len);
	if(len > truelen){
		rd_buf = (uint8_t *)&buf[truelen];
		for(; truelen < len; truelen ++, rd_buf ++){
			*rd_buf = ST_FLASH_USER_DATA_ADD;
		}
		//return OPT_RESULT_A;
	}
	return OPT_SUCCESS;
}



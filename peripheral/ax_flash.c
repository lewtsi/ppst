/**
 * Use inner flash to save user parameters

 * 内部flash从92k到128k为大容量数据存储区，每2*2k空间为一配置区域zone
 * 每条配置为30字节+1字节使用标志(低)+1字节校验字(高)，2048/32=64条
 * 总共36k空间可存储9条不同数据项，本程序可经简单修改兼容16条数据项。

 * 当写入一条配置信息时，第一字节写入长度，之后是校验值，最后是数据
 * 当查找位置时，第一字节不为FF则为已用，查找时使用分段查找
 * 64条分8段，每次查找段首不为ff则查找下一段
 *
 * 由于用一字节表示长度且不为FF，因此单条配置信息的最大的有效长度为254，
 * 也就是说单条信息的最大空间占用为256字节。
 *
 * 从Ver.0.01a开始将各条配置的长度改为可编译前调整的方式，
 * 而且为了保证其有更高的适用范围，信息长度改为两字节表示，
 * 最大长度为(2048-4)。



 * | 有效数据长度[2B] | 校验值[2B] | 有效数据 ... |
 **/
#include "ax_flash.h"
#include "stm32_com.h"


#define FX_FLASH_CFG_BASE_ADDR	((uint32_t)0x08017000)	// 92K

#define FX_FLASH_CFG_CNT_MAX	((uint8_t)9)	// 最多可有多少条配置信息

#define FX_SEG_HEAD_LEN			((uint8_t)4)	// 配置头数据长度
#define FX_VERIFY_POS			((uint8_t)2)	// 校验值的位置
#define FX_FLASH_USER_DATA_ADD	((uint8_t)0x30)	// 用户读取长于实际的补充数据

// ********************** modify segments (OP) ********************** //

#define FX_FLASH_CFG_CNT_USED	((uint8_t)3)	// 现在使用的配置条数
#define FX_FLASH_SEG_LEN_MAX	((uint16_t)256)	// 各条配置占用的最大空间长度
static uint16_t fx_flash_seg_len[FX_FLASH_CFG_CNT_USED] = { 32, 64, 256 };
			//<! 配置信息占用的空间长度 ( MUST BE 2^n !!!)
static uint8_t fx_flash_seg_cnt_max[FX_FLASH_CFG_CNT_USED] = { 64, 32, 8 };
			//<! 2K/fx_flash_seg_len (手动赋值)

// ********************** modify segments (ED) ********************** //

static uint16_t fx_flash_info_len_max[FX_FLASH_CFG_CNT_USED];
			//<! 最大的有效数据长度(seg_len - 4)

static uint16_t fx_flash_zone_flag;	// zone区段选择，16条兼容
static uint8_t fx_flash_seg_cnt[FX_FLASH_CFG_CNT_USED];	// 各条配置的segment cnt

static uint16_t fx_flash_read_buf[FX_FLASH_SEG_LEN_MAX >> 1];

static uint8_t fx_flash_init_flag = 0;

/// Flash config info init: write init value or get postion now
/// \param result bit0-zone init success   bit1-zone init failure
/// \retval OPT_SUCCESS if write init value ok, or got postion now
///			OPT_FAILURE some wrong in init.
/// \note Power on running ONLY once.
opt_result_t ax_flash_config_info_init(uint16_t *result)
{
	uint8_t i_zone = 0, seek1;
	uint32_t addr, ass_addr;

	if(fx_flash_init_flag) return OPT_SUCCESS;	// 已经初始化
	
	*result = 0;
	ass_addr = FX_FLASH_CFG_BASE_ADDR;
	while(i_zone < FX_FLASH_CFG_CNT_USED){	// 必须从0开始
		if(fx_flash_seg_len[i_zone] <= FX_SEG_HEAD_LEN){
			debug_info_output("ax_flash.c flash segment len "
								"is too short.\n");
			return OPT_FAILUR;
		}
		fx_flash_info_len_max[i_zone] = 
				fx_flash_seg_len[i_zone] - FX_SEG_HEAD_LEN;
		
		addr = ass_addr;
		if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
			fx_flash_zone_flag |= (1<<i_zone);
			addr += ST_FLASH_PAGE_SIZE;
			if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
				// 没有数据，置位对应的result位
				//addr -= ST_FLASH_PAGE_SIZE;
				fx_flash_seg_cnt[i_zone] = fx_flash_seg_cnt_max[i_zone] - 1;
				*result |= (1 << i_zone);
				i_zone ++;
				continue;
			}
		}
		for(seek1=1; seek1<fx_flash_seg_cnt_max[i_zone]; seek1++){
			addr += fx_flash_seg_len[i_zone];
			if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
				break;
			}
		}
		fx_flash_seg_cnt[i_zone] = seek1 - 1;
		i_zone ++;
		ass_addr += (ST_FLASH_PAGE_SIZE << 1);
	}
	
	fx_flash_init_flag = 1;
	
	return OPT_SUCCESS;
}

opt_result_t fx_flash_write_half_word(uint32_t addr, uint16_t data)
{
	if(FLASH_ProgramHalfWord(addr, data)!= FLASH_COMPLETE)
		return OPT_ERR_CODE_A;
	if((*(__IO uint16_t*)addr) != data){
		if(FLASH_ProgramHalfWord(addr, data)!= FLASH_COMPLETE)
			return OPT_ERR_CODE_B;
		if((*(__IO uint16_t*)addr) != data){
			return OPT_ERR_CODE_C;
		}
	}
	return OPT_SUCCESS;
}

opt_result_t ax_flash_config_info_write(st_flash_type_t type, 
										char *buf, uint16_t len)
{
	uint32_t addr;
	uint16_t *hwbuf = (uint16_t *)buf;
	uint16_t hwlen, i, data;
	uint8_t xor_tmp = 0;
	opt_result_t wt_result;

	if(type > FX_FLASH_CFG_CNT_USED) return OPT_ERR_CODE_SP;
	if(len > fx_flash_info_len_max[type]) return OPT_FAILUR;

	// xor_tmp <- xor (len ^ data ... )
	xor_tmp = (uint8_t)len;
	xor_tmp ^= (uint8_t)(len >> 8);
	for(i=0; i<len; i++, buf++){
		xor_tmp ^= *buf;
	}

	fx_flash_seg_cnt[type] ++;
	if(fx_flash_seg_cnt[type] == fx_flash_seg_cnt_max[type]){
		addr = FX_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
		if(fx_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
		FLASH_ErasePage(addr);
		fx_flash_zone_flag ^= (1<<type);
		fx_flash_seg_cnt[type] = 0;
	}
	addr = FX_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
	if(fx_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
	addr += fx_flash_seg_cnt[type] * fx_flash_seg_len[type];

	wt_result = fx_flash_write_half_word(addr, len);
	if(wt_result != OPT_SUCCESS){
		return OPT_ERR_CODE_A;
	}
	addr += 2;
	wt_result = fx_flash_write_half_word(addr, (uint16_t)xor_tmp);
	if(wt_result != OPT_SUCCESS){
		return OPT_ERR_CODE_A;
	}
	addr += 2;
	
	hwlen = len >> 1;
	while(hwlen != 0){
		wt_result = fx_flash_write_half_word(addr, *hwbuf);
		if(wt_result != OPT_SUCCESS){
			return OPT_ERR_CODE_A;
		}
		hwbuf ++;
		addr += 2;
		hwlen --;
	}
	if(len & 0x01){
		data = (*hwbuf) & 0x00FF;
		wt_result = fx_flash_write_half_word(addr, data);
		if(wt_result != OPT_SUCCESS){
			return OPT_ERR_CODE_A;
		}
	}
	return OPT_SUCCESS;
}

static uint8_t fx_stream_write_fin_flag;

void ax_flash_info_stream_write_start(void)
{
	fx_stream_write_fin_flag = 0;
}

void ax_flash_info_stream_write_stop(void)
{
	fx_stream_write_fin_flag = 1;
}


/// 信息数据每次传递2B，直到fin_flag置位结束本轮写入
opt_result_t ax_flash_info_stream_write(st_flash_type_t type, uint16_t data)
{
	static uint32_t addr, head_addr;		// 静态变量!!
	static uint8_t xor_tmp = 0, stream_len = 0;	// 静态变量!!
	static uint8_t using_flag = 0;	// 本轮进行中值为(flash_type+1)
	//uint16_t hwlen, i;
	opt_result_t wt_result;

	if(type > FX_FLASH_CFG_CNT_USED) return OPT_ERR_CODE_SP;
	if(!using_flag){
		fx_flash_seg_cnt[type] ++;
		if(fx_flash_seg_cnt[type] == fx_flash_seg_cnt_max[type]){
			addr = FX_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
			if(fx_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
			FLASH_ErasePage(addr);
			fx_flash_zone_flag ^= (1<<type);
			fx_flash_seg_cnt[type] = 0;
		}
		addr = FX_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
		if(fx_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
		addr += fx_flash_seg_cnt[type] * fx_flash_seg_len[type];
		head_addr = addr;
		xor_tmp = 0;
		stream_len = 0;
		using_flag = type + 1;
	}
	if(using_flag != (type + 1))	return OPT_ERR_CODE_G;	
							// 写入的类型与之前的stream类型不匹配
	if(stream_len > fx_flash_info_len_max[type]) return OPT_FAILUR;

	xor_tmp ^= (uint8_t)data;
	xor_tmp ^= (uint8_t)(data >> 8);
	wt_result = fx_flash_write_half_word(addr, data);
	if(wt_result != OPT_SUCCESS){
		return OPT_ERR_CODE_A;
	}
	addr += 2;
	stream_len += 2;
	
	if(fx_stream_write_fin_flag){
		using_flag = 0;
		wt_result = fx_flash_write_half_word(head_addr, stream_len);
		if(wt_result != OPT_SUCCESS){
			return OPT_ERR_CODE_A;
		}
		wt_result = fx_flash_write_half_word(head_addr + 2, (uint16_t)xor_tmp);
		if(wt_result != OPT_SUCCESS){
			return OPT_ERR_CODE_A;
		}
	}

	return OPT_SUCCESS;
}

// flash read : read all of the config info one time, 
// even thought user maybe do not want all the things.
// 返回值共包含3部分: 执行结果、有效数据缓存地址、有效数据长度
// 需确保写入后读取，最好能保证在没写入就读取的情况下也能正确返回错误信息
opt_result_t ax_flash_config_info_read(st_flash_type_t type, 
											char **buf, uint16_t *len)
{
	uint32_t addr;
	uint16_t *hwbuf = fx_flash_read_buf;
	uint8_t *rd_buf = (uint8_t *)fx_flash_read_buf;
	uint8_t ixor = 0;
	uint16_t ass_data, truelen, hwlen;
	
	if(type > FX_FLASH_CFG_CNT_USED) return OPT_ERR_CODE_SP;
	//if(len == 0) return OPT_SUCCESS;
	
	addr = FX_FLASH_CFG_BASE_ADDR + ST_FLASH_PAGE_SIZE * (type << 1);
	if(fx_flash_zone_flag & (1<<type)) addr += ST_FLASH_PAGE_SIZE;
	addr += fx_flash_seg_cnt[type] * fx_flash_seg_len[type];

	truelen = *(__IO uint16_t*)addr;
	if(truelen > fx_flash_info_len_max[type]) return OPT_ERR_CODE_A;
	hwlen = ((truelen + 1) >> 1);
	addr += 2;
	ass_data = *(__IO uint16_t*)addr;
	ixor = (uint8_t)ass_data;
	ixor ^= (uint8_t)truelen;
	ixor ^= (uint8_t)(truelen >> 8);

	// read to local buffer for verifying
	while(hwlen -- != 0){
		addr += 2;
		*hwbuf = *(__IO uint16_t*)addr;
		hwbuf ++;
	}
	
	hwlen = (truelen + 1) & 0xFFFE;
	while((hwlen --) != 0){
		ixor ^= *rd_buf;
		rd_buf ++;
	}
	if(ixor != 0){
		debug_info_output("Flash read xor wrong!\r\n");
		return OPT_ERR_CODE_B;
	}
	
	*buf = (char *)(&fx_flash_read_buf[0]);	// FX_SEG_HEAD_LEN
	*len = truelen;
	
	#if 0
	if(len <= truelen){
		ax_stdlib_buffer_copy((char *)&fx_flash_read_buf, buf, len);
	}else{
		ax_stdlib_buffer_copy((char *)&fx_flash_read_buf, buf, truelen);
		rd_buf = (uint8_t *)&buf[truelen];
		for(; truelen < len; truelen ++, rd_buf ++){
			*rd_buf = FX_FLASH_USER_DATA_ADD;
		}
		//return OPT_RESULT_A;
	}
	#endif
	
	return OPT_SUCCESS;
}



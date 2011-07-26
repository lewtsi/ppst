/**
 * Use inner flash to save user parameters

 * �ڲ�flash��92k��128kΪ���������ݴ洢����ÿ2*2k�ռ�Ϊһ��������zone
 * ÿ������Ϊ30�ֽ�+1�ֽ�ʹ�ñ�־(��)+1�ֽ�У����(��)��2048/32=64��
 * �ܹ�36k�ռ�ɴ洢9����ͬ�����������ɾ����޸ļ���16�������

 * ��д��һ��������Ϣʱ����һ�ֽ�д�볤�ȣ�֮����У��ֵ�����������
 * ������λ��ʱ����һ�ֽڲ�ΪFF��Ϊ���ã�����ʱʹ�÷ֶβ���
 * 64����8�Σ�ÿ�β��Ҷ��ײ�Ϊff�������һ��
 *
 * ������һ�ֽڱ�ʾ�����Ҳ�ΪFF����˵���������Ϣ��������Ч����Ϊ254��
 * Ҳ����˵������Ϣ�����ռ�ռ��Ϊ256�ֽڡ�
 *
 * ��Ver.0.01a��ʼ���������õĳ��ȸ�Ϊ�ɱ���ǰ�����ķ�ʽ��
 * ����Ϊ�˱�֤���и��ߵ����÷�Χ����Ϣ���ȸ�Ϊ���ֽڱ�ʾ��
 * ��󳤶�Ϊ(2048-4)��



 * | ��Ч���ݳ���[2B] | У��ֵ[2B] | ��Ч���� ... |
 **/
#include "ax_flash.h"
#include "stm32_com.h"


#define FX_FLASH_CFG_BASE_ADDR	((uint32_t)0x08017000)	// 92K

#define FX_FLASH_CFG_CNT_MAX	((uint8_t)9)	// �����ж�����������Ϣ

#define FX_SEG_HEAD_LEN			((uint8_t)4)	// ����ͷ���ݳ���
#define FX_VERIFY_POS			((uint8_t)2)	// У��ֵ��λ��
#define FX_FLASH_USER_DATA_ADD	((uint8_t)0x30)	// �û���ȡ����ʵ�ʵĲ�������

// ********************** modify segments (OP) ********************** //

#define FX_FLASH_CFG_CNT_USED	((uint8_t)3)	// ����ʹ�õ���������
#define FX_FLASH_SEG_LEN_MAX	((uint16_t)256)	// ��������ռ�õ����ռ䳤��
static uint16_t fx_flash_seg_len[FX_FLASH_CFG_CNT_USED] = { 32, 64, 256 };
			//<! ������Ϣռ�õĿռ䳤�� ( MUST BE 2^n !!!)
static uint8_t fx_flash_seg_cnt_max[FX_FLASH_CFG_CNT_USED] = { 64, 32, 8 };
			//<! 2K/fx_flash_seg_len (�ֶ���ֵ)

// ********************** modify segments (ED) ********************** //

static uint16_t fx_flash_info_len_max[FX_FLASH_CFG_CNT_USED];
			//<! ������Ч���ݳ���(seg_len - 4)

static uint16_t fx_flash_zone_flag;	// zone����ѡ��16������
static uint8_t fx_flash_seg_cnt[FX_FLASH_CFG_CNT_USED];	// �������õ�segment cnt

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

	if(fx_flash_init_flag) return OPT_SUCCESS;	// �Ѿ���ʼ��
	
	*result = 0;
	ass_addr = FX_FLASH_CFG_BASE_ADDR;
	while(i_zone < FX_FLASH_CFG_CNT_USED){	// �����0��ʼ
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
				// û�����ݣ���λ��Ӧ��resultλ
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


/// ��Ϣ����ÿ�δ���2B��ֱ��fin_flag��λ��������д��
opt_result_t ax_flash_info_stream_write(st_flash_type_t type, uint16_t data)
{
	static uint32_t addr, head_addr;		// ��̬����!!
	static uint8_t xor_tmp = 0, stream_len = 0;	// ��̬����!!
	static uint8_t using_flag = 0;	// ���ֽ�����ֵΪ(flash_type+1)
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
							// д���������֮ǰ��stream���Ͳ�ƥ��
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
// ����ֵ������3����: ִ�н������Ч���ݻ����ַ����Ч���ݳ���
// ��ȷ��д����ȡ������ܱ�֤��ûд��Ͷ�ȡ�������Ҳ����ȷ���ش�����Ϣ
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



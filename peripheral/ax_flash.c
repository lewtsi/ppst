/**
 * Use inner flash to save user parameters

 * �ڲ�flash��92k��128kΪ���������ݴ洢����ÿ2*2k�ռ�Ϊһ��������zone
 * ÿ������Ϊ30�ֽ�+1�ֽ�ʹ�ñ�־(��)+1�ֽ�У����(��)��2048/32=64��
 * �ܹ�36k�ռ�ɴ洢9����ͬ�����������ɾ����޸ļ���16�������

 * ��д��һ��������Ϣʱ����һ�ֽ�д�볤�ȣ�֮����У��ֵ�����������
 * ������λ��ʱ����һ�ֽڲ�ΪFF��Ϊ���ã�����ʱʹ�÷ֶβ���
 * 64����8�Σ�ÿ�β��Ҷ��ײ�Ϊff�������һ��
 **/
#include "ax_flash.h"


#define ST_FLASH_CFG_BASE_ADDR	((uint32_t)0x08017000)	// 92K

#define ST_FLASH_CFG_LEN_MAX	((uint8_t)30)	// ���������ֳ���
#define ST_FLASH_SEG_LEN		((uint8_t)32)	// ÿ������ռ�ÿռ䳤��
#define ST_FLASH_SEG_CNT_MAX	((uint8_t)64)	// 2k/32=64
#define ST_FLASH_CFG_CNT_MAX	((uint8_t)9)	// �����ж�����������Ϣ
#define ST_FLASH_CFG_CNT_USED	((uint8_t)3)	// ����ʹ�õ���������

#define ST_FLASH_USER_DATA_ADD	((uint8_t)0x30)	// �û���ȡ����ʵ�ʵĲ�������

static uint16_t st_flash_zone_flag;	// zone����ѡ��16������
static uint8_t st_flash_seg_cnt[ST_FLASH_CFG_CNT_USED];	// �������õ�segment cnt

static uint16_t st_flash_read_buf[ST_FLASH_SEG_LEN >> 1];

opt_result_t ax_flash_config_info_init(void)
{
	uint8_t i_zone = 0;
	uint8_t seek1;
	uint32_t addr, addr_ass;

	addr_ass = ST_FLASH_CFG_BASE_ADDR;
	//seek1_intv = ST_FLASH_SEG_LEN << 3;
	while(i_zone < ST_FLASH_CFG_CNT_USED){	// �����0��ʼ
		addr = addr_ass;
		if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
			st_flash_zone_flag |= (1<<i_zone);
			addr += ST_FLASH_PAGE_SIZE;
			if(*(__IO uint16_t *)addr == (uint16_t)0xFFFF){
				// û�����ݣ���ʼ��д��
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
// ����û���ȡ�ĳ��ȴ���ʵ���е����ݳ��ȣ������������㲹�䣬�����س���
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



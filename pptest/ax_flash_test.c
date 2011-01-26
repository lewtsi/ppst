
#include "ax_flash_test.h"
#include "ax_usart.h"

static char cfg_init_info[3][72] = {
	"CFG@INFO = 13abcdefg\r\n",	// 23
	"CFG#INFO = 13ABCDEFG0123456789ABCDEFG\r\n",	// 40
	"CFG$INFO = 13ABCDEFG0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\r\n"	// 71
};
static uint8_t init_info_len[3] = {23, 40, 71};
static char info_rd[80];


opt_result_t fx_flash_test_init(void)
{
	uint8_t i = 0;
	uint16_t init_result;
	FLASH_Unlock();
	
	if(ax_flash_config_info_init(&init_result) != OPT_SUCCESS){
		return OPT_FAILURE;
	}
	if(init_result == 0) return OPT_SUCCESS;

	while(i < 3){
		if(init_result & (1 << i)){
			if(ax_flash_config_info_write((st_flash_type_t)i, cfg_init_info[i],
					init_info_len[i]) != OPT_SUCCESS){
				ax_usart_send_char_message("Flash Init Wr failure!\r\n");
			}
		}
		i ++;
	}
	return OPT_SUCCESS;
}

void fx_flash_test_loop(void)
{
	uint8_t i, data = '0', data2 = '0';
	uint16_t cnt;
	opt_result_t opt_result;
	
	while(1){
		for(i=0; i<3; i++){
			opt_result = ax_flash_config_info_write((st_flash_type_t)i, 
								cfg_init_info[i], init_info_len[i]);
			if(opt_result == OPT_SUCCESS){
				if(ax_flash_config_info_read((st_flash_type_t)i, 
					info_rd, init_info_len[i]) != OPT_SUCCESS){
					ax_usart_send_char_message("Flash Read error!\r\n");
				}else{
					ax_usart_send_char_message(info_rd);
				}
			}else{
				ax_usart_send_char_message("ERR: data A read error!\r\n");
			}
			cfg_init_info[i][0] = data2;
			cfg_init_info[i][1] = data;
		}
		if(++ data > '9'){
			data = '0';
			if(++ data2 > '9') data2 = '0';
		}
		cnt = 0xFFFF;
		while(cnt -- != 0);
		ax_usart_send_char_message("--------------------------------\r\n");
	}
}

void ax_flash_test(void)
{
	fx_flash_test_init();
	fx_flash_test_loop();
}



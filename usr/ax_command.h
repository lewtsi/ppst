
#ifndef AX_COMMAND_H
#define AX_COMMAND_H

#include "ax_common.h"

typedef enum{
	CMD_INDEX_ALL = 0,
	CMD_INDEX_NOD = 1,
	CMD_INDEX_SAD,
	CMD_INDEX_UNKNOWN = 0xEE
}ax_cmd_index_t;


typedef struct{
	ax_cmd_index_t index;
	char * command;
	uint8_t cmd_len;
	uint8_t sum;
	uint8_t xor;	// 暂时不使用
	uint8_t res;
	opt_result_t (*func)(char *, uint8_t);	// 处理函数指针
}ax_char_command_t;


opt_result_t ax_char_command_identify_check(void);


#endif




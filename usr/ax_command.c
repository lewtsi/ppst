/**
 * 配置字、指令的识别，另外考虑到程序的完整性，本文件还将处理指令。
 * 从功能上可以看出，本文件将放在较高层次的目录下，暂时决定放在/usr下。
 *
 * 关于此次的CMD指针传递，传递的是从有明显区别的字符开始的字符串；
 * 例如at+ifp,at+cfg,首先应该过滤前面3个相同的字符，将[3]指针传入
 **/

#include "ax_command.h"
#include "ax_usart.h"
#include "ax_stdlib.h"

#define FX_CMD_DEBUG_ON		1

#define FX_CMD_USED_CNT		(0x03)

_fx opt_result_t fx_char_cmd_all(char *cmd, uint8_t ass);
_fx opt_result_t fx_char_cmd_nod(char *cmd, uint8_t ass);
_fx opt_result_t fx_char_cmd_sad(char *cmd, uint8_t ass);


ax_char_command_t char_command[FX_CMD_USED_CNT] = {
		{
			CMD_INDEX_ALL,
			"ALL",			// 41-4C-4C
			3,
			'A',
			0, 0,	// xor, res
			fx_char_cmd_all,
		},{
			CMD_INDEX_NOD,
			"NOD",			// 4E-4F-44
			3,
			'N',
			0, 0,
			fx_char_cmd_nod,
		},{
			CMD_INDEX_SAD,
			"SAD",			// 51-41-44
			3,
			'S',
			0, 0,
			fx_char_cmd_sad,
		}
};

/// char command sum value generation
/// \note if identify function check error, this function MUST be changed.
_fx uint8_t fx_char_cmd_mksum(char *cmd)
{
	return (*cmd);
}

/// command execution: AT+ALL, cmd DO NOT include AT+ALL
_fx opt_result_t fx_char_cmd_all(char *cmd, uint8_t ass)
{
	ax_usart_send_string("CALL EXE\r\n", 10);
	return OPT_SUCCESS;}

/// command execution: AT+NOD	\r\n
/// AT+NOD?  or  AT+NOD=xxxx	\r\n
/// cmd -> ? or =xxxx
_fx opt_result_t fx_char_cmd_nod(char *cmd, uint8_t ass)
{
	if(*cmd == '?'){
		ax_usart_send_string("NOD? EXE\r\n", 10);
		return OPT_SUCCESS;
	}else if(*cmd == '='){
		ax_usart_send_string("NOD=xxxx EXE\r\n", 14);
		return OPT_SUCCESS;
	}
	return OPT_FAILURE;
}

/// command execution: AT+SAD
_fx opt_result_t fx_char_cmd_sad(char *cmd, uint8_t ass)
{
	if(*cmd == '='){
		ax_usart_send_string("SAD=xxxx EXE\r\n", 14);
		return OPT_SUCCESS;
	}
	return OPT_FAILURE;
}

/// identify the char command
/// \param index: the value of command index, NOT a input, but a output value
/// \param cmd: the char command buffer pointer
/// \param len: not used
/// \retval OPT_SUCCESS:identify ok, and executed function ok
/// 		OPT_ERR_CODE_SP: Could NOT identify the char command
///			OPT_XXX: identify ok, but executed has something wrong.
opt_result_t ax_char_command_identify(uint8_t *index, char *cmd, uint8_t len)
{
	uint8_t i = FX_CMD_USED_CNT, sum;
	opt_result_t opt_result;
	
	sum = fx_char_cmd_mksum(cmd);
	while(i != 0){
		-- i;
		if(sum == char_command[i].sum){
			if(ax_buffer_compare(cmd, char_command[i].command, 
									char_command[i].cmd_len) != OPT_SUCCESS){
				break;
			}
			if(index != NULL) *index = i;
			cmd += char_command[i].cmd_len;
			opt_result = (*char_command[i].func)(cmd, 0);
			return opt_result;
		}
	}
	return OPT_ERR_CODE_SP;
}

/// DEBUG: check function identify char command sum 
opt_result_t ax_char_command_identify_check(void)
{
	#if(FX_CMD_DEBUG_ON)
	uint8_t i = FX_CMD_USED_CNT, j;
	uint8_t sum_cur;
	while(i != 0){
		-- i;
		sum_cur = char_command[i].sum;
		for(j=0; j<i; j++){
			if(char_command[j].sum == sum_cur)
				return OPT_FAILURE;
		}
	}
	#endif
	return OPT_SUCCESS;
}






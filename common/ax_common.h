/**
 * 与平台无关的全局定义

 **/
 
#ifndef AX_COMMON_H
#define AX_COMMON_H

#include "ax_common_st.h"

#define _fx static

typedef enum{
	OPT_SUCCESS = 0x00,
	OPT_FAILUR = 0xFF,
	OPT_ERR_CODE_A = 0x01,
	OPT_ERR_CODE_B,
	OPT_ERR_CODE_C,
	OPT_ERR_CODE_D,
	OPT_ERR_CODE_E,
	OPT_ERR_CODE_F,
	OPT_ERR_CODE_G,
	OPT_ERR_CODE_SP,
	OPT_RESULT_A = 0xA0,
	OPT_RESULT_B,
	OPT_RESULT_C,
	OPT_RESULT_D
}opt_result_t;

#endif


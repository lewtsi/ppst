/**
 * 与平台无关的全局定义

 **/
 
#ifndef AX_COMMON_H
#define AX_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include "ax_common_st.h"

#define _fx static
#define DEBUG_MSG_ON	1

#define FLAG_FALSE	0
#define FLAG_TRUE	1

typedef enum{
	OPT_SUCCESS = 0x00,
	OPT_FAILURE = 0x7F,
	OPT_ERR_CODE_A = 0x01,
	OPT_ERR_CODE_B,
	OPT_ERR_CODE_C,
	OPT_ERR_CODE_D,
	OPT_ERR_CODE_E,
	OPT_ERR_CODE_F,
	OPT_ERR_CODE_G,
	OPT_ERR_CODE_SP,
	OPT_RESULT_A = 0x80,
	OPT_RESULT_B = 0x81,
	OPT_RESULT_C = 0x82,
	OPT_RESULT_D = 0x84,
	OPT_RESULT_E = 0x90,
	OPT_RESULT_F = 0xA0,
	OPT_RESULT_G = 0xC0,
	OPT_RESULT_FALSE = 0x00,
	OPT_RESULT_TRUE = 0x01
}opt_result_t;

#endif


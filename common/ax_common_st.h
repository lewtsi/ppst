/**
 * 与平台相关的定义
 **/

#ifndef AX_COMMON_ST_H
#define AX_COMMON_ST_H

#include "stm32f10x.h"


#define NVIC_GLOBAL_PRIORITYGROUP NVIC_PriorityGroup_3
// 抢占优先级
#define NVIC_PreemptionPriority_USART 6	
#define NVIC_SubPriority_USART2	0


#define BKP_DR_VALUE BKP_DR1

#define BKP_DR_WAKEUP_TYPE		BKP_DR3	//<! 休眠唤醒类型(定点0xAA55或间隔0x0)
#define BKP_DR_FIXED_CNT_TOTLE	BKP_DR4	//<! 定点信息总数量
#define BKP_DR_FIXED_CNT_NOW	BKP_DR5	//<! 当前使用的是第几个定点信息
#define BKP_DR_FIXED_TIME_H		BKP_DR6	//<! 当前使用的定点时间高字节
#define BKP_DR_FIXED_TIME_L		BKP_DR7	//<! 对应低字节
#define BKP_DR_FIXED_RUN_TIME	BKP_DR8	//<! 定点唤醒的运行时间

#define FLASH_SPACE_TYPE_FIXED_TIME	2	//<! 保存固定时间点信息

#endif


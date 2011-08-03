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


/// ********************* USART相关定义 *********************

#define AXUSART_AUTO_XOR_ON			1

#define AXUSART_RX_BUFFER_CNT		(2)
#define AXUSART_RX_BUFFER_SIZE		( 118 )
#define AXUSART_RX_MAX_BYTES		(AXUSART_RX_BUFFER_SIZE - 4)

#define AXUSART_TYPE_232    (1)
#define AXUSART_TYPE_485    (2)

#define AXUSART_PARITY_NON	0
#define AXUSART_PARITY_ODD	1
#define AXUSART_PARITY_EVEN	2

#define AXUSART_DATA_BIT9		9
#define AXUSART_DATA_BIT8		8

#define AXUSART_STOP_BIT1     1
#define AXUSART_STOP_BIT2     2

#define AXUSART_TIMEOUT_DEFAULT		20		// 20ms
#define AXUSART_TIMEOUT_MAX_MASK	0x3F	// 63ms

#define AXUSART_ERR_PRT			(0x01<<0)	// ERROR code must larger than zero
#define AXUSART_ERR_BAUDRATE	(0x01<<1)
#define AXUSART_ERR_CHARSIZE	(0x01<<2)
#define AXUSART_ERR_PARITY		(0x01<<3)
#define AXUSART_ERR_STOPBIT		(0x01<<4)
#define AXUSART_ERR_TIMEOUT		(0x01<<5)

typedef struct{
	uint8_t type;	// 232/485
	USART_TypeDef uport;
	uint8_t baudrate;
	uint8_t datalen;
	uint8_t parity;
	uint8_t stopbit;
	uint8_t timeout;	// ms
	uint8_t func_flag;
}st_usart_para_t;

typedef enum{
	ax_usart_flowCtrl_en = 0x01,
	ax_usart_resA = 0x02,
	ax_usart_resB = 0x04
}st_usart_func_flag_t;

typedef enum{	//Main Freq= 72MHz
	BR_2400  = 0,
	BR_4800,
	BR_9600,
	BR_19200,
	BR_38400,
	BR_57600,
	BR_115200,
	BR_230400
}st_usart_baudrate_t;
#define AXUSART_BR_9600_CNT 3
#define AXUSART_BR_115200_CNT 7




#endif


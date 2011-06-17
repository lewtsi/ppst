/**
 * ��ƽ̨��صĶ���
 **/

#ifndef AX_COMMON_ST_H
#define AX_COMMON_ST_H

#include "stm32f10x.h"


#define NVIC_GLOBAL_PRIORITYGROUP NVIC_PriorityGroup_3
// ��ռ���ȼ�
#define NVIC_PreemptionPriority_USART 6	
#define NVIC_SubPriority_USART2	0


#define BKP_DR_VALUE BKP_DR1

#define BKP_DR_WAKEUP_TYPE		BKP_DR3	//<! ���߻�������(����0xAA55����0x0)
#define BKP_DR_FIXED_CNT_TOTLE	BKP_DR4	//<! ������Ϣ������
#define BKP_DR_FIXED_CNT_NOW	BKP_DR5	//<! ��ǰʹ�õ��ǵڼ���������Ϣ
#define BKP_DR_FIXED_TIME_H		BKP_DR6	//<! ��ǰʹ�õĶ���ʱ����ֽ�
#define BKP_DR_FIXED_TIME_L		BKP_DR7	//<! ��Ӧ���ֽ�
#define BKP_DR_FIXED_RUN_TIME	BKP_DR8	//<! ���㻽�ѵ�����ʱ��

#define FLASH_SPACE_TYPE_FIXED_TIME	2	//<! ����̶�ʱ�����Ϣ

#endif


#if 0
MDK 项目属性
C/C++ 
Preprocessor Symbols Define: STM32F10X_CL,USE_STDPERIPH_DRIVER
Include Paths:	..\Libraries\CMSIS\Core\CM3;
				..\Libraries\STM32F10x_StdPeriph_Driver\inc;
				..\Libraries\STM32_ETH_Driver\inc;..\Sources\common;
				..\Sources\core;..\Sources\peripheral
Optimization: -O3


该项目将包括以下几个部分（暂定）
GPIO
EXTI
USART
TIMER
SPI
RTC
CRC
DMA
ADC
DAC
WDT
SDIO

LAN
USB
CAN

另外考虑添加实时操作系统，但可能会另建项目。



2010/11/22
开始创建ppst项目。该项目的目标是建立一个STM32相关的示例性质的工程。

编译器：MDK-v4.11   EWARM-v5.50
标准库：STM32F10x_StdPeriph_Driver-2010-4-23           （请自行添加）

关于文件命名：普通外设操作文件使用p_xxx.c格式，main文件采用zppst.c，方便source insight管理。

其他说明：注释遵循doxygen格式；使用github管理该工程项目；

注：这是我的第一个git管理的项目，摸索中前进吧。

#endif 

﻿



v0.03a	2011/06/17
	添加IIC驱动；
	添加传感器TMP102，hall switch；移植前测试通过，移植后未测试。

V0.02a	2011/01/26
	添加定点和间隔唤醒功能；
	考虑到降低RAM空间的占用，对flash read函数修改。

V0.01a	2011/01/26
	版本记录起始；
	为flash写入添加单独可更改的数据长度；


============================================================

2010/11/31

添加RTC Calendar & Alarm Wakeup功能。


2010/11/29

完成RTC和timer的基本功能测试。


2010/11/25

flash部分测试完成。


2010/11/24

添加了FLASH写入和读取配置信息功能；
FLASH读取测试未完成。


2010/11/23

添加了GPIO USART等功能


2010/11/22

开始创建工程项目。该项目的目标是建立一个STM32相关的示例性质的工程。

编译器：MDK-v4.11   EWARM-v5.50
标准库：STM32F10x_StdPeriph_Driver-2010-4-23           （请自行添加）
关于文件命名：普通外设操作文件使用ax_xxx.c格式，main文件采用zppst.c，
方便source insight管理。
其他说明：注释遵循doxygen格式；使用github管理该工程项目；
The remote git project name is lewtsi/ppst.git, and it is called githubppst in local host.

这是我的第一个git管理的项目，摸索中前进吧。

============================================================
Version命名规则
版本命名分为三部分：0.01a
第一部分表示整体重要改变或成熟性（1-2位十进制数字）；
第二部分表示功能的添加或改变的记录（2位十进制数字）；
第三部分表示某些功能改变的进度指示（1小写字母）；




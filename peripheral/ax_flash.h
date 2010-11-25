#ifndef AX_FLASH_H
#define AX_FLASH_H

#include "ax_stdlib.h"

#if defined (STM32F10X_HD) || defined (STM32F10X_CL)
	#define ST_FLASH_PAGE_SIZE  (uint32_t)0x800
#else
	#warning "<st_flash.h> Maybe you need to modify the flash page size."
#endif

typedef enum{
	FLASH_DATA_TYPE_A = 0,
	FLASH_DATA_TYPE_B = 1,
	FLASH_DATA_TYPE_C,
	FLASH_DATA_TYPE_D,
	FLASH_DATA_TYPE_E,
	FLASH_DATA_TYPE_F,
	FLASH_DATA_TYPE_G,
	FLASH_DATA_TYPE_H
}st_flash_type_t;

opt_result_t ax_flash_config_info_init(void);
opt_result_t ax_flash_config_info_write(st_flash_type_t type, 
										char *buf, uint8_t len);
opt_result_t ax_flash_config_info_read(st_flash_type_t type, 
										char *buf, uint8_t len);

#endif


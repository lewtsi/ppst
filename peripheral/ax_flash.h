#ifndef AX_FLASH_H
#define AX_FLASH_H

#include "ax_common.h"

#if defined (STM32F10X_HD) || defined (STM32F10X_CL)
	#define ST_FLASH_PAGE_SIZE  (uint32_t)0x800
#else
	#warning "<st_flash.h> Maybe you need to modify the flash page size."
#endif

//<! MUST be 0 1 2 ...
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

opt_result_t ax_flash_config_info_init(uint16_t *result);
opt_result_t ax_flash_config_info_write(st_flash_type_t type, 
										char *buf, uint16_t len);
void ax_flash_info_stream_write_start(void);
void ax_flash_info_stream_write_stop(void);
opt_result_t ax_flash_info_stream_write(st_flash_type_t type, uint16_t data);
opt_result_t ax_flash_config_info_read(st_flash_type_t type, 
										char **buf, uint16_t *len);
#endif


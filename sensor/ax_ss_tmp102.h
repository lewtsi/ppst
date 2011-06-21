#ifndef AX_SS_TMP102_H
#define AX_SS_TMP102_H

#include "ax_common.h"
#include "ax_i2c.h"

typedef struct{
	unsigned SD : 1;
	unsigned TM : 1;
	unsigned POL: 1;
	unsigned FQ	: 2;
	unsigned R	: 2;	// converter resolution (read only)
	unsigned OS : 1;
	unsigned RES: 4;
	unsigned EM	: 1;
	unsigned AL : 1;	// (read only)
	unsigned CR	: 2;	// conversion rate
}tmp102_config_t;

typedef struct{
	unsigned rwflag : 1;
	unsigned addr : 7;
}tmp102_address_t;

typedef enum{
	SD_DISABLE = 0,
	SD_ENABLE = 1
}tmp102_shutdown_t;

typedef enum{
	COMPARATOR_MODE = 0,
	INTERRUPT_MODE = 1
}tmp102_thermostat_t;

typedef enum{
	POL_DISABLE = 0,
	POL_ENABLE = 1
}tmp102_polarity_t;

typedef enum{
	CONSECUTIVE_FAULTS_1 = 0,
	CONSECUTIVE_FAULTS_2,
	CONSECUTIVE_FAULTS_4,
	CONSECUTIVE_FAULTS_6
}tmp102_fault_queue_t;

typedef enum{
	ONE_SHOT_DISABLE = 0,
	ONE_SHOT_ENABLE = 1
}tmp102_one_shot_t;

typedef enum{
	NORMAL_MODE = 0,
	EXTENDED_MODE = 1
}tmp102_extended_mode_t;

typedef enum{
	CR_0_25_HZ = 0,
	CR_1_HZ,
	CR_4_HZ,
	CR_8_HZ
}tmp102_conversion_rate_t;

typedef enum{
	ADDR_GND = 0x48,
	ADDR_VCC = 0X49,
	ADDR_SDA = 0X4A,
	ADDR_SCL = 0X4B
}tmp102_addr_t;

typedef enum{
	RWFLAG_W = 0,
	RWFLAG_R = 1,
	RWMASK_W = 0xFE,	// & MASK
	RWMASK_R = 0x01		// | MASK
}tmp102_rw_t;

typedef enum{
	POINTER_TMP_REG = 0,
	POINTER_CFG_REG,
	POINTER_TLOW_REG,
	POINTER_THIGH_REG
}tmp102_pointer_addr_t;

typedef struct{
	uint8_t addr;
	uint8_t preg;
	uint16_t cfg;
}tmp102_write_t;

typedef struct{
	uint8_t addr;
	uint8_t preg;
	uint16_t *value;
}tmp102_read_t;

opt_result_t ax_ss_tmp102_init(void);
opt_result_t ax_tmp102_read_temp_value(uint8_t addr, uint16_t *value);
opt_result_t ax_tmp102_read_cfg_value(uint8_t addr, uint16_t *value);

#endif

/* EOF */


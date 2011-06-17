#include "ax_ss_tmp102.h"
#include "ax_i2c.h"

static tmp102_address_t tmp102_address;
static tmp102_config_t tmp102_config;
static tmp102_write_t write_config;
static tmp102_read_t read_value;
static tmp102_write_t *p_config = &write_config;
static tmp102_read_t *p_value = &read_value;

// 先写低位，后写高位
_fx opt_result_t fx_tmp102_write_config(tmp102_write_t *cnfg)
{
	uint8_t cfg_h, cfg_l;
	uint16_t cfg_w = (uint16_t)(cnfg->cfg);
	cfg_l = (uint8_t)cfg_w;
	cfg_h = (uint8_t)(cfg_w >> 8);
	
	ax_iic_start();
	ax_iic_write_byte((uint8_t)(cnfg->addr) & RWMASK_W);
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_write_byte((uint8_t)(cnfg->preg));
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_write_byte(cfg_l);
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_write_byte(cfg_h);
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_stop();

	return OPT_SUCCESS;
}

// 先读高位，后读低位。注:STM32为小端存储(L->small H-> large)
_fx opt_result_t fx_tmp102_read_word_reg(tmp102_read_t *cnfg)
{
	
	ax_iic_start();
	ax_iic_write_byte((uint8_t)(cnfg->addr) & RWMASK_W);
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_write_byte((uint8_t)(cnfg->preg));
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	//ax_iic_stop();
	//delay_us(20);
	ax_iic_start();
	ax_iic_write_byte((uint8_t)(cnfg->addr) | RWMASK_R);
	if(ax_iic_chk_ack() == 0) return OPT_FAILURE;
	ax_iic_read_byte((uint8_t *)(cnfg->value) + 1);
	ax_iic_ack();
	ax_iic_read_byte((uint8_t *)(cnfg->value));
	ax_iic_ack();	//iic_nack();
	ax_iic_stop();

	return OPT_SUCCESS;
}

_fx opt_result_t fx_tmp102_read_reg(uint8_t addr, uint8_t reg, uint16_t *value)
{
	p_value->addr = addr << 1;
	p_value->preg = reg;	//;
	p_value->value = value;
	
	return fx_tmp102_read_word_reg(p_value);
}

opt_result_t ax_ss_tmp102_init(void)
{
	ax_iic_init();
	
	tmp102_address.addr = ADDR_GND;
	tmp102_address.rwflag = RWFLAG_W;

	tmp102_config.SD = SD_DISABLE;
	tmp102_config.TM = COMPARATOR_MODE;
	tmp102_config.POL = POL_DISABLE;
	tmp102_config.FQ = CONSECUTIVE_FAULTS_1;
	tmp102_config.R	= 0;	// converter resolution (read only)
	tmp102_config.OS = ONE_SHOT_DISABLE;
	tmp102_config.RES = 0;
	tmp102_config.EM = EXTENDED_MODE;	//NORMAL_MODE;	
	tmp102_config.AL = 0;	// (read only)
	tmp102_config.CR = CR_4_HZ;	// conversion rate
	
	p_config->addr = *((uint8_t *)&tmp102_address);	//0x90;
	p_config->cfg = *((uint16_t *)&tmp102_config);	//0x9000;default:0xa060
	p_config->preg = POINTER_CFG_REG;
	
	if(fx_tmp102_write_config(p_config) != OPT_SUCCESS){
		return OPT_FAILURE;
	}
	//delay_ms(50);
	//tmp102_read_config_reg(ADDR_GND, POINTER_CFG_REG, &tmp16);
	return OPT_SUCCESS;
}

opt_result_t ax_tmp102_read_temp_value(uint8_t addr, uint16_t *value)
{
	if(fx_tmp102_read_reg(addr, POINTER_TMP_REG, value) == OPT_SUCCESS){
		*value >>= 3;	// 数据右对齐
		return OPT_SUCCESS;
	}else{
		return OPT_FAILURE;
	}
}

opt_result_t ax_tmp102_read_cfg_value(uint8_t addr, uint16_t *value)
{
	return fx_tmp102_read_reg(addr, POINTER_CFG_REG, value);
}



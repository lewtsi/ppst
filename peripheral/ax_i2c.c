
/**
 * IIC的SDA线采用OUT_OD模式，所以必须有外置上拉电阻。
 **/

#include "iic_driver.h"

#define FX_IIC_GPIO_PORT	GPIOB
#define FX_IIC_SCL_PIN		GPIO_Pin_10
#define FX_IIC_SDA_PIN		GPIO_Pin_11
#define FX_IIC_RCC			RCC_APB2Periph_GPIOB

#define fx_iic_scl_high()	(FX_IIC_GPIO_PORT->BSRR = FX_IIC_SCL_PIN)
#define fx_iic_scl_low()	(FX_IIC_GPIO_PORT->BRR  = FX_IIC_SCL_PIN)
#define fx_iic_sda_high()	(FX_IIC_GPIO_PORT->BSRR = FX_IIC_SDA_PIN)
#define fx_iic_sda_low()	(FX_IIC_GPIO_PORT->BRR  = FX_IIC_SDA_PIN)
#define fx_iic_sda_value	(FX_IIC_GPIO_PORT->IDR  & FX_IIC_SDA_PIN)

static GPIO_InitTypeDef  GPIO_InitStructure;

void ax_iic_init(void)
{
	RCC_APB2PeriphClockCmd(FX_IIC_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  FX_IIC_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(FX_IIC_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = FX_IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(FX_IIC_GPIO_PORT, &GPIO_InitStructure);
}

uint8_t ax_iic_start(void)
{
	fx_iic_sda_high();
	delay_us(2);
	fx_iic_scl_high();
	delay_us(5);
	fx_iic_sda_low();
	delay_us(3);
	fx_iic_scl_low();
	delay_us(2);

	return 0;
}

void ax_iic_stop(void)
{
	fx_iic_scl_low();
	delay_us(2);
	fx_iic_sda_low();
	delay_us(2);
	fx_iic_scl_high();
	delay_us(5);
	fx_iic_sda_high();
	delay_us(4);
}

void ax_iic_ack(void)
{
	fx_iic_scl_low();
	delay_us(2);
	fx_iic_sda_low();
	delay_us(2);
	fx_iic_scl_high();
	delay_us(3);
	fx_iic_scl_low();
	delay_us(2);
	fx_iic_sda_high();
	delay_us(2);
}

void ax_iic_nack(void)
{
	fx_iic_scl_low();
	delay_us(2);
	fx_iic_sda_high();
	delay_us(2);
	fx_iic_scl_high();
	delay_us(5);
	fx_iic_scl_low();
	delay_us(2);
}

uint8_t ax_iic_chk_ack(void)
{
	uint8_t iic_flag = 0;
	
	fx_iic_scl_low();
	delay_us(3);
	fx_iic_scl_high();
	delay_us(2);
	if(fx_iic_sda_value == 0)
		iic_flag = 1;
	fx_iic_scl_low();
	delay_us(2);
	
	return iic_flag;
}

uint8_t ax_iic_write_byte(uint8_t data)
{
	uint8_t i = 8;
	for(i=0; i<=7; i++){
		fx_iic_scl_low();
		delay_us(2);
		if(data & 0x80)
			fx_iic_sda_high();
		else fx_iic_sda_low();
		delay_us(2);
		fx_iic_scl_high();
		delay_us(3);
		data <<= 1;
	}
	fx_iic_scl_low();
	delay_us(2);
	
	return 0;
}

uint8_t ax_iic_read_byte(uint8_t *data)
{
	uint8_t i, tmp = 0;
	for(i=0; i<=7; i++){
		fx_iic_scl_low();
		delay_us(3);
		fx_iic_scl_high();
		delay_us(2);
		tmp <<= 1;
		if(fx_iic_sda_value)
			tmp |= 0x01;
		delay_us(1);
	}
	fx_iic_scl_low();	// sp.

	*data = tmp;

	return 0;
}

uint8_t ax_iic_dev_write_byte(uint8_t addr, uint8_t data)
{
	ax_iic_start();
	ax_iic_write_byte(0xa0);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_write_byte(addr);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_write_byte(data);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_stop();

	return 1;
}

uint8_t ax_iic_dev_read_byte(uint8_t addr, uint8_t *data)
{
	ax_iic_start();
	ax_iic_write_byte(0xa0);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_write_byte(addr);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_start();
	ax_iic_write_byte(0xa1);
	if(ax_iic_chk_ack() == 0) return 0;
	ax_iic_read_byte(data);
	ax_iic_nack();
	ax_iic_stop();

	return 1;
}



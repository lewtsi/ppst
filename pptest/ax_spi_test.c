#include "ax_spi_test.h"

#define FX_SPI_MODE_MASTER	0xAA
#define FX_SPI_MODE_SLAVER	0x55
#ifdef STM32F10X_CL
#define FX_SPI_WORK_MODE	FX_SPI_MODE_MASTER
#else
#define FX_SPI_WORK_MODE	FX_SPI_MODE_SLAVER
#endif

#define FX_SPI_MASTER_DMA_EN	0
#define FX_SPI_SLAVER_DMA_EN	1

#define FX_DATA_SIZE	((uint8_t)4)
#define FX_DATA_LEN		((uint16_t)1000)
#define FX_DATA_SAPCE	(FX_DATA_LEN * FX_DATA_SIZE)
static char spi_data_buffer[FX_DATA_SIZE][FX_DATA_LEN];
static char * spi_data_buf_cur = spi_data_buffer[0]; // 指向当前写入的buffer
static volatile uint8_t spi_buffer_index = 0;
static volatile uint16_t spi_data_counter = 0;

static char spi_trans_data = 0;
static volatile uint8_t spi_rcv_fin_flag;
static volatile uint8_t spi_tx_fin_flag;

// RDY管脚 MASTER - PE0  SLAVER - PB11
// 发送方MASTER为输入，接收方SLAVER为输出
_fx void fx_spi_ready_signal_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	#if(FX_SPI_WORK_MODE == FX_SPI_MODE_MASTER)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	#else
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	#endif
}

_fx void fx_spi_slaver_trans_enable(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_11);
}

_fx void fx_spi_slaver_trans_disable(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

// 检查spi允许发送数据，低电平运行，高电平停止
_fx uint8_t fx_spi_master_check_status(void)
{
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0))
		return 0;
	else return 1;
}

_fx void fx_spi_test_init(void)
{
	uint16_t i;
	fx_spi_ready_signal_init();
	#if(FX_SPI_WORK_MODE == FX_SPI_MODE_MASTER)
		#if(FX_SPI_MASTER_DMA_EN)
		#else
		ax_spi_master_init();
		#endif
		ax_spi_master_ss_start();
		//ax_timerA_init(TCNT16_500MS);
	#else
		for(i=0; i<FX_DATA_SAPCE; i++){
			*spi_data_buf_cur++ = 0xA8;
		}
		spi_data_buf_cur = spi_data_buffer[0];
		#if(FX_SPI_SLAVER_DMA_EN)
		ax_spi_slaver_dma_1line_init(spi_data_buffer[0], FX_DATA_LEN);
		#else
		ax_spi_slaver_init();
		#endif
		ax_timerA_init(TCNT16_500MS);
		fx_spi_slaver_trans_enable();
	#endif
}

_fx void fx_spi_test_loop(void)
{
	for(;;){
		#if(FX_SPI_WORK_MODE == FX_SPI_MODE_MASTER)
		//if(ax_get_timerA_flag()){
			//ax_set_timerA_flag(0);
			if(fx_spi_master_check_status()){
				ax_spi_master_trans_data(spi_trans_data ++);
			}
		//}
		#else
		if(ax_get_timerA_flag()){
			ax_set_timerA_flag(0);
			if(spi_rcv_fin_flag){
				spi_rcv_fin_flag = 0;
				if(++ spi_buffer_index >= 4) spi_buffer_index = 0;
				spi_data_buf_cur = spi_data_buffer[spi_buffer_index];
				spi_data_counter -= FX_DATA_LEN;
				#if(FX_SPI_SLAVER_DMA_EN)
				ax_spi_slaver_set_dma_address(spi_data_buf_cur);
				#endif
				fx_spi_slaver_trans_enable();
			}
		}
		#endif
	}
}

void ax_spi_test(void)
{
	//ax_usart_send_char_message("Function :: SPI TEST");
	fx_spi_test_init();
	fx_spi_test_loop();
}

void SPI2_IRQHandler(void)
{
	//if(SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE)) {
	//	SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_RXNE);
	*spi_data_buf_cur = (char)SPI_I2S_ReceiveData(SPI2);
	//spi_data_buffer[spi_buffer_index][spi_data_counter] = (char)SPI_I2S_ReceiveData(SPI2);
	spi_data_buf_cur ++;
	if(++ spi_data_counter >= FX_DATA_LEN){
		fx_spi_slaver_trans_disable();
		spi_rcv_fin_flag = 1;
	}
	//}
}

// SPI2-RX
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4)){
		fx_spi_slaver_trans_disable();
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		// spi_rcv_counter = DMA_GetCurrDataCounter(DMA1_Channel4);
		spi_rcv_fin_flag = 1;
		spi_data_counter += FX_DATA_LEN;
	}
}

// SPI2-TX
void DMA1_Channel5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC5)){
		DMA_ClearITPendingBit(DMA1_IT_GL5);
		// spi_rcv_counter = DMA_GetCurrDataCounter(DMA1_Channel5);
		spi_tx_fin_flag = 1;
	}
}




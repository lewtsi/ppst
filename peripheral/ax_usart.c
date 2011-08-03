
#include "ax_usart.h"

#define USART_DMA_ENABLE	0
#define USART_BAUD_CNT_MAX	12

static uint8_t fx_usart_rcv_fin;
static ax_usart_rcv_buff_t fx_usart_rcv_buff[AXUSART_RX_BUFFER_CNT];
static ax_usart_rcv_buff_t *p_com_rcv_buff = &fx_usart_rcv_buff[0];
static ax_usart_rcv_buff_t *fx_rcv_pool_start;
static ax_usart_rcv_buff_t *fx_rcv_pool_end;
static ax_usart_rcv_buff_t *fx_rcv_pool_head;
static ax_usart_rcv_buff_t *fx_rcv_pool_tail;
static volatile uint8_t fx_rcv_pool_items_free;
static volatile uint8_t fx_rcv_pool_items_used;
static bool fx_rcv_pool_OV_flag;	// overflow

static char fx_hex2ascii[] = "0123456789ABCDEF";

void fx_usart_pool_init( void )
{
    fx_rcv_pool_start = fx_usart_rcv_buff;
    fx_rcv_pool_end = &fx_usart_rcv_buff[AXUSART_RX_BUFFER_CNT - 1];    
    fx_rcv_pool_head = fx_rcv_pool_start;
    fx_rcv_pool_tail = fx_rcv_pool_end;    
    fx_rcv_pool_items_free = AXUSART_RX_BUFFER_CNT;
    fx_rcv_pool_items_used = 0;    
    fx_rcv_pool_OV_flag = false;
}

void fx_usart_rcv_end_handler(void)
{    
	if (fx_rcv_pool_items_free == 0){
		fx_rcv_pool_OV_flag = true;
	}else{
		if (fx_rcv_pool_head == fx_rcv_pool_end){
			fx_rcv_pool_head = fx_rcv_pool_start;
		}else{
			++fx_rcv_pool_head;
		}
		--fx_rcv_pool_items_free;
		++fx_rcv_pool_items_used;
	}
}

ax_usart_rcv_buff_t * ax_usart_get_rcv_pool(void)
{
	if (fx_rcv_pool_tail == fx_rcv_pool_end){
		fx_rcv_pool_tail = fx_rcv_pool_start;
	}else{
		++fx_rcv_pool_tail;
	}
	++fx_rcv_pool_items_free;
	--fx_rcv_pool_items_used;

	return fx_rcv_pool_tail;
}

uint8_t ax_usart_get_rcv_OV_flag(void)
{
	return fx_rcv_pool_OV_flag;
}

uint8_t ax_usart_get_rcv_used(void)
{
	return fx_rcv_pool_items_used;
}



_fx uint8_t fx_usart_parameter_verify(st_usart_para_t *p_para)
{
	USART_InitTypeDef USART_InitStructure;
	uint8_t verify_result=0;

	// 230400/460800/921600有0.16%的误码率 @72M
	uint32_t com_baudrate[]={2400, 4800, 9600, 19200, 38400,
						 57600, 115200, 230400, 460800, 921600, 
						 2250000, 4500000};
	uint8_t com_br_2Btime[]={63,31,15,7,3,2,2,2,2,2,1,1};

	if(p_para->stopbit==1){
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
	}else if(p_para->stopbit==2){
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
	}else{
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		verify_result |= AXUSART_ERR_STOPBIT;
	}
		
	if((p_para->baudrate!=0)&&(p_para->baudrate <= USART_BAUD_CNT_MAX)){
		USART_InitStructure.USART_BaudRate=com_baudrate[p_para->cfg_uart_band-1];
		if(p_para->timeout>AXUSART_TIMEOUT_MAX_MASK){
			p_para->timeout=AXUSART_TIMEOUT_MAX_MASK;
			verify_result |= AXUSART_ERR_TIMEOUT;
		}else if(p_para->timeout < com_br_2Btime[p_para->cfg_uart_band-1]){
			p_para->timeout=(com_br_2Btime[p_para->cfg_uart_band-1]);
		}
	}else{
		USART_InitStructure.USART_BaudRate=com_baudrate[AXUSART_BR_9600_CNT-1];
		p_para->timeout =(com_br_2Btime[AXUSART_BR_9600_CNT-1]);		
		verify_result |= AXUSART_ERR_BAUDRATE;
	}
	
	// charsize 8~9
	if(p_para->datalen==9){
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	}else{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		if(p_para->datalen!=8) verify_result |= AXUSART_ERR_CHARSIZE;
	}
	
	// parity 0/1/2
	if(p_para->parity==0){
		USART_InitStructure.USART_Parity = USART_Parity_No;
	}else if(p_para->parity==1){
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}else if(p_para->parity==2){
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}else{
		USART_InitStructure.USART_Parity = USART_Parity_No;
		verify_result |= AXUSART_ERR_PARITY;
	}
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(AXUSART_PORT, &USART_InitStructure);

	return verify_result;
}

uint8_t ax_usart_init(st_usart_para_t * p_para)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ClockInitTypeDef USARTClockInitStructure;
	
	uint8_t init_result;
	uint16_t tim_init_value;

	fx_usart_rcc();
	p_com_rcv_buff->data_xor=0;
	p_com_rcv_buff->data_len=0;
	
	USART_DeInit(AXUSART_PORT);

	NVIC_InitStructure.NVIC_IRQChannel = AXUSART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 
											NVIC_PreemptionPriority_USART;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SubPriority_USART2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = AXUSART_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(AXUSART_IOPORT, &GPIO_InitStructure);

	// Configure USART Rx as input floating
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = AXUSART_RX_PIN;
	GPIO_Init(AXUSART_IOPORT, &GPIO_InitStructure);

	if((p_para->func_flag & ax_usart_flowCtrl_en) || 
			(p_para->type == AXUSART_TYPE_485)){
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = AXUSART_ASSPIN;
		GPIO_Init(AXUSART_ASSPORT, &GPIO_InitStructure);
	}

	USARTClockInitStructure.USART_Clock = USART_Clock_Disable;
	USARTClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USARTClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USARTClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(AXUSART_PORT,&USARTClockInitStructure);

	init_result=fx_usart_parameter_verify(p_para);

	USART_ITConfig(AXUSART_PORT, USART_IT_RXNE, ENABLE);
	USART_Cmd(AXUSART_PORT, ENABLE);
	
	tim_init_value = p_para->cfg_uart_timeout * TIM_INIT_1MS;
	ax_timUsart_init(tim_init_value);

	return init_result;
}

void ax_usart_send_char(uint8_t data)
{
	while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
	AXUSART_PORT->DR = data;
}

void ax_usart_send_string(uint8_t *buf, uint8_t len)
{
	while(len != 0){
		while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
		AXUSART_PORT->DR = *buf++;
		len --;
	}
}

void ax_usart_send_message(char *msg)
{
	while(*msg != '\0'){
		while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
		AXUSART_PORT->DR = *msg ++;
	}
}

// 0xA0B1C2D3 --> u8len = 4 --> "A0B1C2D3"
void ax_usart_send_hex2ascii_string(uint8_t *buf, uint8_t u8len)
{
	uint8_t *p = buf + u8len - 1;
	uint8_t data;

	while(u8len -- != 0){
		data = ((*p >> 4) & 0x0F);
		ax_usart_send_char(fx_hex2ascii[data]);
		data = *p & 0x0F;
		ax_usart_send_char(fx_hex2ascii[data]);
		p --;
	}
}


/// 等待串口数据发送完成，仅特殊情况下使用
void ax_usart_wait_for_send_fin(void)
{
	while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
}

void ax_debug_message_output(char *msg)
{
	#if(DEBUG_MSG_ON)
	ax_usart_send_message(msg);
	#endif
}


ax_usart_rcv_buff_t* ax_usart_get_received_data( void )
{
    return p_com_rcv_buff;
}


uint16_t ax_usart_received_counter_value( void )
{
	if(ax_timUsart_get_OV_flag() || fx_usart_rcv_fin){
		return p_com_rcv_buff->data_len; 
	}else{
		return 0;
	}
}

void ax_usart_reset_receiver(void)
{
	uint8_t dummy;
	
	USART_ITConfig(AXUSART_PORT, USART_IT_RXNE, DISABLE);
	p_com_rcv_buff->data_xor=0;
	p_com_rcv_buff->data_len=0;
	fx_usart_rcv_fin = false;
	ax_timUsart_set_OV_flag(false);
	USART_GetITStatus(AXUSART_PORT, USART_IT_RXNE);
	dummy = USART_ReceiveData(AXUSART_PORT);
	dummy = dummy;
	
	USART_ITConfig(AXUSART_PORT, USART_IT_RXNE, ENABLE);
}

void USART1_IRQHandler(void)
{
	uint8_t receivedData;

	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
		USART_ReceiveData(USART1);
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == RESET) return;
	receivedData = USART_ReceiveData(USART1);	//Collect data.
	//USART_ClearITPendingBit(USART1, USART_IT_RXNE);

	ax_timUsart_disable();
	ax_timUsart_counter_clear();
	ax_timUsart_set_OV_flag(false);
	
	if (p_com_rcv_buff->data_len < AXUSART_RX_MAX_BYTES){
		p_com_rcv_buff->buff[p_com_rcv_buff->data_len] = receivedData;
		p_com_rcv_buff->data_len++;
		#if(AXUSART_AUTO_XOR_ON)
		p_com_rcv_buff->data_xor^=receivedData;
		#endif
	}else{	//End of data stream.
		USART_ITConfig(AXUSART_PORT, USART_IT_RXNE, DISABLE);
		p_com_rcv_buff->buff[AXUSART_RX_MAX_BYTES] = receivedData;
		p_com_rcv_buff->data_len = AXUSART_RX_MAX_BYTES;
		fx_usart_rcv_fin = true;
	}

	ax_timUsart_enable();
}



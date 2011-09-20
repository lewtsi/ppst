#include "ax_usart.c"

st_usart_para_t fx_usart_para_alloc;
st_usart_para_t * fx_usart_para = &fx_usart_para_alloc;

void ax_usart_test(void)
{
	ax_usart_rcv_buff_t* fx_rcv_buf;

	fx_usart_para->type = AXUSART_TYPE_232;
	fx_usart_para->uport = USART1;
	fx_usart_para->baudrate = AXUSART_BR_9600_CNT;
	fx_usart_para->datalen = AXUSART_DATA_BIT8;
	fx_usart_para->parity = AXUSART_PARITY_NON;
	fx_usart_para->stopbit = AXUSART_STOP_BIT1;
	fx_usart_para->timeout = AXUSART_TIMEOUT_DEFAULT;
	fx_usart_para->func_flag = 0;
	ax_usart_init(fx_usart_para);

	while(1){
		if(ax_usart_get_rcv_used()){
			 fx_rcv_buf = ax_usart_get_rcv_pool();
			 ax_usart_send_char('@');
			 ax_usart_send_string(fx_rcv_buf->buff, fx_rcv_buf->data_len);
			 ax_usart_processing_fin();
		}
	}
}



/**
 * ���ļ���ҪӦ�Ի������ķ�����ʹ�õĲ���������
 *
 */

#include "ax_buffer.h"

#define FX_DOUBLE_BUFFER_EN	1	//<! ˫���湦��ʹ��

#define FX_BUFFER_LEN	512


#if(FX_BUFFER_LEN < 256)
typedef uint8_t buf_int_t;
#else
typedef uint16_t buf_int_t;
#endif

typedef struct{
	uint8_t *data_cur;
	uint16_t data_cnt;	// �ܹ���Ч�����ݼ�¼
	buf_int_t buf_pos;	// ��ǰbuffer��ʹ�ü���
	uint8_t 
	
}fx_buffer_t;



#if(FX_DOUBLE_BUFFER_EN)

static uint8_t db_buffer[2][FX_BUFFER_LEN];
static uint8_t db_buffer_cur;
static buf_int_t db_buffer_pos;



void ax_db_buffer_init(void)
{
	db_buffer_cur = 0;
	db_buffer_pos = 0;
}






#endif




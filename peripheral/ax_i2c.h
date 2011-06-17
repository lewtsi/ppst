#ifndef IIC_DRIVER_H
#define IIC_DRIVER_H

#include "ax_common.h"

void ax_iic_init(void);
uint8_t ax_iic_start(void);
void ax_iic_stop(void);
void ax_iic_ack(void);
void ax_iic_nack(void);
uint8_t ax_iic_chk_ack(void);
uint8_t ax_iic_write_byte(uint8_t data);
uint8_t ax_iic_read_byte(uint8_t *data);
uint8_t ax_iic_dev_write_byte(uint8_t addr, uint8_t data);
uint8_t ax_iic_dev_read_byte(uint8_t addr, uint8_t *data);


#endif

/* EOF */


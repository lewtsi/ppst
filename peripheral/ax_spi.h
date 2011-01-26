#ifndef AX_SPI_H
#define AX_SPI_H

#include "ax_common.h"

#define SPI2_DR_Address    0x4000380C

void ax_spi_master_init(void);
void ax_spi_master_dma_init(char *data_buf, uint32_t buf_size);
void ax_spi_master_ss_start(void);
void ax_spi_master_ss_end(void);
uint8_t ax_spi_master_trans_data(uint8_t data);
uint8_t ax_spi_master_trans_packet(uint8_t *buf, uint16_t len);
void ax_spi_master_dma_send_data(char *buf, uint16_t len);
void ax_spi_slaver_init(void);
void ax_spi_slaver_dma_1line_init(char *rcv_buf, uint32_t buf_size);
void ax_spi_slaver_dma_2lines_init(char *rcv_buf, uint32_t buf_size);
void ax_spi_slaver_set_dma_address(char *addr);

#endif


#ifndef _EQ_DRC_DBG_IN_UART_H
#define _EQ_DRC_DBG_IN_UART_H

#define little_endian_read_16(buf, ofs)         *(uint16_t *)((uint8_t *)buf + (ofs))
#define little_endian_read_32(buf, ofs)         *(uint32_t *)((uint8_t *)buf + (ofs))
#define HUART_BUFFER_LEN                        (260+10)
#define EQ_BUFFER_LEN                           (260+10)

typedef struct {
    u32 *addr;
    u32 *len;
} eq_param;

typedef struct {
    u8 rx_type;
}eq_dbg_cb_t;

extern eq_dbg_cb_t eq_dbg_cb;
extern u8 eq_rx_buf[EQ_BUFFER_LEN];

void eq_drc_dbg_init(void);
void tx_ack(uint8_t *packet, uint16_t len);
#endif


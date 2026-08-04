#ifndef _WIRELESS_TXRX_H
#define _WIRELESS_TXRX_H

void wireless_con_adapter_init(void);
void wireless_con_device_init(void);
u8 wireless_d2a_get_rx_frame(u8 idx, u8 *buf, uint size);
void wireless_d2a_put_tx_frame(u8 *ptr, u16 size);

#endif

#ifndef _BSP_BSP_H
#define _BSP_DUT_H

#define PACKET_NO_ERR            0
#define PACKET_BUF_FULL          1
#define PACKET_RECV_ERR          2

#define PACKET_MIN               4

#define RX_ELEMENT_NUM           4
#define RX_ELEMENT_NUM_MASK      (RX_ELEMENT_NUM-1)
#define RX_BUF_SIZE              20


void le_hci_cmd_init(void);
void le_hci_cmd_exit(void);

#endif

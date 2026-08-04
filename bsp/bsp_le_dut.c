#include "include.h"
#include "driver_com.h"
#include "driver_gpio.h"
#include "bsp.h"

///BQB_RF和FCC TEST通用的接口
#if LE_DUT_UART_EN

typedef struct {
    uint8_t buf[RX_ELEMENT_NUM][RX_BUF_SIZE];
    uint8_t len[RX_ELEMENT_NUM];
    uint8_t w_idx;
    uint8_t r_idx;
    uint8_t total_size;
    uint8_t status;
} hci_pkt_cb;

AT(.buf.le_dut.hci_cmd)
static hci_pkt_cb hci_cmd_pkt;

ALIGNED(4)
static u8 hci_rx_buffer[128] AT(.buf.le_dut.hci_buf);


extern u32 __comm_test_vma, __comm_test_lma, __comm_test_size;

///初步处理接收到的数据,并保存到pkt_buf,最后通过msg_enqueue的方式去执行命令
AT(.text.le_dut.hci_proc)
void le_hci_cmd_process(u8 *data, u16 len)
{
    if (len >= RX_BUF_SIZE || len < PACKET_MIN){
        hci_cmd_pkt.status   = PACKET_RECV_ERR ;

    } else if((hci_cmd_pkt.total_size >= RX_ELEMENT_NUM)){
        hci_cmd_pkt.status = PACKET_BUF_FULL;

    } else {
        memcpy(hci_cmd_pkt.buf[(hci_cmd_pkt.w_idx)&RX_ELEMENT_NUM_MASK], data, len);
        hci_cmd_pkt.len[(hci_cmd_pkt.w_idx)&RX_ELEMENT_NUM_MASK] = len;
        hci_cmd_pkt.status = PACKET_NO_ERR;
        (hci_cmd_pkt.w_idx)++;
        (hci_cmd_pkt.total_size)++;
    }

    msg_enqueue(EVT_HCI_CMD);
}

///将pkt_buf地址和tx函数给到蓝牙基带
AT(.text.le_init.dut)
void le_hci_cmd_init(void)
{
    bsp_hsuart_cfg_t bsp_hsuart_cfg;

    memcpy(&__comm_test_vma, &__comm_test_lma, (u32)&__comm_test_size);

    memset(&hci_cmd_pkt, 0x00, sizeof(hci_pkt_cb));
    ble_dut_pkt_buf_set(&hci_cmd_pkt);
    ble_hci_tx_callback_set(bsp_huart_tx);

    bsp_hsuart_cfg.tx_gpio_pin = GPIO_PA7;
    bsp_hsuart_cfg.rx_gpio_pin = GPIO_PA7;
    bsp_hsuart_cfg.baud = 9600;
    bsp_hsuart_cfg.rx_isr_en = 1;
    bsp_hsuart_cfg.rx_buf_ptr = hci_rx_buffer;
    bsp_hsuart_cfg.rx_buf_len = 128;
    bsp_hsuart_cfg.isr = le_hci_cmd_process;
    bsp_hsuart_cfg.pu_pd_en = 1;

    bsp_huart_init(&bsp_hsuart_cfg);
}

AT(.text.le_exit.dut)
void le_hci_cmd_exit(void)
{
}
#endif

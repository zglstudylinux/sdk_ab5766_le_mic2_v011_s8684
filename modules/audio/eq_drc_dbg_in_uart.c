#include "include.h"
#include "eq_drc_dbg_in_uart.h"

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

u8 eq_rx_buf[EQ_BUFFER_LEN];
#if EQ_DRC_DBG_IN_UART
eq_dbg_cb_t eq_dbg_cb;

void tx_ack(uint8_t *packet, uint16_t len)
{
    delay_5ms(1);   //延时一段时间再ack
    if (eq_dbg_cb.rx_type) {
        bsp_huart_tx(packet, len);
    } else {
    }
    eq_dbg_cb.rx_type = 0;
}

AT(.com_text.eq_drc_dbg)
void eq_drc_dbg_cmd_process(u8 *data, u16 len)
{
    if((data[0]=='C')&&(data[1]=='F')&&(data[2]=='G')){
        msg_enqueue(EVT_ONLINE_SET_EFFECT);
        eq_dbg_cb.rx_type = 1;
    }
}

void eq_drc_dbg_init(void)
{
    memset(&eq_dbg_cb, 0, sizeof(eq_dbg_cb_t));

    bsp_hsuart_cfg_t bsp_hsuart_cfg;
    bsp_hsuart_cfg.tx_gpio_pin = xcfg_cb.huart_io_sel;
    bsp_hsuart_cfg.rx_gpio_pin = xcfg_cb.huart_io_sel;
    bsp_hsuart_cfg.baud = 1500000;
    bsp_hsuart_cfg.rx_isr_en = 1;
    bsp_hsuart_cfg.rx_buf_ptr = eq_rx_buf;
    bsp_hsuart_cfg.rx_buf_len = EQ_BUFFER_LEN;
    bsp_hsuart_cfg.isr = eq_drc_dbg_cmd_process;
    bsp_hsuart_cfg.pu_pd_en = 1;

    bsp_huart_init(&bsp_hsuart_cfg);
}
#endif




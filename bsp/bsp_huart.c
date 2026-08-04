#include "include.h"
#include "bsp_huart.h"
#include "api.h"
#include "driver_com.h"
#include "driver_hsuart.h"
#include "driver_gpio.h"
#include "config.h"

#if LE_DUT_UART_EN || EQ_DRC_DBG_IN_UART

static bsp_hsuart_str_t bsp_hsuart_str;


//AT(.com_text.huart.txdone)
//static void huart_wait_txdone(void)
//{
//    while (hsuart_get_flag(HSUART_IT_TX) == RESET);
//}

///设置HUART波特率
AT(.text.huart.set_baudrate)
void huart_set_baudrate(uint baud_rate)
{
    hsuart_baud_config(HSUART_REG, baud_rate);
}

///HUART 发送1Byte数据
AT(.com_text.huart.putchar)
void huart_putchar(const char ch)
{
    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET);
    hsuart_send_data(HSUART_REG, (uint16_t)ch);

}

///HUART 获取RXFIFO中的数据个数
AT(.com_text.huart.getcnt)
uint huart_get_rxcnt(void)
{
    return hsuart_get_fifo_counter(HSUART_REG);
}

///HUART 获取1Byte数据
AT(.com_text.huart.getchar)
char huart_getchar(void)
{
    char data = 0;

    hsuart_receive_data_from_fifo(HSUART_REG, (uint8_t *)&data, 1);

    return data;
}

/////HUART 发送数据
//AT(.com_text.huart.text)
//void huart_tx(const void *buf, uint len)
//{
//    huart_wait_txdone();
//
//    if ((u32)buf >= 0x10000000) {
//        const char *txbuf = buf;
//        HSUART->con &= ~BIT(7);        //buf mode
//        while (len--) {
//            huart_putchar(*txbuf++);
//        }
//    } else {
//        HSUART->con |= BIT(7);         //dma mode
//        hsuart_dma_start(HSUT_TRANSMIT, DMA_ADR(buf), len);
//    }
//}
//
//void huart_exit(void)
//{
//}

AT(.com_text.huart.rx_isr)
static void huart_rx_isr(void)
{
    if (bsp_hsuart_str.rx_en && (hsuart_get_flag(HSUART_REG, HSUART_IT_RX) != RESET || hsuart_get_flag(HSUART_REG, HSUART_IT_RX_TMR_OV) != RESET)) {
        if (bsp_hsuart_str.rx_isr_cb) {
            bsp_hsuart_str.rx_isr_cb(bsp_hsuart_str.rxbuf, hsuart_get_fifo_counter(HSUART_REG));
        }
        hsuart_clear_flag(HSUART_REG, HSUART_IT_RX | HSUART_IT_RX_TMR_OV);
        hsuart_dma_start(HSUART_REG, HSUT_RECEIVE, DMA_ADR(bsp_hsuart_str.rxbuf), bsp_hsuart_str.rxbuf_len);
    }

}

AT(.com_text.huart.txdone)
void bsp_huart_wait_txdone(void)
{
    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET);
}

///HUART 发送数据
AT(.com_text.huart.tx)
void bsp_huart_tx(const void *buf, uint len)
{
    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET);
    bsp_hsuart_str.tx_busy_flag = 1;
    hsuart_dma_start(HSUART_REG, HSUT_TRANSMIT, DMA_ADR(buf), len);
}

void bsp_huart_init(bsp_hsuart_cfg_t *bsp_hsuart_cfg)
{
    gpio_init_typedef gpio_init_structure;
    hsuart_init_typedef hsuart_init_struct;

    memset((u8 *)&bsp_hsuart_str, 0, sizeof(bsp_hsuart_str_t));
    clk_gate0_cmd(CLK_GATE0_HSUART, CLK_EN);

    bsp_hsuart_str.tx_gpio_pin = bsp_hsuart_cfg->tx_gpio_pin;
    bsp_hsuart_str.rx_gpio_pin = bsp_hsuart_cfg->rx_gpio_pin;
    if (bsp_hsuart_cfg->rx_isr_en) {
        bsp_hsuart_str.rxbuf = bsp_hsuart_cfg->rx_buf_ptr;
        bsp_hsuart_str.rxbuf_len = bsp_hsuart_cfg->rx_buf_len;
        bsp_hsuart_str.rx_en = bsp_hsuart_cfg->rx_isr_en;
        bsp_hsuart_str.rx_isr_cb = bsp_hsuart_cfg->isr;
    }
    if (bsp_hsuart_str.tx_gpio_pin == bsp_hsuart_str.rx_gpio_pin) {
        gpio_init_structure.gpio_pin = GPIO_PIN_GET(bsp_hsuart_cfg->tx_gpio_pin);
        gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
        gpio_init_structure.gpio_fen = GPIO_FEN_PER;
        gpio_init_structure.gpio_fdir = GPIO_FDIR_MAP;
        gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
        gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
        if (!bsp_hsuart_cfg->pu_pd_en) {
            gpio_init_structure.gpio_pupd = GPIO_PUPD_NULL;
        }
        gpio_init(GPIO_PORT_GET(bsp_hsuart_cfg->tx_gpio_pin), &gpio_init_structure);

        gpio_func_mapping_config(GPIO_PORT_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_PIN_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_CROSSBAR_IN_HURRX);
        gpio_func_mapping_config(GPIO_PORT_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_PIN_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_CROSSBAR_OUT_HURTX);
    } else {
        gpio_init_structure.gpio_pin = GPIO_PIN_GET(bsp_hsuart_cfg->rx_gpio_pin);              //PA0 HUART  RX
        gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
        gpio_init_structure.gpio_fen = GPIO_FEN_PER;
        gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
        gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
        gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
        gpio_init(GPIO_PORT_GET(bsp_hsuart_cfg->rx_gpio_pin), &gpio_init_structure);

        gpio_init_structure.gpio_pin = GPIO_PIN_GET(bsp_hsuart_cfg->tx_gpio_pin);            //PA1  HUART TX
        gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
        gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
        gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
        gpio_init(GPIO_PORT_GET(bsp_hsuart_cfg->tx_gpio_pin), &gpio_init_structure);

        gpio_func_mapping_config(GPIO_PORT_GET(bsp_hsuart_cfg->rx_gpio_pin), GPIO_PIN_GET(bsp_hsuart_cfg->rx_gpio_pin), GPIO_CROSSBAR_IN_HURRX);
        gpio_func_mapping_config(GPIO_PORT_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_PIN_GET(bsp_hsuart_cfg->tx_gpio_pin), GPIO_CROSSBAR_OUT_HURTX);
    }

    hsuart_init_struct.baud = bsp_hsuart_cfg->baud;
    hsuart_init_struct.tx_mode = HSUT_TX_DMA_MODE;
    hsuart_init_struct.tx_stop_bit = HSUT_STOP_BIT_2BIT;
    hsuart_init_struct.tx_word_len = HSUT_TX_LENGTH_8b;

    if (bsp_hsuart_str.tx_gpio_pin == bsp_hsuart_str.rx_gpio_pin) {
        hsuart_init_struct.one_line_en = HSUT_ONELINE_EN;
    } else {
        hsuart_init_struct.one_line_en = HSUT_ONELINE_DIS;
    }
    hsuart_init_struct.rx_mode = HSUT_RX_DMA_MODE;
    hsuart_init_struct.rx_word_len = HSUT_RX_LENGTH_8b;
    hsuart_init_struct.rx_dma_buf_mode = HSUT_DMA_BUF_SINGLE;
    hsuart_init(HSUART_REG, &hsuart_init_struct);

    if (bsp_hsuart_cfg->rx_isr_en) {
        hsuart_clear_flag(HSUART_REG, HSUART_IT_RX | HSUART_IT_RX_TMR_OV);
        hsuart_rx_idle_config(HSUART_REG, HSUART_OV_CNT, ENABLE);
        hsuart_pic_config(HSUART_REG, huart_rx_isr, 0, HSUART_IT_RX, ENABLE);
        hsuart_cmd(HSUART_REG, HSUT_TRANSMIT | HSUT_RECEIVE, ENABLE);
        hsuart_dma_start(HSUART_REG, HSUT_RECEIVE, DMA_ADR(bsp_hsuart_str.rxbuf), bsp_hsuart_str.rxbuf_len);
    } else {
        hsuart_cmd(HSUART_REG, HSUT_TRANSMIT, ENABLE);
    }
}

///HUART 停止函数
AT(.text.bsp.wireless_mic)
void bsp_huart_stop(u8 rx_gpio_pin)
{
    hsuart_deinit(HSUART_REG);
}
#endif

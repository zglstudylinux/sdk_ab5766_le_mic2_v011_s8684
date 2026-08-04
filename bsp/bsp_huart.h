#ifndef _BSP_HUART_H
#define _BSP_HUART_H

#define HSUART_OV_CNT       0x20

typedef void (*huart_rx_isr_t)(u8 *rx_buf, u16 rx_buf_len);

typedef struct {
    u8 tx_gpio_pin;
    u8 rx_gpio_pin;
    u32 baud;
    u8 rx_isr_en;
    u8 *rx_buf_ptr;
    u16 rx_buf_len;
    huart_rx_isr_t isr;
    u8 pu_pd_en;
} bsp_hsuart_cfg_t;

typedef struct {
    u8 tx_gpio_pin;
    u8 rx_gpio_pin;
    u8 *rxbuf;
    u16 rxbuf_len;
    u8 rx_en;                       //rx enable
    volatile u8 tx_busy_flag;       //tx busy flag
    huart_rx_isr_t rx_isr_cb;       //rx_isr callback
} bsp_hsuart_str_t;

void huart_exit(void);
void huart_set_baudrate(uint baudrate);
void huart_putchar(const char ch);
void huart_tx(const void *buf, uint len);
uint huart_get_rxcnt(void);
char huart_getchar(void);

void bsp_huart_init(bsp_hsuart_cfg_t *bsp_hsuart_cfg);
void bsp_huart_tx(const void *buf, uint len);
void bsp_huart_wait_txdone(void);
void bsp_huart_stop(u8 rx_gpio_pin);

#endif

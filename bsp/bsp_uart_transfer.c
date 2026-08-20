#include "include.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "bsp_uart_transfer.h"

uart_transfer_cb_typedef uart_rx = {
    .done = 0,
    .len = 0,
    .timeout = 50, //ms
};

AT(.com_text.isr)
void uart_isr(void)
{
    if (uart_get_flag(UART1_REG, UART_IT_RX) != RESET) {
        uart_rx.tick = tick_get();
        if (uart_rx.len < sizeof(uart_rx.buf)) {
            uart_rx.buf[uart_rx.len++] = uart_receive_data(UART1_REG);
        }

        uart_clear_flag(UART1_REG, UART_IT_RX);
    }
}

void uart_transfer_init(u32 baud)
{
    uart_init_typedef uart_init_struct;
    gpio_init_typedef gpio_init_structure;

    //复位接收软件状态，避免睡眠前残留半包在唤醒后与新数据拼接
    uart_rx.len = 0;
    uart_rx.tick = 0;
    uart_rx.done = 0;

    clk_gate0_cmd(CLK_GATE0_UART1, CLK_EN);
    clk_uart_clk_set(UART1_REG, CLK_UART_XOSC24M);

    // RX
    gpio_init_structure.gpio_pin = UART_RX_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
    gpio_init(UART_RX_PORT, &gpio_init_structure);

    // TX
    gpio_init_structure.gpio_pin = UART_TX_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(UART_TX_PORT, &gpio_init_structure);

    gpio_func_mapping_config(UART_RX_PORT, UART_RX_PIN, GPIO_CROSSBAR_IN_UART1RX);
    gpio_func_mapping_config(UART_TX_PORT, UART_TX_PIN, GPIO_CROSSBAR_OUT_UART1TX);

    uart_init_struct.baud = baud;
    uart_init_struct.mode = UART_SEND_RECV_MODE;
    uart_init_struct.word_len = UART_WORD_LENGTH_8b;
    uart_init_struct.stop_bits = UART_STOP_BIT_1BIT;
    uart_init_struct.one_line_enable = UART_SEPARATE;
    uart_init_struct.baud_fix = UART_BAUD_FIX;
    uart_init(UART1_REG, &uart_init_struct);

    uart_pic_config(UART1_REG, uart_isr, 0, UART_IT_RX, ENABLE);

    uart_cmd(UART1_REG, ENABLE);
}

void uart_transfer_example(void)
{
    u8 *p = uart_rx.buf;
    if (((tick_get() - uart_rx.tick) >= uart_rx.timeout) && uart_rx.len) {
        printf("-->send\n");
        //用快照长度发送,避免 ISR 在发送期间并发改 len 导致永远发不完
        u16 left = uart_rx.len;
        bool tx_ok = true;
        while (left && tx_ok) {
            uart_send_data(UART1_REG, *p++);
            //等 TX 完成,带超时保护,避免 UART1 异常时阻塞喂不上看门狗
            u32 wait = 0;
            while (uart_get_flag(UART1_REG, UART_IT_TX) != SET) {
                if (++wait > 100000) {
                    tx_ok = false;
                    break;
                }
            }
            left--;
        }
        uart_rx.len = 0;
    }
}

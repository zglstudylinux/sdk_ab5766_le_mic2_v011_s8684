#ifndef _UART_TRANSFER_H
#define _UART_TRANSFER_H

#define UART_TX_PORT                GPIOA_REG
#define UART_TX_PIN                 GPIO_PIN_0
#define UART_RX_PORT                GPIOA_REG
#define UART_RX_PIN                 GPIO_PIN_1

typedef struct {
    u8 done;
    volatile u16 len;
    u32 timeout;
    u32 tick;
    u8 buf[32];
} uart_transfer_cb_typedef;


void uart_transfer_init(u32 baud);
void uart_transfer_example(void);

#endif // _UART_TRANSFER_H

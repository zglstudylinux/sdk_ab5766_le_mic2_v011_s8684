#ifndef _UART_TEST_HW_H
#define _UART_TEST_HW_H

#include "include.h"

//双板串口极限测试硬件后端
//普通串口 UART1 与高速串口 HSUT0 分时复用同一对引脚: PA0=TX, PA1=RX
//双板交叉接线: A.PA0 -> B.PA1, B.PA0 -> A.PA1, GND 共地

typedef enum {
    UART_HW_UART1 = 0,          //普通串口 UART1
    UART_HW_HSUART = 1,         //高速串口 HSUT0
} uart_hw_type_t;

bool uart_hw_init(uart_hw_type_t type, u32 baud);
bool uart_hw_set_baud(uart_hw_type_t type, u32 baud);
void uart_hw_deinit(uart_hw_type_t type);

bool uart_hw_putbyte(uart_hw_type_t type, u8 byte);         //阻塞发送单字节(带超时保护)
bool uart_hw_hsuart_tx_block(u8 *buf, u16 len);             //HSUART DMA 发送一块并等待完成
u16 uart_hw_read(u8 *buf, u16 len);                         //读取当前后端 RX 环形缓冲(非阻塞)
u32 uart_hw_get_drop_cnt(void);                             //RX 环形缓冲溢出丢弃计数(诊断用)

#endif // _UART_TEST_HW_H

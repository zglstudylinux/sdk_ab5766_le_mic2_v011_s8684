#ifndef _UART_CH340_TEST_H
#define _UART_CH340_TEST_H

#include "include.h"

//单板 CH340 普通串口极限测试(test/uart_ch340_test.c):
//UART1(PA0=TX, PA1=RX) 接 PC 端 CH340 USB 转串口, PC 脚本(ch340_uart_test.ps1)做考官:
//按波特率阶梯发 55AA 流(板子收流校验)再收 55AA 流(板子发流), 两者各自统计。
//板侧结果走 PB3 调试口打印。配合宏: UART_CH340_TEST_EN。
void uart_ch340_test(void);

#endif

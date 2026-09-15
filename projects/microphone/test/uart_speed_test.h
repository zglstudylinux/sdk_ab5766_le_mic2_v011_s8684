#ifndef _UART_SPEED_TEST_H
#define _UART_SPEED_TEST_H

#include "include.h"

//双板串口极限测试: 普通串口 UART1 + 高速串口 HSUT0, PA0/PA1 分时复用
//角色: 适配器=主机(发起测试), 话筒=从机(应答+统计), 单固件烧两块板
//结果从调试口输出(UART0, 默认 PB3, 1.5Mbps)
//使能开关 UART_SPEED_TEST_EN 见 config_ab5766_le_mic.h

void func_uart_test(void);

#endif // _UART_SPEED_TEST_H

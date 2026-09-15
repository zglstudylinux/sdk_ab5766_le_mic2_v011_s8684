#ifndef _UART_LOOPBACK_TEST_H
#define _UART_LOOPBACK_TEST_H

#include "include.h"

//单板串口回环测试: 用跳线短接本板 PA0(TX)-PA1(RX), 自发自收验证收发通路
//分别测 UART1 和 HSUART 两个后端的各档波特率
//返回 true 表示基础波特率(双板测试要用到的档位)全部无误码, 可以进入双板测试;
//高档位(如 XOSC48M/PLL 时钟源档)失败只打印报告, 不阻断双板测试(双板爬坡会再次体现)

bool uart_loopback_test_all(void);

#endif // _UART_LOOPBACK_TEST_H

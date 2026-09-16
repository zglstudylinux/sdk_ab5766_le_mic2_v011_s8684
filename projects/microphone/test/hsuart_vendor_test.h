#ifndef _HSUART_VENDOR_TEST_H
#define _HSUART_VENDOR_TEST_H

#include "include.h"

//原厂方法 HSUART 对照测试(test/hsuart_vendor_test.c):
//初始化/接收/数据块发送/小帧发送全部照抄原厂 bsp_huart.c 的调用序列, 不做本地改动,
//用于逐项验证旧测试(uart_test_hw.c)偏离原厂的做法是否必要。
//与旧测试通过宏互斥: UART_HSUART_VENDOR_TEST_EN=1 时 func_run 进入本测试,
//置 0 则恢复旧测试(FUNC_UART_TEST)。旧测试文件一律未改动。
void hsvendor_uart_test(void);

#endif

#ifndef _FUNC_H
#define _FUNC_H

#include "func_le_dut.h"
#include "func_mic_emit.h"
#include "func_adapter.h"
#include "func_lowpwr.h"


//task number
enum {
    FUNC_NULL = 0,
    FUNC_BT,
    FUNC_ADAPTER,
    FUNC_MIC_EMIT,
    FUNC_LE_DUT,
    FUNC_UART_TEST,                                 //双板串口极限测试(test/uart_speed_test.c)
    FUNC_HSUART_VENDOR_TEST,                        //原厂方法 HSUART 对照测试(test/hsuart_vendor_test.c)
    FUNC_UART_CH340_TEST,                           //单板 CH340 普通串口测试(test/uart_ch340_test.c)
    FUNC_PWROFF,
    FUNC_IDLE,
};

//task control block
typedef struct {
    u8 sta;                                         //cur working task number
    u8 last;                                        //lask task number
} func_cb_t;
extern func_cb_t func_cb;

extern const u8 adapter_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 mic_emit_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];

void func_process(void);
void func_message(u16 msg);

void func_run(void);
void func_idle(void);
void func_bt(void);
void func_le_fcc(void);
void func_le_bqb_rf(void);
void func_tbox(void);
void func_uart_test(void);                          //test/uart_speed_test.c
void hsvendor_uart_test(void);                      //test/hsuart_vendor_test.c
void uart_ch340_test(void);                         //test/uart_ch340_test.c

#endif

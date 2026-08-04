#include "include.h"
#include "func.h"

#if FUNC_LE_DUT_EN

AT(.text.func.le_dut)
void func_le_dut_sub_process(void)
{
    // le_fcc_app_proc();
}

AT(.text.func.le_dut)
void func_le_dut_process(void)
{
    func_process();
    func_le_dut_sub_process();
}

AT(.text.func.le_dut)
void func_le_dut_init(void)
{
    bt_setup();
}

AT(.text.func.le_dut)
void func_le_dut_enter(void)
{
    msg_queue_clear();
    lowpwr_pwroff_auto_dis();
    le_hci_cmd_init();
    func_le_dut_init();
}

AT(.text.func.le_dut)
void func_le_dut_exit(void)
{
    lowpwr_pwroff_auto_en();
    bt_off();
    func_cb.last = FUNC_LE_DUT;
}

AT(.text.func.le_dut)
void func_le_dut(void)
{
    printf("%s\n", __func__);

    func_le_dut_enter();

    while (func_cb.sta == FUNC_LE_DUT) {
        func_le_dut_process();
        func_le_dut_message(msg_dequeue());
    }

    func_le_dut_exit();
}

#endif

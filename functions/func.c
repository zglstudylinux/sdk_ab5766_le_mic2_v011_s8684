#include "include.h"
#include "dec.h"

func_cb_t func_cb AT(.buf.func_cb);

static const struct {
    const void *msg_tbl;
    u8 dev_online;      //0=不检查，!0=检查对应bit的dev_online
} func_info[] = {
    [FUNC_BT]       = {NULL,    0},
#if !ADAPTER_CLOSE_EN
    [FUNC_ADAPTER]  = {adapter_key_msg_tbl,     0},
#endif
#if !WIRELESS_MIC_CLOSE_EN
    [FUNC_MIC_EMIT] = {mic_emit_key_msg_tbl,      0},
#endif
    [FUNC_LE_DUT]   = {mic_emit_key_msg_tbl,      0},
    [FUNC_PWROFF]   = {NULL, 0},
    [FUNC_IDLE]     = {NULL, 0},
};

AT(.text.func.process)
void pwroff_process(void)
{
    struct pwroff_tag *pwroff = &sys_cb.pwroff;

    if(pwroff->key_state == PWROFF_W4_TIMEOUT) {
        if(tick_check_expire(pwroff->key_ticks, pwroff->delay_ticks)) {
            pwroff->key_state = PWROFF_END;
            pwroff->pwr_key_ind = 1;
        }
    }

    if(pwroff->all_flag != 0) {
        func_cb.sta = FUNC_PWROFF;
    }

    if(sys_cb.pwroff_delay == 0) {
        func_cb.sta = FUNC_PWROFF;
    }
}

AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();

    bt_run_loop();

#if BSP_UART_TRANSFER_EN
    uart_transfer_example();
#endif

#if BSP_CHARGE_EN
    if (xcfg_cb.charge_working_while_charging) {
        bsp_charge_process();
    } else {
        bool charge_in = (charge_detect_dc() == CHARGE_DC_STATUS_ONLINE) ? true : false;
#if BSP_CHARGE_BOX_EN
        if(sys_cb.charge_box_type != CHARGE_BOX_DISABLE) {
            //充电仓低电量的时候, VUSB不会升到5V, 用inbox检测入仓关机
            charge_in = CHARGE_INBOX()? true : false;
        }
#endif
        if (charge_in) {
            WDT_RST();
        }
    }
#endif

    pwroff_process();
}

//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    bool event_valid = true;

    switch (msg) {
        case MSG_PWR_HOLD:
            if(sys_cb.pwroff.key_state == PWROFF_IDLE) {
                sys_cb.pwroff.key_ticks = tick_get();
                sys_cb.pwroff.key_state = PWROFF_W4_TIMEOUT;
            }
            break;
        case MSG_PWR_RELEASE:
            if(sys_cb.pwroff.key_state == PWROFF_W4_TIMEOUT) {
                sys_cb.pwroff.key_state = PWROFF_IDLE;
            }
            break;
        case MSG_PWR_OFF:
            sys_cb.pwroff.pwr_key_ind = 1;
            break;

#if LE_DUT_UART_EN
        case EVT_HCI_CMD:
            ble_hci_cmd_check();
            break;
#endif

#if EQ_DRC_DBG_IN_UART
        case EVT_ONLINE_SET_EFFECT:
            toolkit_process();
            break;
#endif
        default:
            event_valid = false;
            break;
    }

    if (event_valid) {
        lowpwr_pwroff_delay_reset();
        lowpwr_sleep_delay_reset();
    }
}

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
    bsp_param_sync();

    lowpwr_sleep_delay_reset();

    lowpwr_pwroff_delay_reset();

    key_set_msg_tbl(func_info[func_cb.sta].msg_tbl);
}

AT(.text.func)
void func_exit(void)
{
    func_cb.sta = FUNC_IDLE;
}

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);

    if (wireless_role_is_adapter()) {
        func_cb.sta = FUNC_ADAPTER;
    } else {
        func_cb.sta = FUNC_MIC_EMIT;
    }

#if FUNC_LE_DUT_EN
    func_cb.sta = FUNC_LE_DUT;
#endif

    while (1) {
        func_enter();
        switch (func_cb.sta) {
#if !WIRELESS_MIC_CLOSE_EN
        case FUNC_MIC_EMIT:
            func_mic_emit();
            break;
#endif
#if !ADAPTER_CLOSE_EN
        case FUNC_ADAPTER:
            func_adapter();
            break;
#endif


#if FUNC_LE_DUT_EN
        case FUNC_LE_DUT:
            func_le_dut();
            break;
#endif

        case FUNC_PWROFF:
            func_pwroff();
            break;

        default:
            func_exit();
            break;
        }
    }
}

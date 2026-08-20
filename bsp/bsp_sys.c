#include "include.h"
#include "driver_lowpwr.h"
#include "bsp_saradc_key.h"
#include "bsp_sddac.h"


xcfg_cb_t xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);
u16 tmr5ms_cnt;
u16 tmr1ms_cnt = 0;;
static void seg_info_dump(const char *name, u32 start, u32 size)
{
    u32 end = start + size - 1;
    printf("%s: range[0x%x-0x%x], size=%dk\n", name, start, end, size/1024);
}

void code_info_dump(void)
{
    extern u32 __heap_start, __heap_size;
    extern u32 __comm_vma, __comm_size;
    extern u32 __bss_start, __bss_size;
    extern u32 __bram_start, __bram_size;

    printf("----------------------------------------------------\n");
    seg_info_dump("comm", (u32)&__comm_vma,     (u32)&__comm_size);
    seg_info_dump("heap", (u32)&__heap_start,   (u32)&__heap_size);
    seg_info_dump("bss ", (u32)&__bss_start,    (u32)&__bss_size);
    seg_info_dump("bram", (u32)&__bram_start,   (u32)&__bram_size);
    printf("----------------------------------------------------\n");
}

///切flash回调函数，可以用于统计切flash次数，检测切flash频率等
//AT(.com_rodata.miss)
//static const char cache_miss_str[] = "cache_miss_cnt: %d\n";
//AT(.com_rodata.miss)
//static const char miss_addr_str[] = "miss_addr(0x10000000+): %x\n";
AT(.com_text.timer)
void debug_dump_info_cache_miss_callback(u32 miss_addr)
{
//    sys_cb.cache_miss_cnt++;
//    if (tick_check_expire(sys_cb.cache_miss_ticks, 1000)) {
//        printk(cache_miss_str, sys_cb.cache_miss_cnt);
//        sys_cb.cache_miss_cnt = 0;
//        sys_cb.cache_miss_ticks = tick_get();
//    }
}
AT(.com_text.timer)
void usr_tmr1ms_isr_callback(void)
{
#if BSP_KEY_EN && WIRELESS_MIC_KEY_PRESS_MUTE_EN
    ///定时器使能在按键之后，可以直接调用
    tmr1ms_cnt++;
    if ((tmr1ms_cnt%2) == 0) {
        bsp_key_scan(0);
    }
#endif

}
//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_thread_do(void)
{
    tmr5ms_cnt++;
#if BSP_DLED_EN
    //10ms timer process
    if ((tmr5ms_cnt % 2) == 0) {
        dled_scan_10ms();
    }
#endif

#if (BSP_LED_EN && !BSP_DLED_EN)
    //50ms timer process
    if ((tmr5ms_cnt % 10) == 0) {
        u32 tick = tick_get();
        led_scan(tick);
    }
#endif

#if BSP_KEY_EN && !WIRELESS_MIC_KEY_PRESS_MUTE_EN
    bsp_key_scan(0);
#endif

    //100ms timer process
    if ((tmr5ms_cnt % 20) == 0) {
        lowpwr_tout_ticks();
    }

    //500ms timer process
    if ((tmr5ms_cnt % 100) == 0) {
        msg_enqueue(MSG_SYS_500MS);
    }

    //1s timer process
    if ((tmr5ms_cnt % 200) == 0) {
        msg_enqueue(MSG_SYS_1S);
    }
}

static uint8_t cfg_pmu_get(void)
{
    uint8_t pmu_cfg;
    pmu_cfg = (BUCK_MODE_EN * PMU_CFG_BUCK_EN) | PMU_CFG_VBAT;

    if (!BUCK_MODE_EN) {
        pmu_cfg |= (PMU_CFG_VDDBT_CAPLESS * xcfg_cb.vddbt_capless_en);
    }

#if !BSP_CHARGE_EN
    pmu_cfg |= PMU_CFG_CHARGE_DIS;
#endif

    return pmu_cfg;
}

AT(.text.bsp.sys.init)
static void bsp_var_init(void)
{
    sys_cb.pwroff_time = -1L;
    sys_cb.pwroff_delay = -1L;

#if !FUNC_LE_DUT_EN
    if (xcfg_cb.sys_off_time != 0) {
        sys_cb.pwroff_time = (u32)xcfg_cb.sys_off_time * 10;    //100ms为单位 * 10;    //100ms为单位
        sys_cb.pwroff_delay = sys_cb.pwroff_time;
    }
#endif

    sys_cb.pwroff.delay_ticks = xcfg_cb.pwroff_press_time * 500 + 1500 - (KEY_LONG_TIMES*5);
#if BSP_CHARGE_BOX_EN
    sys_cb.charge_box_type = xcfg_cb.charge_en? xcfg_cb.ch_box_type: CHARGE_BOX_DISABLE;
#endif

    if(!xcfg_cb.ft_osc_cap_en || (xcfg_cb.osci_cap == 0 && xcfg_cb.osco_cap == 0)) {        //没有过产测时，使用自定义OSC电容
        xcfg_cb.osci_cap = xcfg_cb.uosci_cap;
        xcfg_cb.osco_cap = xcfg_cb.uosco_cap;
        xcfg_cb.osc_both_cap = xcfg_cb.uosc_both_cap;
    }

#if ADAPTER_USB_MIC_RX_EN
    if (xcfg_cb.wireless_adapter_en) {
//        RSTCON0 |= BIT(0);
//        CLKCON0 |= BIT(15);
//        CLKCON4 &= ~BIT(25);
//        CLKCON1 &= ~BIT(10);             //USB
        dev_init(1);
    }
#endif

    msg_queue_init();
}

void power_on_check(void)
{
    u32 power_on_check_time = PWRON_PRESS_TIME;
    u32 wakeup_reason = WK_LP_WK0_PENDING | WK_LP_VUSB_PENDING;

    if (sys_cb.wakeup_reason & WK_LP_INBOX_PENDING)  {
        //1、出仓唤醒, 判断是否要开机
        //2、充满电关机后, 拔出VUSB, 判断是否要开机
        if(xcfg_cb.poweron_vusb_out_en) {
            return;
        }
    } else if ((sys_cb.wakeup_reason & WK_LP_VUSB_PENDING) && xcfg_cb.poweron_vusb_out_en){                          //VUSB退出是否自动开机
        return;
    } else if ((!(sys_cb.wakeup_reason & wakeup_reason) && xcfg_cb.poweron_first_bat_en) || power_on_check_time == 0) {
        return;
    }

#if BSP_KEY_EN

    u32 tick = tick_get();
    u16 key;
    u16 cnt_up = 0;

    while (1) {
        WDT_CLR();
        delay_5ms(1);

        key = bsp_key_scan(1);

        if (!tick_check_expire(tick, power_on_check_time)) {
            if (key != KEY_ID_PP) {
                cnt_up++;
                if(cnt_up > 5){
                    func_pwroff();
                }
            }else{
                cnt_up = 0;
            }
        } else {
            bsp_key_set_power_on_flag(true);
            return;
        }
    }
#endif
}


AT(.text.bsp.sys.init)
void bsp_sys_init(void)
{
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    u32 rand_seed = *((u32 *)&xcfg_cb.le_addr[2]);
    sys_get_rand_key_init(rand_seed);

    bsp_var_init();

    wireless_mic_var_init();

    pmu_init(cfg_pmu_get());

#if BSP_SARADC_EN
    bsp_saradc_init();
#endif

#if BSP_KEY_EN
    bsp_key_init();
#endif

#if BSP_LED_EN
    led_init();
#if BSP_DLED_EN
    dled_init();
#endif
#endif // BSP_LED_EN

#if BSP_CHARGE_EN
    bsp_charge_init();
#endif

#if BSP_VBAT_DETECT_EN
    bsp_vbat_detect_init();
#endif // BSP_VBAT_DETECT_EN

    power_on_check();

    // clock init
    sys_clk_set(SYS_CLK_SEL);

    bsp_param_init();

    xosc_init();

    // enable user timer callback
    sys_set_tmr_enable(1, 1);

    if (MIC_ANA_MODE == MIC_MODE_NOT_RC) {
        mic_bias_trim_init(MIC_VDD_LVMODE, MIC_IN_TRIM_TARGET);
    }

#if BSP_UART_TRANSFER_EN
    //放在 ADC 偏置 trim 之后：避免 UART1 RX 中断干扰 mic_bias_trim_init 的采样时序,导致 trim 超时看门狗复位
    uart_transfer_init(BSP_UART_TRANSFER_BAUD);
#endif

#if BSP_TSEN_EN
    bsp_tsensor_init();
#endif // BSP_TSEN_EN

#if EQ_DRC_DBG_IN_UART
    if (xcfg_cb.huart_en) {
        eq_drc_dbg_init();
    }
#endif

    printf("%s\n",__func__);

    func_cb.sta = FUNC_BT;
}

void bsp_periph_init(void)
{
#if BSP_UART_DEBUG_EN
    bsp_uart_debug_init();
#endif

#if BSP_SARADC_EN
    bsp_saradc_init();
#endif

#if BSP_KEY_EN
    bsp_key_init();
#endif

#if BSP_VBAT_DETECT_EN
    bsp_vbat_detect_init();
#endif

#if BSP_TSEN_EN
    bsp_tsensor_init();
#endif // BSP_TSEN_EN

#if BSP_UART_TRANSFER_EN
    //放在所有外设初始化之后,避免 UART1 RX 中断干扰 SARADC 等外设的采样初始化
    uart_transfer_init(BSP_UART_TRANSFER_BAUD);
#endif
}

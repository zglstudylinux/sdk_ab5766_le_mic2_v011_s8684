/**********************************************************************
*
*   strong_symbol.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"
#include "driver_uart.h"

uint32_t cfg_sdk_version = SDK_VERSION;

STRONG uint8_t cfg_spiflash_speed_up_en = SPIFLASH_SPEED_UP_EN | (FLASH_DUAL_READ * 0x02) | (FLASH_QUAD_READ * 0x04);     //SPI FLASH提速。部份FLASH不支持提速
STRONG uint8_t cfg_snf_rc = CLK_SNF_RC2M;

STRONG void xosc_get_xcfg(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

STRONG void param_xosc_read(u8 *param)
{
    bsp_param_read(param, PARAM_BT_XOSC_CAP, PARAM_BT_XOSC_CAP_LEN);
}

STRONG void param_xosc_write(u8 *param)
{
    bsp_param_write(param, PARAM_BT_XOSC_CAP, PARAM_BT_XOSC_CAP_LEN);
    bsp_param_sync();
}


AT(.com_text.spiflash)
STRONG bool os_spiflash_addr_check(u32 addr, uint len)
{
    if ((addr >= CM_START) && ((addr + len) <= (CM_START + CM_SIZE))) {
        return true;
    }

#if AB_FOT_EN
    if ((addr >= FOT_PACK_START) && ((addr + len) <= (FOT_PACK_START + FOT_PACK_SIZE))) {
        return true;
    } else if ((addr >= 0x1000) && ((addr + len) <= 0x2000)) {
        return true;
    }
#endif

    printf("-->os_spiflash_addr_err:0x%x\n", addr);

    return false;
}


AT(.com_text.timer)
STRONG void usr_tmr5ms_thread_callback(void)
{
    usr_tmr5ms_thread_do();
}

STRONG void sys_clk_change_callback(void)
{

}

#if BSP_UART_DEBUG_EN
STRONG void uart_debug_init_callback(void)
{
    bsp_uart_debug_init();
}
#else
AT(.com_text.printf)
void uart_debug_putchar(char ch)
{

}

STRONG void uart_debug_init_callback(void)
{
    my_printf_init(uart_debug_putchar);
}
#endif // BSP_UART_DEBUG_EN

uint8_t pmu_cfg_vddio(void)
{
    return xcfg_cb.vddio_sel;
}

uint8_t pmu_cfg_vddbt(void)
{
    return xcfg_cb.vddbt_sel;
}

uint8_t pmu_cfg_vddcore(void)
{
    return xcfg_cb.vddcore_sel;
}

#if (!WIRELESS_FREQ_SHIFT2_EN && !ADAPTER_FREQ_SHIFT_EN)
void pacc_effect_freq_shift_init(pacc_effect_cb pacc_tbl, pacc_cs_t* next_cs, u16 frame_len) {}
#endif

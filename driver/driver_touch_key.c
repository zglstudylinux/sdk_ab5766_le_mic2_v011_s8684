/**
******************************************************************************************************************************
*
*@file driver_touch_key.c
*
*@brief Source file for Touch Key Driver
*
*@Create date: 2023-03-23
*
*
*Copyright (c) 2023, BLUETRUM
******************************************************************************************************************************
**/
#include "driver_touch_key.h"
#include "driver_gpio.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if TRACE_EN
AT(.com_rodata.irq)
const char tkey_dbg_smaller[] = "smaller pnd, %08x(%d) %d %x\n";
AT(.com_rodata.irq)
const char tkey_dbg_large[] = "large pnd, %08x(%d) %d\n";
AT(.com_rodata.irq)
const char tkey_dbg_range[] = "range pnd, %08x(%d) %d\n";
AT(.com_rodata.irq)
const char tkey_dbg_to[] = "timeout pnd, %08x(%d) %d\n";
AT(.com_rodata.irq)
const char tkey_dbg_pto[] = "press timeout pnd, %08x(%d) %d\n";
AT(.com_rodata.irq)
const char tkey_dbg_str5[] = "inpcon %08x, %d %d\n";
AT(.com_rodata.irq)
const char tkey_dbg_str6[] = "%x %x %x %x %x %X %x %x %x %x %x %x\n";
#endif

void touch_key_dump(touch_key_typedef *touch_key_reg)
{
    TRACE  ("\n");
    TRACE ("INPCON:  %08X\n", INPCON );
    TRACE ("INPCPND: %08X\n", INPCPND);
    TRACE ("TKCON:   %08X\n", touch_key_reg->tkcon);
    TRACE ("TKCON1:  %08X\n", touch_key_reg->tkcon1);
    TRACE ("TKCON2:  %08X\n", touch_key_reg->tkcon2);
    TRACE ("TKACON0:  %08X\n", touch_key_reg->tkacon0);
    TRACE ("TKACON1:  %08X\n", touch_key_reg->tkacon1);
    TRACE ("TKCNT:   %08X\n", touch_key_reg->tkcnt);
    TRACE ("TKBCNT:  %08X\n", touch_key_reg->tkbcnt);
    TRACE ("TKVARI:   %08X\n", touch_key_reg->tkvari);

    if (touch_key_reg->tkcon & TKCON_TK0EN && touch_key_reg->tkcon & TKCON_TK0PNDEN) {
        TRACE("touch key ch0 enable\n");
    }

    if (touch_key_reg->tkcon & TKCON_TK1EN && touch_key_reg->tkcon & TKCON_TK1PNDEN) {
        TRACE("touch key ch1 enable\n");
    }

    if (touch_key_reg->tkcon & TKCON_TK2EN && touch_key_reg->tkcon & TKCON_TK2PNDEN) {
        TRACE("touch key ch2 enable\n");
    }

    if (touch_key_reg->tkcon & TKCON_TK3EN && touch_key_reg->tkcon & TKCON_TK3PNDEN) {
        TRACE("touch key ch3 enable\n");
    }

    TRACE("touch key ch = 0x%02x\r\n", (touch_key_reg->tkcon & TKCON_TKSEL) >> 3);
}

AT(.com_periph.touch.get_key)
u8 touch_key_get_key(touch_key_typedef *touch_key_reg)
{
    if (!touch_key_reg->tkcon & TKCON_TKEN) {
        return 0;
    }

    if (INPCON & BIT(19)) {            //touch key state
        return 1;
    }

    return 0;
}

AT(.com_periph.tkey.isr)
void touch_key_isr(void)
{
    u32 inpcon = INPCON;
    u32 tk_exp_pnd = (inpcon >> 21) & (inpcon >> 5) & 0xf;
    u32 tk_pto_pnd = (inpcon >> 29) & (inpcon >> 13) & 0x1;
    u32 tk_pnd = (inpcon >> 16) & (inpcon >> 0) & 0x1;

    tk_exp_pnd |= (tk_pto_pnd << 4);                        //bit0-smaller exception pending
                                                            //bit1-larger exception pending
                                                            //bit2-range exception pending
                                                            //bit3-timeout exception pending
                                                            //bit4-press timeout exception pending

    if (tk_exp_pnd) {                                       //touch key exception pending
    #if TRACE_EN
        if (tk_exp_pnd & BIT(0)) {
            printf(tkey_dbg_smaller, TKBCNT, TKBCNT & 0xfff, TKCNT, inpcon);
            printf(tkey_dbg_str6, INPCON, INPCPND, TKCON, TKCON1, TKCON2, TKACON0, TKACON1, TKCNT, TKBCNT, TKCDPR0, TKCDPR1, TKVARI);
        }
        if (tk_exp_pnd & BIT(1)) {
            printf(tkey_dbg_large, TKBCNT, TKBCNT & 0xfff, TKCNT);
        }
        if (tk_exp_pnd & BIT(2)) {
            printf(tkey_dbg_range, TKBCNT, TKBCNT & 0xfff, TKCNT);
        }
        if (tk_exp_pnd & BIT(3)) {
            printf(tkey_dbg_to, TKBCNT, TKBCNT & 0xfff, TKCNT);
        }
        if (tk_exp_pnd & BIT(4)) {
            printf(tkey_dbg_pto, TKBCNT, TKBCNT & 0xfff, TKCNT);
        }
    #endif
        INPCPND = BIT(21);
    }

    if (tk_pnd) {                                           //touch key timer pending
        INPCPND = BIT(16);
        touch_key_tkcnt_isr(inpcon >> 30, TKCNT & 0xfff);
#if TRACE_EN
        printf(tkey_dbg_str5, inpcon, TKBCNT & 0xfff, TKCNT);
#endif
    }
}


/**
  * @brief  Touch key interrup enable and isr registered.
  *
  * @retval none
  */
void touch_key_isr_init(void)
{
    u32 inpcon;

    sys_irq_init(IRQn_FREQDET_TOUCH_KEY, 0, touch_key_isr);

    inpcon = BIT(0);                                        //touch key pending interrupt enable
    //inpcon |= BIT(4)                                  //touch key bcnt valid pending interrupt enable
    inpcon |= (0xf << 5);                               //touch key exception pending interrupt enable
    inpcon |= BIT(13);                                  //tkey press timeout interrupt enable
    //inpcon |= BIT(14);                                //touch key variance interrupt enable

    INPCON = inpcon;
}


/**
  * @brief  To reset touch key,so that we can change REGISTER value without power off.
  *
  * @retval none
  */
void touch_key_sw_reset(void)
{
    RTCCON0 |= BIT(4);
    INPCPND = BIT(0);               //inpwr_swrst
    RTCCON0 &= ~BIT(19);            //osc core disable
}

/**
  * @brief  To init touch key clock.
  *
  * @params[in] touch_key_reg   Touch Key Register Base Address
  * @params[in] clk_sel         Touch Key Clock Select.
  * @params[in] tkclk_div2_enable     CLK2M_TK to CLK2M_P divide
  *
  * @retval none
  */
void touch_key_clk_init(touch_key_typedef *touch_key_reg, CLK_TOUCH_KEY_TYPEDEF clk_sel, uint8_t tkclk_div2_enable)
{
    uint32_t tkcon1 = touch_key_reg->tkcon1;
    uint32_t clk_freq = 0;

    clk_touch_key_clk_set(clk_sel);

    clk_freq = clk_touch_key_clk_get(CLK_VALUE_MODE_FREQ);

   if (tkclk_div2_enable) {
        tkcon1 |= TKCON1_DIV2SEL;
        tkcon1 &= ~TKCON1_TKCLK2MDIV;
        tkcon1 |= (u32)(clk_freq / (125 * 2048)) << 16;  //125Hz  8ms
    } else {
        tkcon1 &= ~TKCON1_DIV2SEL;
        tkcon1 &= ~TKCON1_TKCLK2MDIV;
        tkcon1 |= (u32)(clk_freq / (125 * 1024)) << 16;  //125Hz  8ms
    }


    tkcon1 |= BIT(23);   //touch key clock enable
    tkcon1 |= BIT(22);   //touch key clock enable
    tkcon1 |= BIT(13);   //touch key clock enable

    touch_key_reg->tkcon1 = tkcon1;

    TRACE("%s: %x, %d\n", __func__, touch_key_reg->tkcon1, clk_freq);
}

void touch_key_ch_init(touch_key_typedef *touch_key_reg, tkey_cfg_t *tkey_cfg)
{
    tkey_cfg_t *p = (tkey_cfg_t *)tkey_cfg;
    uint32_t tkcon = touch_key_reg->tkcon;
    uint32_t tkcon1 = touch_key_reg->tkcon1;
    uint32_t tkcdpr0 = touch_key_reg->tkcdpr0;
    uint32_t tkcdpr1 = touch_key_reg->tkcdpr1;
    uint32_t tkacon0 = touch_key_reg->tkacon0;
    uint32_t tkacon1 = touch_key_reg->tkacon1;

    //BCNT CH
    tkcon &= ~TKCON_TKCH_FIX;

    tkcon &= ~TKCON_TKSEL;
    tkcon |= p->bcnt_dig_ch << 4;

    tkcon1 &= ~TKCON1_TK_CHSEL_SW;
    tkcon1 |= p->bcnt_ana_ch << 14;

    if (p->key[0]->en) {
        RTCCON1 &= ~(1 << 4);                   //WK pull up disable, or may cause TKCNT error
        RTCCON1 |= BIT(8);                      //10s reset source 0:WKO 1 touch key
    }

    //KEY0 -> WKO
    tkcon &= ~TKCON_TK0EN;
    tkcon |= p->key[0]->en * TKCON_TK0EN;  //KEY0 DIG en

    tkcon &= ~TKCON_TK0PNDEN;
    tkcon |= p->key[0]->en * TKCON_TK0PNDEN;  //KEY0 pnd en

    tkcdpr0 &= ~TKCDPR_TKCDPR0;
    tkcdpr0 |= p->key[0]->cdpr << 0;

    tkacon0 &= ~TKACON0_TK_ENCHN0;           //KEY0 ANA en
    tkacon0 |= p->key[0]->en * TKACON0_TK_ENCHN0;

    tkacon0 &= ~TKACON0_TK_CTRIM0;
    tkacon0 |= p->key[0]->ctrim << 0;

    tkacon0 &= ~TKACON0_TK_ITRIM0;
    tkacon0 |= p->key[0]->itrim << 6;

    //KEY1 -> PA3
    tkcon &= ~TKCON_TK1EN;
    tkcon |= p->key[1]->en * TKCON_TK1EN;

    tkcon &= ~TKCON_TK1PNDEN;
    tkcon |= p->key[1]->en * TKCON_TK1PNDEN;

    tkcdpr0 &= ~TKCDPR_TKCDPR1;
    tkcdpr0 |= p->key[1]->cdpr << 16;

    tkacon0 &= ~TKACON0_TK_ENCHN1;
    tkacon0 |= p->key[0]->en * TKACON0_TK_ENCHN1;

    tkacon0 &= ~TKACON0_TK_CTRIM1;
    tkacon0 |= p->key[1]->ctrim << 12;

    tkacon0 &= ~TKACON0_TK_ITRIM1;
    tkacon0 |= p->key[1]->itrim << 18;

    //KEY2 -> PA4
    tkcon &= ~TKCON_TK2EN;
    tkcon |= p->key[2]->en * TKCON_TK2EN;

    tkcon &= ~TKCON_TK2PNDEN;
    tkcon |= p->key[2]->en * TKCON_TK2PNDEN;

    tkcdpr1 &= ~TKCDPR_TKCDPR1;
    tkcdpr1 |= p->key[2]->cdpr << 0;

    tkacon1 &= ~TKACON1_TK_ENCHN2;
    tkacon1 |= p->key[2]->en * TKACON1_TK_ENCHN2;

    tkacon1 &= ~TKACON1_TK_CTRIM2;
    tkacon1 |= p->key[2]->ctrim << 0;

    tkacon1 &= ~TKACON1_TK_ITRIM2;
    tkacon1 |= p->key[2]->itrim << 6;

    //KEY3 -> PA5
    tkcon &= ~TKCON_TK3EN;
    tkcon |= p->key[3]->en * TKCON_TK3EN;

    tkcon &= ~TKCON_TK3PNDEN;
    tkcon |= p->key[3]->en * TKCON_TK3PNDEN;

    tkcdpr1 &= ~TKCDPR_TKCDPR1;
    tkcdpr1 |= p->key[3]->cdpr << 16;

    tkacon1 &= ~TKACON1_TK_ENCHN3;
    tkacon1 |= p->key[3]->en * TKACON1_TK_ENCHN3;

    tkacon1 &= ~TKACON1_TK_CTRIM3;
    tkacon1 |= p->key[3]->ctrim << 12;

    tkacon1 &= ~TKACON1_TK_ITRIM3;
    tkacon1 |= p->key[2]->itrim << 18;

    touch_key_reg->tkcon   = tkcon;
    touch_key_reg->tkcon1  = tkcon1;
    touch_key_reg->tkcdpr0 = tkcdpr0;
    touch_key_reg->tkcdpr1 = tkcdpr1;
    touch_key_reg->tkacon0 = tkacon0;
    touch_key_reg->tkacon1 = tkacon1;
}

void touch_key_bcnt_calibration_init(touch_key_typedef *touch_key_reg, tkey_cfg_t *tkey_cfg)
{
    uint32_t tkcon = touch_key_reg->tkcon;

    if (tkey_cfg->auto_calib) {
       tkcon |= TKCON_TKTOUPDBEN |  TKCON_TKRUPDBEN | TKCON_TKLUPDBEN | TKCON_TKSUPDBEN;
    } else {
       tkcon &= ~(TKCON_TKTOUPDBEN |  TKCON_TKRUPDBEN | TKCON_TKLUPDBEN | TKCON_TKSUPDBEN);
    }

    touch_key_reg->tkcon = tkcon;
}

/**
  * @brief  To init touch key configuration,including clock and other registers.
  *
  * @params[in] touch_key_reg   Touch Key Register Base Address
  * @params[in] first_pwron     To Tell The Driver Whether Touch Key Is First Time Init Or Not.
  * @params[in] clk_sel         Touch Key Clock Select.
  * @params[in] tkclk_div2_enable     CLK2M_TK to CLK2M_P divide, typical value is 0.
  *
  * @retval     -1:     If failed
                0:      If success
  */
int touch_key_init(touch_key_typedef *touch_key_reg, void *tkey_cfg, CLK_TOUCH_KEY_TYPEDEF clk_sel, uint8_t tkclk_div2_enable)
{
    int ret_code = 1;
    tkey_cfg_t *p = (tkey_cfg_t *)tkey_cfg;

    //Touch Key Reset, or it will cause register write failure
    RTCCON0 |= BIT(5);

    touch_key_clk_init(touch_key_reg, clk_sel, tkclk_div2_enable);
    touch_key_ch_init(touch_key_reg, tkey_cfg);
    touch_key_bcnt_calibration_init(touch_key_reg, tkey_cfg);

    uint32_t tkcon = touch_key_reg->tkcon;
    uint32_t tkcon1 = touch_key_reg->tkcon1;

    tkcon1 &= ~TKCON1_TKARTRIM;
    tkcon1 |= 1 << 0;

    tkcon |= TKCON_TKSTAWKEN;    //touch key wakeup enable
    tkcon |= TKCON_TKBCNTVAL;    //touch key base_cnt valid enable
    tkcon |= TKCON_TKEN;         //touch key enable
    tkcon |= TKCON_TKAEN_ATSEL;  //touch key auto enable
    tkcon |= TKCON_TKMODE;       //touch key auto enable

    touch_key_reg->tkbcnt = 0xfff;
    touch_key_reg->tkcon = tkcon;
    touch_key_reg->tkcon1 = tkcon1;
    touch_key_reg->tktmr = p->reg_tktmr;
    touch_key_reg->tkpthd = p->reg_tkpthd;
    touch_key_reg->tkethd = p->reg_tkethd;
    touch_key_reg->tkvari = p->reg_tkvari;
    touch_key_reg->tkvarithd = p->reg_tkvarithd;
    touch_key_reg->tkcon2 = p->reg_tkcon2;

    touch_key_isr_init();

    touch_key_dump(touch_key_reg);

    return ret_code;
}


/*
 * @File name    : driver_lowpwr.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-17
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#include "driver_gpio.h"
#include "driver_saradc.h"
#include "driver_lowpwr.h"

//VRTC11 = 0.7V + n * 0.065
#define VRTC11_NORMAL   6   //RTCCON4 [27-29]
#define VRTC11_LOWPWR   3   //RTCCON4 [24-26]

extern u32 __comm_vma;

extern u8 pmu_get_vddcore(void);

static lowpwr_sfr_backup_t sfr_backup;

static struct {
    bool        need_rc;
    uint32_t    gpio_pin;
} lowpwr_config;

uint32_t lowpwr_get_wakeup_source(void)
{
    u32 reason = RTCCON9 | RTCCON10;

    if(reason){
        if(reason & WK_LP_KEYSCAN_PENDING){
            printf("KEY_SCAN_WK\n");
        }
        if(reason & WK_LP_QDEC_PENDING){
            printf("QDEC_WK\n");
        }
        if(reason & WK_LP_GPIO_PENDING){
            printf("GPIO_WK\n");
        }
        if(reason & WK_LP_TOUCH_PENDING){
            printf("TOUCH_WK\n");
        }
        if(reason & WK_LP_INBOX_PENDING){
            printf("INBOX_WK\n");
        }
        if(reason & WK_LP_VUSB_PENDING){
            printf("VUSB_WK\n");
        }
        if(reason & WK_LP_WK0_PENDING){
            printf("WKO_WK\n");
        }
        if(reason & WK_LP_RTC_1S_PENDING){
            printf("RTC1S_WK\n");
        }
        if(reason & WK_LP_ALARM_PENDING){
            printf("ALMWKP_WK\n");
        }

        WK_LP_PENDING_CLR();
    }

    return reason;

}

void lowpwr_clr_wakeup_pending(void)
{
    GPIO_EDGE_PENDING_CLR();

    WK_LP_PENDING_CLR();
}

void lowpwr_wakeup_disable(void)
{
    lowpwr_clr_wakeup_pending();

    gpio_edge_capture_disable();

    lowpwr_config.gpio_pin = 0;

    WK_LP_SRC_ALL_DIS();
}

AT(.com_periph.lowpwr.sleep)
bool lowpwr_is_wakeup_pending(u32 ignore_src)
{
    u32 wk_pending = 0x1fff;

    if(ignore_src){
        wk_pending &= ~ignore_src;
    }

    return (RTCCON9 & wk_pending);
}

void lowpwr_wk0_wakeup_config(lowpwr_wakeup_typedef *config)
{
#if 1
    if (config->gpio_cfg.edge == GPIO_EDGE_FALLING) {
        wko_io_config(ENABLE, DISABLE, ENABLE);
        RTCCON1 &= ~BIT(6);         //wk pin low level wakeup
    } else {
        wko_io_config(DISABLE, ENABLE, ENABLE);
        RTCCON1 |= BIT(6);          //wk pin high level wakeup
    }
    WK_LP_SRC_EN(WK_LP_WK0);
#else
    wk0_edge_capture_config(config->gpio_cfg.edge);
    GPIO_WK_SLEEP_EN();
    WK_LP_SRC_EN(WK_LP_GPIO);
#endif
}

uint32_t lowpwr_gpio_wakeup_pin_get(void)
{
    return lowpwr_config.gpio_pin;
}

void lowpwr_gpio_wakeup_config(lowpwr_wakeup_typedef *config)
{
    uint32_t pin = config->gpio_cfg.gpio_pin;

    if (config->gpio_cfg.gpiox == GPIOB_REG) {
        pin = config->gpio_cfg.gpio_pin << 16;
    }
    lowpwr_config.gpio_pin |= pin;

    gpio_edge_capture_config(&config->gpio_cfg);

    GPIO_WK_SLEEP_EN();

    WK_LP_SRC_EN(WK_LP_GPIO);
}

static void lowpwr_wakeup_config_do(lowpwr_wakeup_typedef *config)
{
    if(config){
        if(!(config->source & WK_LP_TOUCH)){
            RTCCON0 &= ~BIT(4);     //touch key between core interface disable
        }

        if(config->source & WK_LP_WK0){
            lowpwr_wk0_wakeup_config(config);
        }

        if(config->source & WK_LP_TOUCH){
            RTCCON0 |= BIT(4);          //touch key between core interface enable
            RTCCON1 |= BIT(9);          //VDDTK enable for touch key logic
        }

        if(config->source & WK_LP_GPIO){
            lowpwr_gpio_wakeup_config(config);
        }

        WK_LP_SRC_EN(config->source);

        if (WK_LP_SRC_IS_EN(WK_LP_ALARM | WK_LP_RTC_1S | WK_LP_BT| WK_LP_TOUCH | WK_LP_KEYSCAN | WK_LP_QDEC)) {
            lowpwr_config.need_rc = true;
        } else {
            lowpwr_config.need_rc = false;
        }
    }
}

void lowpwr_wakeup_config(lowpwr_wakeup_typedef *config)
{
    if(config){
        lowpwr_wakeup_config_do(config);
    }
}

void lowpwr_wakeup_source_enable(WK_LP_SRC_SEL wk_src)
{
    WK_LP_SRC_EN(wk_src);
}

void lowpwr_wakeup_source_disable(WK_LP_SRC_SEL wk_src)
{
    if (wk_src == WK_LP_SRC_ALL) {
        WK_LP_SRC_ALL_DIS();
        return;
    }

    WK_LP_SRC_DIS(wk_src);
}

ALIGNED(512)
void lowpwr_pwroff_mode1_do(void)
{
    u32 rtccon3;

    saradc_deinit();            //disable saradc

    PICCONCLR = BIT(0);         //disable global interrupt
    NMICON = 0;

    WK_LP_SRC_DIS(WK_LP_GPIO);  //pwroff mode1 disable vddio,so here we must disable gpio wakeup

    WK_LP_PENDING_CLR();

    if (WK_LP_SRC_IS_EN(WK_LP_TOUCH)) {               //Touch need rc2m clk
        RTCCON0 |= SNF_RC_EN_BIT;                         //RC2M_RTC enable
        asm("nop");asm("nop");asm("nop");
        RTCCON0 = (RTCCON0 & ~(0x03 << 12)) | (0x02 << 12); //CLK2M in Touch select SNF_RC_RTC
    } else{
        RTCCON0  &= ~SNF_RC_EN_BIT;                       //RC2M_RTC disable
    }

    RTCCON4 &= ~BIT(15);    //VIO_PD not work

    RTCCON4 = (RTCCON4 & ~(0x3f<<24)) | (VRTC11_LOWPWR<<24) | (VRTC11_NORMAL<<27);

    CLKCON0 &= ~0xc;        //sys_clk switch to RC
    asm("nop");
    asm("nop");
    asm("nop");

    XOSCCON = 0;
    PLL0CON = 0;

    RTCCON1 |= BIT(19);                         //c2r_lf_pd_en

    rtccon3 = RTCCON3 & ~(0xFF);        //disable [0]VDDBT, [1]VDDIO, [2]VDDCORE,[3]VRTC11 bypass to VLCORE,[5]VDDCORE bypass to VLCORE,[7]VDDIO_AON
    rtccon3 |= BIT(4) | BIT(6);         //[4]core power down 2 enable, [6]core power down enable

    WDT_DIS();

    LPMCON |= BIT(3);       //避免关机过程被残留的pending误唤醒，引起RTC寄存器概率性出错

    RTCCON |= BIT(5);       //RTC wake up power down mode reset system enable
    RTCCON3 = rtccon3;

    LPMCON |= BIT(0);	    //sleep mode
    LPMCON |= BIT(1);       //idle mode

    asm("nop");asm("nop");asm("nop");

    while(1);
}

//in pwroff_mode2_final, can't use subfunction
AT(.com_text.pwroff.mode2)
static void pwroff_mode2_final(void)
{
    u32 rtccon3;
    u32 rtccon0;

    ///wakeup source need rc2m clk
    if (lowpwr_config.need_rc) {
        RTCCON0 |= SNF_RC_EN_BIT;                   //RC2M_RTC enable
    } else{
        RTCCON0 &= ~SNF_RC_EN_BIT;                  //RC2M_RTC disable
    }

    PWRCON1 = (PWRCON1 & ~(0XF<<14)) | (0X0<<14);   //DI_SW DISABLE

    RTCCON8 &= ~BIT(29);

    RTCCON4 = (RTCCON4 & ~(0x3f<<24)) | (VRTC11_LOWPWR<<24) | (VRTC11_NORMAL<<27);

    rtccon0 = RTCCON0;

    if (WK_LP_SRC_IS_EN(WK_LP_TOUCH)) {
        rtccon0 = (rtccon0 & ~(0x03 << 12)) | (SNF_RC_SEL_BIT << 12); //CLK2M in Touch select SNF_RC_RTC
    }

    if (WK_LP_SRC_IS_EN(WK_LP_KEYSCAN)) {
        rtccon0 = (rtccon0 & ~(0x03 << 14)) | (SNF_RC_SEL_BIT << 14); //CLK2M in Keyscan select SNF_RC_RTC
    }

    RTCCON0 = rtccon0;

    CLKCON0 &= ~0xc;        //sys_clk switch to RC
    asm("nop");
    asm("nop");
    asm("nop");

    XOSCCON = 0;
    PLL0CON = 0;

//  when switch power, reduce lp_top power, disable ram0/1 clock
    CLKGAT0 &= ~((3<<2) | BIT(15)); // sram0/1 & LP disable clock

    RTCCON5 &= ~(1<<11); // dis sram if

    RTCCON5 |= (BIT(15) | BIT(16));   //RAM0 RAM1 shutdown
    //RTCCON5 |= BIT(13); //RAM0/1 light sleep
    //RTCCON5 |= BIT(14); //RAM0/1 deep sleep

    // pmu power down
    LVDCON &= ~(BIT(3) | BIT(5));               //disable LVD reset
    RTCCON1 |= BIT(19);                         //c2r_lf_pd_en

    rtccon3 = RTCCON3 & ~(0xFF);                //disable [0]VDDBT, [1]VDDIO, [2]VDDCORE,[5]VDDCORE bypass to VLCORE
    rtccon3 |= (BIT(7) | BIT(4) | BIT(3));      //[3]VRTC11 bypass to VLCORE, [4]core power down 2 enable,[7]VDDIO_AON

    //disable WDT
    WDT_DIS();

    LPMCON |= BIT(3);   //避免关机过程被残留的pending误唤醒，引起RTC寄存器概率性出错

    RTCCON |= BIT(5);   //RTC wake up power down mode reset system enable

    RTCCON3 = rtccon3;
    LPMCON |= BIT(0);	//sleep mode
    LPMCON |= BIT(1);   //idle mode
    while(1);
}

AT(.com_text.pwroff_mode2)
void lowpwr_pwroff_mode2_do(void)
{
    u8 *src, *dst;
    u32 i;
    void (*p_func)(void);

    p_func = (void (*)(void))CACHE_ADDR;

    saradc_deinit();

    PICCONCLR = BIT(0);     //disable global interrupt
    NMICON = 0;

    WK_LP_PENDING_CLR();

    PWRCON1 &= ~(0xf<<14);  //disable flash power gate
    PWRCON1 |= BIT(18);

    GPIOGDE = 0;
    GPIOGPU = 0;

    RTCCON5 &= ~(1<<12);   //CLEAR LP FLAG
    GPIO_EDGE_PENDING_CLR();

    //copy pwroff_mode2_final to cache ram

    dst = (u8*) CACHE_ADDR;
    src = (u8*) (&pwroff_mode2_final);
    for(i = 0; i < 0x300; i++){
        *dst++ = *src++;
    }

    //jmp to run lp_sniff_final()
    p_func();
}


AT(.com_sleep.lv2)
WEAK void lowpwr_sleep_sfr_user_save(void)
{

}

AT(.com_sleep.lv2)
WEAK void lowpwr_sleep_sfr_user_resume(void)
{

}


AT(.com_sleep.lv2_do)
void lp_sleep_lv2_do(void)
{
    u32 rtccon3 = RTCCON3;

    // 1. disable clock
    CLKGAT0 &= ~(BIT(15) | BIT(3) | BIT(2)); //lp clk, sram0/1 disable clock

    // 2. disable interface
    //    disable if, then deep sleep memory, can not do in 1 cycle
    RTCCON5 &= ~(1<<11);    // dis sram if
    //RTCCON5 = (RTCCON5 & ~(0x7<<13)) | (1<<13);
    RTCCON5 |=  (2<<13);    // 1xx : shutdown
                            // 01x : deep sleep
                            // 001 : light sleep
                            // 000 : normal
    RTCCON5 |= (1<<17);     // set lp_flag

    RTCCON5 |= BIT(0);

    WDT_DIS();

    RTCCON1 |= BIT(19);                         //c2r_lf_pd_en

    // pmu power down ...
    rtccon3 &= ~(1<<5);                 //disable VDDCORE bypass to VLCORE
    rtccon3 |= ((1<<3) | (1<<4));       //[3]VRTC11 bypass to VLCORE, [4]core power down 2 enable
    rtccon3 &= ~(0x7<<0);               //disable [0]VDDBT, [1]VDDIO, [2]VDDCORE

    LPMCON |= BIT(3);       //避免关机过程被残留的pending误唤醒，引起RTC寄存器概率性出错

    RTCCON |= BIT(5);       //RTC wake up power down mode reset system enable
    RTCCON3 = rtccon3;
//    RTCCON3 |= (1<<6);          // PD CORE

    LPMCON |= BIT(0);
    LPMCON |= BIT(1);       //idle mode
    asm("nop");
    asm("nop");
    asm("nop");
    while(1);
}

AT(.com_sleep.lv2_proc)
void lp_sleep_lv2_proc(void)
{
    u32 rtccon0 = RTCCON0;
    u32 *dst = (u32 *)0x70000;
    u32 *src = (u32 *)&lp_sleep_lv2_do;

    for(uint i=0; i<512/4; i++) {
        *dst++ = *src++;
    }

    // 1, save IO
    CLKGAT0 |= (1<<15);         // LP_CLK Enable
    RTCCON5 &= ~(1<<12);        // CLEAR LP FLAG

    WK_LP_PENDING_CLR();

    RTCCON8 &= ~BIT(29);
    RTCCON3 |= BIT(7);      //enable vddio aon ldo

    RTCCON4 = (RTCCON4 & ~(0x3f<<24)) | (VRTC11_LOWPWR<<24) | (VRTC11_NORMAL<<27);

    // PG OFF
    PWRCON1 |= BIT(9) | BIT(18);            // SD/FS PG PD
    PWRCON1  = PWRCON1 & ~((0x3f << 10)) ;  // PG off
    // ... other analog pd

    if (WK_LP_SRC_IS_EN(WK_LP_TOUCH)) {
        rtccon0 = (rtccon0 & ~(0x03 << 12)) | (SNF_RC_SEL_BIT << 12); //CLK2M in Touch select SNF_RC_RTC
    }

    if (WK_LP_SRC_IS_EN(WK_LP_KEYSCAN)) {
        rtccon0 = (rtccon0 & ~(0x03 << 14)) | (SNF_RC_SEL_BIT << 14); //CLK2M in Keyscan select SNF_RC_RTC
    }

    RTCCON0 = rtccon0;

    // 3, select sniff rc clock and disable pll,xosc
    CLKCON0  = (CLKCON0 & ~(3<<2)) | (0<<2); // sel rc2m
    asm("nop");
    asm("nop");
    asm("nop");

    // 4, PLL, XOSC, CLK2M_BT off
    PLL0CON  = 0;
    PLL0CON1 = 0;
    XOSCCON = 0;

    asm("j 0x70000");
};

AT(.com_sleep.lv2_wakeup)
void lp_wakeup_lv2_proc(void)
{
    RTC_WDT_EN();

	RTCCON1 &= ~BIT(19);                                    //c2r_lf_pd_en

    RTCCON4 &= ~BIT(6);                                     //VDDBT to VCORE dis

    RTCCON4 |= BIT(5) ;                                     //VCORE from VDDIO

    PICADR = (u32)&__comm_vma;

    LVDCON &= ~BIT(30);                                     //Enable reset pending

    if(!(sfr_backup.rtccon5 & BIT(0))){                     //buck mode
        PWRCON1 |= BIT(7);                                  //DI_EN_CLIMIT = 1
        PWRCON1 = (PWRCON1 & ~(0X3<<2)) | (0X1<<2);         //DI_CLIMIT_S   109:85:366mA
        RTCCON5 &= ~BIT(0);
    }

    PWRCON0 |= BIT(20);                                     //pmu normal
    delay(25);

    // 2, analog init
    RTCCON3 |= BIT(0);                                      //VDDDBT
    PWRCON0 = (PWRCON0 & ~0x1f) | pmu_get_vddcore();        //VDDCORE is 1.15V
    asm("nop");

    PLL0CON |= BIT(12);                                     //DI_EN_LDO

    RSTCON0 = sfr_backup.rstcon0;                           //pllsdm reset release

    // 3, PLL,XOSC,CLK2M_BT, on
    XOSCCON = sfr_backup.xosccon;
    delay(380);

    PLL0CON  = sfr_backup.pll0con;
    PLL0CON1 = sfr_backup.pll0con1;

    CLKDIVCON0 = sfr_backup.clkdivcon0;

    // 4, resume CLK
    CLKCON1 = sfr_backup.clkcon1;                           //xosc_mux_clk select xosc24m
    asm("nop");
    asm("nop");
    asm("nop");

    CLKCON0 = sfr_backup.clkcon0;                           //sysclk select XOSC24M
    asm("nop");
    asm("nop");

    CLKCON4 = sfr_backup.clkcon4;
}

AT(.com_sleep.lv2_proc)
WEAK void lowpwr_sleep_lv2_proc(void)
{
    sfr_backup.piccon  = PICCON;
    sfr_backup.picen   = PICEN;
    sfr_backup.picpr   = PICPR;
    sfr_backup.nmicon  = NMICON;

    sfr_backup.clkgat0 = CLKGAT0;
    sfr_backup.clkgat1 = CLKGAT1;
    sfr_backup.clkgat2 = CLKGAT2;

    sfr_backup.pwrcon0 = PWRCON0 & ~BIT(20);
    sfr_backup.pwrcon1 = PWRCON1;
    sfr_backup.pwrcon2 = PWRCON2;

    sfr_backup.clkcon0 = CLKCON0;
    sfr_backup.clkcon1 = CLKCON1;
    sfr_backup.clkcon4 = CLKCON4;
    sfr_backup.clkdivcon0 = CLKDIVCON0;

    sfr_backup.xosccon = XOSCCON;
    sfr_backup.pll0con = PLL0CON;
    sfr_backup.pll0con1 = PLL0CON1;

    sfr_backup.rstcon0 = RSTCON0;
    sfr_backup.rtccon = RTCCON;
    sfr_backup.rtccon5 = RTCCON5;

    lowpwr_sleep_sfr_save();
    lowpwr_sleep_sfr_user_save();

    u32 gpiogde = GPIOGDE;
    u32 gpiogpu = GPIOGPU;

    GPIOGDE = 0;
    GPIOGPU = 0;
    PWRCON1 &= ~(0XF<<14);                          //DI_SW DISABLE

    sfr_backup.rtccon0 = RTCCON0;

    if (lowpwr_config.need_rc) {
        RTCCON0 |= SNF_RC_EN_BIT;                   //RC2M_RTC enable
    } else{
        RTCCON0 &= ~SNF_RC_EN_BIT;                  //RC2M_RTC disable
    }

    lp_sleep(lp_sleep_lv2_proc, lp_wakeup_lv2_proc);

    RTCCON4 |= BIT(5) | BIT(6);                     //VCORE from VDDBT & VDDIO

    RTCCON0 = sfr_backup.rtccon0;
    asm("nop");
    asm("nop");

    PWRCON1 = sfr_backup.pwrcon1 | BIT(7);
    PWRCON2 = sfr_backup.pwrcon2;
    PWRCON0 = sfr_backup.pwrcon0 | BIT(20);

    CLKGAT0 = sfr_backup.clkgat0;
    CLKGAT1 = sfr_backup.clkgat1;
    CLKGAT2 = sfr_backup.clkgat2;

	lowpwr_sleep_sfr_resume();
    lowpwr_sleep_sfr_user_resume();

    GPIOGDE = gpiogde;
    GPIOGPU = gpiogpu;

    RTCCON = sfr_backup.rtccon;

	PICCON  = sfr_backup.piccon;
	PICPR   = sfr_backup.picpr;
	PICEN   = sfr_backup.picen;
	NMICON  = sfr_backup.nmicon;

    if(sfr_backup.rtccon5 & BIT(0)){
        RTCCON4 &= ~BIT(6);                         //LDO Mode,VCORE disable VDDBT, from VDDIO
    }else{
        RTCCON4 &= ~BIT(5);                         //Buck Mode,VCORE disable VDDIO, from VDDBT
    }

    PWRCON1 &= ~BIT(7);
}

AT(.com_sleep.sleep)
void lowpwr_sleep_proc(void)
{
    lowpwr_sleep_lv2_proc();
}

AT(.com_sleep.sleep)
void lowpwr_sleep_exit(void)
{
    if (SNF_RC_IS_300K()) {
        RTCCON4 = (RTCCON4 & ~(0x7<<27)) | (VRTC11_NORMAL<<27);
    }
}

/*
 * @File name    : driver_lowpwr.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-17
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_LOWPWR_H
#define _DRIVER_LOWPWR_H

#include "driver_com.h"
#include "driver_gpio.h"

#define WK_LP_SRC_EN(src)           (RTCCON3 |= src)
#define WK_LP_SRC_DIS(src)          (RTCCON3 &= ~src)
#define WK_LP_SRC_IS_EN(src)        (RTCCON3 & (src))
#define WK_LP_SRC_ALL_DIS()         (RTCCON3 &= ~0x17ff00)

#define WK_LP_PENDING_CLR()         {RTCCON9 = 0x1fff; RTCCON10 = 0xffff;}


//As defined by RTCCON3
typedef enum {
    WK_LP_ALARM             = BIT(8),
    WK_LP_RTC_1S            = BIT(9),
    WK_LP_WK0               = BIT(10),
    WK_LP_VUSB              = BIT(11),
    WK_LP_INBOX             = BIT(12),
    WK_LP_BT                = BIT(13),
    WK_LP_TOUCH             = BIT(14),
    WK_LP_KEYSCAN           = BIT(16),
    WK_LP_GPIO              = BIT(17),
    WK_LP_QDEC              = BIT(20),
    WK_LP_SRC_ALL           = 0xFFFF,
} WK_LP_SRC_SEL;

//As defined by RTCCON9
typedef enum {
    WK_LP_ALARM_PENDING     = BIT(0),
    WK_LP_RTC_1S_PENDING    = BIT(1),
    WK_LP_WK0_PENDING       = BIT(2),
    WK_LP_VUSB_PENDING      = BIT(3),
    WK_LP_INBOX_PENDING     = BIT(4),
    WK_LP_BT_PENDING        = BIT(5),
    WK_LP_TOUCH_PENDING     = BIT(6),
    WK_LP_GPIO_PENDING      = BIT(7),
    WK_LP_QDEC_PENDING      = BIT(8),
    WK_LP_KEYSCAN_PENDING   = BIT(12),
} WK_LP_PENDING_SEL;

typedef struct {
    u32 piccon;
    u32 picen;
    u32 picpr;
    u32 nmicon;
    u32 clkgat0;
    u32 clkgat1;
    u32 clkgat2;

    u32 pwrcon0;
    u32 pwrcon1;
    u32 pwrcon2;

    u32 xosccon;
    u32 pll0con;
    u32 pll0con1;

    u32 clkcon0;
    u32 clkcon1;
    u32 clkcon4;
    u32 clkdivcon0;

    u16 rstcon0;
    u32 rtccon;
    u32 rtccon0;
    u32 rtccon5;
} lowpwr_sfr_backup_t;

typedef struct {
    WK_LP_SRC_SEL           source;
    gpio_edge_cap_typedef   gpio_cfg;
} lowpwr_wakeup_typedef;

uint32_t lowpwr_get_wakeup_source(void);
void lowpwr_wakeup_disable(void);
bool lowpwr_is_wakeup_pending(u32 ignore_src);
void lowpwr_clr_wakeup_pending(void);
void lowpwr_wakeup_config(lowpwr_wakeup_typedef *config);
void lowpwr_wakeup_source_enable(WK_LP_SRC_SEL wk_src);
void lowpwr_wakeup_source_disable(WK_LP_SRC_SEL wk_src);
void lowpwr_wk0_wakeup_config(lowpwr_wakeup_typedef *config);
void lowpwr_gpio_wakeup_config(lowpwr_wakeup_typedef *config);
void lowpwr_sleep_proc(void);
void lowpwr_sleep_exit(void);
uint32_t lowpwr_gpio_wakeup_pin_get(void);
/**
  * @brief  only rtc digital, vddio is disable, only wk0 can wakeup system
  */
void lowpwr_pwroff_mode1_do(void);

/**
  * @brief  rtc digital,low power and vddio is enable, wk0, keyscan, touch and gpio can wakeup system
  */
void lowpwr_pwroff_mode2_do(void);


#endif

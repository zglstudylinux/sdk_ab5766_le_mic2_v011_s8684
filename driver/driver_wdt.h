/*
 * @File name    : driver_wdt.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-01
 * @Description  : This file contains all the functions prototypes for the WDT library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_WDT_H
#define _DRIVER_WDT_H

#include "driver_com.h"

//clk select enumeration
typedef enum {
    WDT_CLK_RC32K       = 0x00010000,
    WDT_CLK_X32K        = 0x00000000,
} WDT_CLK_TYPEDEF;

//time select enumeration
typedef enum {
    WDT_TIME_1MS        = 0x00000000,
    WDT_TIME_256MS      = 0x00100000,
    WDT_TIME_512MS      = 0x00200000,
    WDT_TIME_1024MS     = 0x00300000,
    WDT_TIME_2048MS     = 0x00400000,
    WDT_TIME_4096MS     = 0x00500000,
    WDT_TIME_8192MS     = 0x00600000,
    WDT_TIME_16384MS    = 0x00700000,
} WDT_TIME_TYPEDEF;

/**
  * @brief  Enable or disable the WDT peripheral.
  * @param  state: the state of the WDT peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void wdt_cmd(FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the WDT reset.
  * @param  state: the state of the WDT reset.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void wdt_reset_cmd(FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the WDT interrupt.
  * @param  state: the state of the WDT interrupt.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void wdt_it_cmd(FUNCTIONAL_STATE state);

/**
  * @brief  Secect the WDT clock.
  * @param  clk: The WDT clock.
  *         this parameter can be one of the following values:
  *             @arg WDT_CLK_RC32K: RC32K.
  *             @arg WDT_CLK_X32K: X32K from 26M divider.
  * @retval None.
  */
void wdt_clk_select(WDT_CLK_TYPEDEF clk);

/**
  * @brief  Secect the WDT time.
  * @param  time: The WDT time.
  *         this parameter can be one of the following values:
  *             @arg WDT_TIME_1MS: 1ms.
  *             @arg WDT_TIME_256MS: 256ms.
  *             @arg WDT_TIME_512MS: 512ms.
  *             @arg WDT_TIME_1024MS: 1024ms.
  *             @arg WDT_TIME_2048MS: 2048ms.
  *             @arg WDT_TIME_4096MS: 4096ms.
  *             @arg WDT_TIME_8192MS: 8192ms.
  *             @arg WDT_TIME_16384MS: 16384ms.
  * @retval None.
  */
void wdt_time_select(WDT_TIME_TYPEDEF time);

/**
  * @brief  Get the WDT flag.
  * @param  wdt_flag: specifies the flag to set.
  *         this parameter can be one of the following values:
  *             @arg WDT_FLAG_PENDING: WDT pending.
  * @retval The state of wdt_flag (SET or RESET).
  */
FLAG_STATE wdt_get_flag(uint32_t wdt_flag);

/**
  * @brief  Clear the WDT pending.
  * @param  None
  * @retval None
  */
void wdt_clear(void);

#endif // _DRIVER_WDT_H

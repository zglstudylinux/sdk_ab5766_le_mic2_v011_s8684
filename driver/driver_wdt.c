/*
 * @File name    : driver_wdt.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-01
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the WDT peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#include "driver_wdt.h"


void wdt_cmd(FUNCTIONAL_STATE state)
{
    wdt_typedef * wdt = WDT_REG;
    uint32_t temp_reg = wdt->con;

    if (state) {
        temp_reg |= (uint32_t)(0x01 << WDTCON_WDTEN_LSB);
    } else {
        temp_reg &= ~WDTCON_WDTEN_MASK;
        temp_reg |= (uint32_t)(0x0a << WDTCON_WDTEN_LSB);
    }

    wdt->con = temp_reg;
}

void wdt_reset_cmd(FUNCTIONAL_STATE state)
{
    wdt_typedef * wdt = WDT_REG;
    uint32_t temp_reg = wdt->con;

    if (state) {
        temp_reg |= (uint32_t)(0x01 << WDTCON_RSTEN_LSB);
    } else {
        temp_reg &= ~WDTCON_RSTEN_MASK;
        temp_reg |= (uint32_t)(0x0a << WDTCON_RSTEN_LSB);
    }

    wdt->con = temp_reg;
}

void wdt_it_cmd(FUNCTIONAL_STATE state)
{
    wdt_typedef * wdt = WDT_REG;
    uint32_t temp_reg = wdt->con;

    temp_reg &= ~WDTCON_WDTIE_MASK;

    if (state) {
        temp_reg |= (uint32_t)(0x05 << WDTCON_WDTIE_LSB);
    } else {
        temp_reg |= (uint32_t)(0x0a << WDTCON_WDTIE_LSB);
    }

    wdt->con = temp_reg;
}

void wdt_clk_select(WDT_CLK_TYPEDEF clk)
{
    wdt_typedef * wdt = WDT_REG;
    uint32_t temp_reg = wdt->con;

    temp_reg &= ~WDTCON_WDTCSEL_MASK;
    temp_reg |= clk;

    if(clk) {
        temp_reg |= (uint32_t)(0x05 << WDTCON_WDTCSEL_LSB);
    } else {
        temp_reg |= (uint32_t)(0x0a << WDTCON_WDTCSEL_LSB);
    }

    wdt->con = temp_reg;
}

void wdt_time_select(WDT_TIME_TYPEDEF time)
{
    wdt_typedef * wdt = WDT_REG;
    uint32_t temp_reg;

    temp_reg = wdt->con;

    temp_reg &= ~(WDTCON_TMRSEL_MASK | WDTCON_TMRSEL_WR_MASK);
    temp_reg |= time;
    temp_reg |= (0x0a << WDTCON_TMRSEL_WR_LSB);

    wdt->con = temp_reg;
}

FLAG_STATE wdt_get_flag(uint32_t wdt_flag)
{
    wdt_typedef * wdt = WDT_REG;
    if ((wdt->con & WDTCON_WDTPND) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

void wdt_clear(void)
{
    wdt_typedef * wdt = WDT_REG;
    wdt->con = (uint32_t)0x0a;
}

/*
 * @File name    : driver_keyscan.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-11
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the KEYSCAN peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#include "driver_keyscan.h"

void keyscan_init(keyscan_typedef * keyscan, keyscan_init_typedef *keyscan_init_struct)
{
    /* Configure Control0 Register */
    keyscan->con1 &= ~(KEYSCAN_CON1_KEYPORT_STAB_TIME | KEYSCAN_CON1_KEY_DEBO_TIMES | KEYSCAN_CON1_KEY_WAIT_TIMES);
    keyscan->con1 |= (keyscan_init_struct->stable_times * KEYSCAN_CON1_KEYPORT_STAB_TIME_0) & KEYSCAN_CON1_KEYPORT_STAB_TIME;
    keyscan->con1 |= (keyscan_init_struct->debounce_times * KEYSCAN_CON1_KEY_DEBO_TIMES_0) & KEYSCAN_CON1_KEY_DEBO_TIMES;
    keyscan->con1 |= (keyscan_init_struct->wait_times * KEYSCAN_CON1_KEY_WAIT_TIMES_0) & KEYSCAN_CON1_KEY_WAIT_TIMES;

    /* Configure Key Scan Mode for Row/Column Scan */
    keyscan->con0 &= ~KEYSCAN_CON0_KEYSCAN_MODE;
    keyscan->con0 |= keyscan_init_struct->key_mode & KEYSCAN_CON0_KEYSCAN_MODE;

    /* Configure Low Power Mode */
    keyscan->con0 &= ~KEYSCAN_CON0_KEYSCAN_LP;
    keyscan->con0 |= keyscan_init_struct->low_power_mode & KEYSCAN_CON0_KEYSCAN_LP;

    /* Configure Key Valid Wakeup System */
    keyscan->con0 &= ~KEYSCAN_CON0_KEYVALID_WKUP_EN;
    keyscan->con0 |= keyscan_init_struct->wakeup & KEYSCAN_CON0_KEYVALID_WKUP_EN;

    /* Configure Key Scan Software rstn */
    keyscan->con0 &= ~KEYSCAN_CON0_KEYSCAN_SWRSTN;
    keyscan->con0 |= keyscan_init_struct->sw_reset & KEYSCAN_CON0_KEYSCAN_SWRSTN;

    /* Configure Port Mask */
    keyscan->con0 &= ~KEYSCAN_CON0_KEYPORT_EN;
    keyscan->con0 |= (keyscan_init_struct->key_mask * KEYSCAN_CON0_KEYPORT_EN_0) & KEYSCAN_CON0_KEYPORT_EN;

    /* Configure PU */
    keyscan->con0 &= ~KEYSCAN_CON0_PU_SEL;
    keyscan->con0 |= keyscan_init_struct->pu_sel & KEYSCAN_CON0_PU_SEL;

    /* Configure PD */
    keyscan->con0 &= ~KEYSCAN_CON0_PD_SEL;
    keyscan->con0 |= keyscan_init_struct->pu_sel & KEYSCAN_CON0_PD_SEL;

    /* Configure IO Group */
    keyscan->con0 &= ~KEYSCAN_CON0_IOGROUP_SEL;
    keyscan->con0 |= keyscan_init_struct->io_group & KEYSCAN_CON0_IOGROUP_SEL;
}

void keyscan_deinit(keyscan_typedef * keyscan)
{
    keyscan->con0 = 0;
    keyscan->con1 = 0;

    /* The clock is not unique to this module */
    //clk_gate0_cmd(CLK_GATE0_LP, CLK_DIS);
}

AT(.com_periph.keyscan.en_ctrl)
void keyscan_cmd(keyscan_typedef * keyscan, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        keyscan->con0 |= KEYSCAN_CON0_KEYSCAN_EN;
    } else {
        keyscan->con0 &= ~KEYSCAN_CON0_KEYSCAN_EN;
    }
}

void keyscan_pic_config(keyscan_typedef * keyscan, isr_t isr, int pr, KEYSCAN_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type == 0) {
        return;
    }

    if (interrupt_type & KEYSCAN_IT_VALID) {
        interrupt_pending_bit |= KEYSCAN_CON0_KEYVALID_INT_EN;
    }

    if (interrupt_type & KEYSCAN_IT_INVALID) {
        interrupt_pending_bit |= KEYSCAN_CON0_KEYINVALID_INT_EN;
    }

    if (state != DISABLE) {
        keyscan->con0 |= interrupt_pending_bit;
        sys_irq_init(IRQn_KEYSCAN_AUBUFx, pr, isr);
    } else {
        keyscan->con0 &= ~interrupt_pending_bit;
    }
}

AT(.com_periph.keyscan.flag)
FLAG_STATE keyscan_get_flag(keyscan_typedef * keyscan, KEYSCAN_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & KEYSCAN_IT_VALID) {
        interrupt_pending_bit |= KEYSCAN_CLR_KEYVALID_PND_CLR;
    }

    if (interrupt_type & KEYSCAN_IT_INVALID) {
        interrupt_pending_bit |= KEYSCAN_CLR_KEYINVALID_PND_CLR;
    }

    if (keyscan->con0 & interrupt_pending_bit) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.keyscan.flag)
void keyscan_clear_flag(keyscan_typedef * keyscan, KEYSCAN_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & KEYSCAN_IT_VALID) {
        interrupt_pending_bit |= KEYSCAN_CON0_KEYVALID_PND;
    }

    if (interrupt_type & KEYSCAN_IT_INVALID) {
        interrupt_pending_bit |= KEYSCAN_CON0_KEYINVALID_PND;
    }

    keyscan->clr |= interrupt_pending_bit;
}

AT(.com_periph.keyscan.query)
uint32_t keyscan_get_groupA_key_id(keyscan_typedef * keyscan)
{
    uint8_t  idx;
    uint8_t  *p;
    uint32_t info_reg[2];
    uint32_t key_val = 0;
    uint32_t key_val_img = 0;

    info_reg[0] = keyscan->info0;
    info_reg[1] = keyscan->info1;
    p = (u8 *)info_reg;

    key_val |= ((info_reg[0] & 0xfe) >> 1) * KA1;
    key_val |= ((info_reg[0] & 0xfc00) >> 10) * KB2;
    key_val |= ((info_reg[0] & 0xf80000) >> 19) * KC3;
    key_val |= ((info_reg[0] & 0xf0000000) >> 28) * KD4;
    key_val |= ((info_reg[1] & 0xe0) >> 5) * KE5;
    key_val |= ((info_reg[1] & 0xc000) >> 14) * KF6;
    key_val |= ((info_reg[1] & 0x800000) >> 23) * KG7;

    if (keyscan->con0 & KEYSCAN_CON0_KEYSCAN_MODE) {
        //Resolve matrix keystroke conflicts
        for (u8 i = 0; i < 8; i++) {
            idx = i - 1;
            for (u8 j = 0; j < i; j++) {
                key_val_img |= (p[i] & (0x01  << j))? (0x01 << idx): 0;
                idx = idx + 6 - j;
            }
        }
    } else {
        key_val_img = 0xffffffff;
    }

    return key_val & key_val_img;
}

AT(.com_periph.keyscan.query)
uint32_t keyscan_get_groupB_key_id(keyscan_typedef * keyscan)
{
    uint32_t info2_reg;
    uint32_t key_val = 0;

    info2_reg = keyscan->info2;

    key_val |= (info2_reg & 0xff) * KJ0;
    key_val |= ((info2_reg & 0xff00) >> 8) * KI0;

    return key_val;
}

AT(.com_periph.keyscan.lowpwr_mode)
bool keyscan_is_lowpwr_mode(keyscan_typedef * keyscan)
{
    bool mode = (keyscan->con0 & KEYSCAN_CON0_KEYSCAN_LP) >> 2;

    return mode;
}


/*
 * @File name    : driver_tmr.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-20
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the TMR peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#include "driver_tmr.h"

void tmr_base_init(tmr_typedef *tmrx, tmr_base_init_typedef *tmr_base_init_struct)
{
    uint32_t tmp_reg = tmrx->con;
    //---------- tmr controller register ----------//
    tmp_reg &= ~(uint32_t)TMRxCON_PCLKSEL;
    tmp_reg |= (uint32_t)tmr_base_init_struct->clock_source;

    tmp_reg &= ~(uint32_t)TMRxCON_PDIVSEL;
    tmp_reg |= (uint32_t)tmr_base_init_struct->clock_div;

    if (tmrx == TMR2_REG || tmrx == TMR3_REG) {
        tmp_reg &= ~(uint32_t)TMRxCON_MODE_SEL;
        tmp_reg |= (uint32_t)tmr_base_init_struct->mode;
    }

    tmrx->con = tmp_reg;

    tmrx->cnt = 0;

    tmrx->period = (uint32_t)(tmr_base_init_struct->period);
}


void tmr_deinit(tmr_typedef *tmrx)
{
    tmrx->con = 0;

    if (tmrx == TMR0_REG) {
        clk_gate0_cmd(CLK_GATE0_TMR0, CLK_DIS);
    } else if (tmrx == TMR1_REG) {
        clk_gate0_cmd(CLK_GATE0_TMR1, CLK_DIS);
    } else if (tmrx == TMR2_REG) {
        clk_gate0_cmd(CLK_GATE0_TMR2, CLK_DIS);
    } else if (tmrx == TMR3_REG) {
        clk_gate1_cmd(CLK_GATE1_TMR3, CLK_DIS);
    }
}

AT(.com_periph.tmr.get_cnt)
void tmr_set_counter(tmr_typedef *tmrx, uint32_t cnt)
{
    tmrx->cnt = cnt;
}


AT(.com_periph.tmr.set_period)
void tmr_set_period(tmr_typedef *tmrx, uint32_t period)
{
    tmrx->period = period;
}


AT(.com_periph.tmr.get_cnt)
uint32_t tmr_get_counter(tmr_typedef *tmrx)
{
    return (uint32_t)(tmrx->cnt);
}


AT(.com_periph.tmr.get_period)
uint32_t tmr_get_period(tmr_typedef *tmrx)
{
    return (uint32_t)(tmrx->period);
}


AT(.com_periph.tmr.cmd)
void tmr_cmd(tmr_typedef *tmrx, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        tmrx->con |= TMRxCON_TMREN;
    } else {
        tmrx->con &= (uint32_t)(~TMRxCON_TMREN);
    }
}

AT(.com_periph.tmr.get_cap)
uint32_t tmr_get_capture(tmr_typedef *tmrx)
{
    if (tmrx == TMR0_REG || tmrx == TMR1_REG) {
        return ~((uint32_t)0);
    }
    return (uint32_t)(tmrx->capture_val);
}

void tmr_capture_config(tmr_typedef *tmrx, TMR_MODE_TYPEDEF edge_select)
{
    if (tmrx == TMR2_REG || tmrx == TMR3_REG) {
        uint32_t tmp_reg = tmrx->con;
        tmrx->con &= ~(uint32_t)TMRxCON_MODE_SEL;
        tmrx->con |= (uint32_t)edge_select;
        tmrx->con = tmp_reg;
    }
}


void tmr_pic_config(tmr_typedef *tmrx, isr_t isr, int pr, TMR_IT_TYPEDEF interrup_type, FUNCTIONAL_STATE state)
{
    uint8_t irq_vector;
    uint32_t tmr_interrupt_bit = 0;
    uint32_t all_interrupt_type_mask = TMR_IT_UPDATE | TMR_IT_CAPTURE;

    if (interrup_type == 0) {
        return;
    }

    if (tmrx == TMR0_REG || tmrx == TMR1_REG) {
        if (interrup_type & TMR_IT_CAPTURE) {
            interrup_type &= ~TMR_IT_CAPTURE;
        }
    }

    /*--- Get interrupt vector ---*/
    if (tmrx == TMR0_REG) {
        irq_vector = IRQn_TMR0;
    }else if (tmrx == TMR1_REG) {
        irq_vector = IRQn_TMR1;
    } else if (tmrx == TMR2_REG) {
        irq_vector = IRQn_TMR2;
    } else if (tmrx == TMR3_REG) {
        irq_vector = IRQn_TMR3;
    } else {
        return;
    }

    /*--- Get interrupt ctrl bit ---*/
    if (interrup_type & TMR_IT_UPDATE) {
        tmr_interrupt_bit |= TMRxCON_TIE;
    }

    if (interrup_type & TMR_IT_CAPTURE) {
        tmr_interrupt_bit |= TMRxCON_CPT_IE;
    }

    /*--- Execute configured ---*/
    if (state != DISABLE) {
        tmrx->con |= tmr_interrupt_bit;
        sys_irq_init(irq_vector, pr, isr);
    } else {
        tmrx->con &= ~tmr_interrupt_bit;
        if (interrup_type == all_interrupt_type_mask) {
            PICENCLR = BIT(irq_vector);
        }
    }
}


AT(.com_periph.tmr.get)
FLAG_STATE tmr_get_flag(tmr_typedef *tmrx, TMR_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & TMR_IT_CAPTURE) {
        if (tmrx == TMR0_REG || tmrx == TMR1_REG) {
            return RESET;
        } else {
            interrupt_pending_bit |= TMRxCON_CPTPND;
        }
    }

    if (interrupt_type & TMR_IT_UPDATE) {
        interrupt_pending_bit |= TMRxCON_TPND;
    }

    if ((tmrx->con & interrupt_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.tmr.clear)
void tmr_clear_flag(tmr_typedef *tmrx, TMR_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & TMR_IT_UPDATE) {
        interrupt_pending_bit |= TMRxCPND_TPCLR;
    }

    if (interrupt_type & TMR_IT_CAPTURE) {
        if (tmrx == TMR2_REG || tmrx == TMR3_REG) {
            interrupt_pending_bit |= TMRxCPND_CPTPCLR;
        }
    }

    tmrx->cpnd |= interrupt_pending_bit;
}

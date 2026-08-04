/*
 * @File name    : driver_ledc.c
 * @Author       : Bluetrum IOT Team.
 * @Date         : 2025-09-10
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the LEDC peripheral.
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_ledc.h"


void ledc_init(ledc_typedef *ledc, ledc_init_typedef *ledc_timing_init_struct)
{
    u32 tmp_reg;

    /* Load the register value to variable */
    tmp_reg = ledc->con;

    /* Clear value need to configure */
    tmp_reg &= ~(uint32_t)(LEDC_CON_OUT_INV | \
                           LEDC_CON_BAUD_MASK | \
                           LEDC_CON_DELAY_MASK | \
                           LEDC_CON_IN_FORMAT | \
                           LEDC_CON_IDLE_LEVEL | \
                           LEDC_CON_VLD_INV);

    /* Config output data all invert */
    tmp_reg |= ledc_timing_init_struct->output_invert & LEDC_CON_OUT_INV;
    /* Config input format */
    tmp_reg |= ledc_timing_init_struct->format & LEDC_CON_IN_FORMAT;
    /* Config idle level */
    tmp_reg |= ledc_timing_init_struct->idle_level & LEDC_CON_IDLE_LEVEL;
    /* Config valid data direct or invert */
    tmp_reg |= ledc_timing_init_struct->valid_data_mode & LEDC_CON_VLD_INV;
    tmp_reg &= ~LEDC_CON_BYTE_INV;
    tmp_reg |= ledc_timing_init_struct->byte_inv_number & LEDC_CON_BYTE_INV;

    /* Restore the value to register */
    ledc->con = tmp_reg;
}


void ledc_timing_init(ledc_typedef *ledc, ledc_timing_init_typedef *ledc_timing_init_struct)
{
    u32 tmp_reg;
    ledc_timing_init_typedef *p = ledc_timing_init_struct;

    /* Load the register value to variable */
    tmp_reg = ledc->con;
    /* Config baud */
    tmp_reg |= (ledc_timing_init_struct->baud << LEDC_CON_BAUD_LSB) & LEDC_CON_BAUD_MASK;
    /* Config delay */
    tmp_reg |= (ledc_timing_init_struct->delay << LEDC_CON_DELAY_LSB) & LEDC_CON_DELAY_MASK;
    /* Restore the value to register */
    ledc->con = tmp_reg;

    /* Config code element timing */
    tmp_reg = 0;
    if ((p->code_1_high <= p->baud) && (p->code_0_high <= p->baud)) {
        tmp_reg = (p->code_1_high << LEDC_TIX_T1H_LSB) | \
                  ((p->baud - p->code_1_high) << LEDC_TIX_T1L_LSB) | \
                  (p->code_0_high << LEDC_TIX_T0H_LSB) | \
                  ((p->baud - p->code_0_high));
    }
    ledc->tix = tmp_reg;

    /* Config reset timing */
    tmp_reg = (p->reset_low << LEDC_RSTX_RSTL_LSB) | p->reset_high;
    ledc->rstx = tmp_reg;
}


void ledc_deinit(ledc_typedef *ledc)
{
    ledc->con &= ~(LEDC_CON_DMA_INT_EN | \
                   LEDC_CON_RST_INT_EN | \
                   LEDC_CON_INT_EN | \
                   LEDC_CON_EN);
    clk_gate2_cmd(CLK_GATE2_LEDC, CLK_DIS);
}


AT(.com_text.ledc.cmd)
void ledc_cmd(ledc_typedef *ledc, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        ledc->con |= LEDC_CON_EN;
    } else {
        ledc->con &= ~LEDC_CON_EN;
    }
}


AT(.com_periph.ledc.transport)
void ledc_dma_kick(ledc_typedef *ledc, uint32_t addr, uint16_t len)
{
    ledc->addr = addr;
    ledc->cnt = LEDC_CNT_DMA_KST | len;
}


AT(.com_periph.ledc.transport)
void ledc_kick(ledc_typedef *ledc)
{
    ledc->con |= LEDC_CON_LEDC_KICK;
}


void ledc_pic_config(ledc_typedef *ledc, isr_t isr, int pr, LEDC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_pending_bit = 0;

    if (flag_type == 0) {
        return;
    }

    if (flag_type & LEDC_FLAG_DONE) {
        flag_pending_bit |= LEDC_CON_INT_EN;
    }

    if (flag_type & LEDC_FLAG_RST) {
        flag_pending_bit |= LEDC_CON_RST_INT_EN;
    }

    if (flag_type & LEDC_FLAG_DMA) {
        flag_pending_bit |= LEDC_CON_DMA_INT_EN;
    }

    if (state != DISABLE) {
        ledc->con |= flag_pending_bit;
        sys_irq_init(IRQn_IR_QDEC_LEDC, pr, isr);
    } else {
        ledc->con &= ~flag_pending_bit;
    }
}


AT(.com_periph.ledc.flag)
FLAG_STATE ledc_get_flag(ledc_typedef *ledc, LEDC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & LEDC_FLAG_DONE) {
        flag_pending_bit |= LEDC_PEND_LEDC_PND;
    }

    if (flag_type & LEDC_FLAG_RST) {
        flag_pending_bit |= LEDC_PEND_RST_PND;
    }

    if (flag_type & LEDC_FLAG_DMA) {
        flag_pending_bit |= LEDC_PEND_DMA_PND;
    }

    if ((ledc->pend & flag_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}


AT(.com_periph.ledc.flag)
void ledc_clear_flag(ledc_typedef *ledc, LEDC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & LEDC_FLAG_DONE) {
        flag_pending_bit |= LEDC_PEND_LEDC_PND;
    }

    if (flag_type & LEDC_FLAG_RST) {
        flag_pending_bit |= LEDC_PEND_RST_PND;
    }

    if (flag_type & LEDC_FLAG_DMA) {
        flag_pending_bit |= LEDC_PEND_DMA_PND;
    }

    ledc->pend = flag_pending_bit;
}


void ledc_set_frame_length(ledc_typedef *ledc, uint16_t frame_length)
{
    ledc->fd = frame_length & 0xfff;
}


void ledc_set_loop_number(ledc_typedef *ledc, uint16_t loop_n)
{
    ledc->lp = loop_n & 0x7ff;
}


uint16_t ledc_get_frame_length(ledc_typedef *ledc)
{
    return (ledc->fd & 0xfff);
}


uint16_t ledc_get_loop_number(ledc_typedef *ledc)
{
    return (ledc->lp & 0x7ff);
}

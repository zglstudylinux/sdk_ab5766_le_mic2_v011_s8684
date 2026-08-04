/*
 * @File name    : driver_irrx.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-11
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the IRRX peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#include "driver_ir.h"


void irrx_init(irrx_typedef *irrx_reg, irrx_init_typedef *irrx_param)
{
    //rx con
    irrx_reg->con &= ~IRRX_CON_IRWKEN;

    irrx_reg->con &= ~IRRX_CON_IRRXSEL;
    irrx_reg->con |= irrx_param->data_bits;

    irrx_filter_init(&irrx_param->filter);

    //timing
    u32 irrx_clk = clk_irrx_clk_get(CLK_VALUE_MODE_FREQ);

    irrx_reg->period0   = ((uint16_t)((uint64_t)RPTPR_CNT   * irrx_clk/1000000) << IRRX_PR0_RPTPR_LSB)|
                                ((uint16_t)((uint64_t)DATPR_CNT   * irrx_clk/1000000) << IRRX_PR0_DATPR_LSB);

    irrx_reg->period1   = ((uint16_t)((uint64_t)DATA_1_CNT  * irrx_clk/1000000) << IRRX_PR1_ONEPR_LSB) |
                                ((uint16_t)((uint64_t)DATA_0_CNT  * irrx_clk/1000000) << IRRX_PR1_ZEROPR_LSB);

    irrx_reg->err0      = ((uint16_t)((uint64_t)RPTERR_CNT  * irrx_clk/1000000) << IRRX_ERR0_RPTPR_LSB)|
                                ((uint16_t)((uint64_t)DATERR_CNT  * irrx_clk/1000000) << IRRX_ERR0_DATPR_LSB);

    irrx_reg->err1      = ((uint16_t)((uint64_t)TOPR_CNT    * irrx_clk/1000000) << IRRX_ERR1_TOPR_LSB) |
                            ((uint16_t)((uint64_t)ONEERR_CNT  * irrx_clk/1000000) << IRRX_ERR1_ONEPR_LSB) |
                            ((uint16_t)((uint64_t)ZEROERR_CNT * irrx_clk/1000000) << IRRX_ERR1_ZEROPR_LSB);
}

void irrx_filter_init(const ir_filter_config_typedef *filter_param)
{
    IRFLTCON &= ~IRFLT_CON_SRCS;
    IRFLTCON |= filter_param->filter_src;

    IRFLTCON &= ~IRFLT_CON_LEN_MASK;
    IRFLTCON |= filter_param->filter_len << IRFLT_CON_LEN_LSB;

    IRFLTCON &= ~IRFLT_CON_EN_BIT;
    IRFLTCON |= (filter_param->filter_en << IRFLT_CON_EN_POS);
}

void irrx_pic_config(irrx_typedef *irrx_reg, isr_t isr, int pr, IRRX_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_pending_bit = 0;

    if (flag_type == 0) {
        return;
    }

    if (flag_type & IRRX_FLAG_RXPND) {
        flag_pending_bit |= IRRX_CON_IRIE;
    }

    if (state != DISABLE) {
        irrx_reg->con |= flag_pending_bit;
        sys_irq_init(IRQn_IR_QDEC_LEDC, pr, isr);
    } else {
        irrx_reg->con &= ~flag_pending_bit;
    }
}

AT(.com_periph.irrx.flag)
FLAG_STATE irrx_get_flag(irrx_typedef *irrx_reg, IRRX_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & IRRX_FLAG_RXPND) {
        flag_pending_bit |= IRRX_CON_RXPND;
    } else if (flag_type & IRRX_FLAG_KEYRELS) {
        flag_pending_bit |= IRRX_CON_KEYRELS;
    }

    if ((irrx_reg->con & flag_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.irrx.flag)
void irrx_clear_flag(irrx_typedef *irrx_reg, IRRX_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & IRRX_FLAG_RXPND) {
        flag_pending_bit |= IRRX_CPND_CRXPND;
    }

    if (flag_type & IRRX_FLAG_KEYRELS) {
        flag_pending_bit |= IRRX_CPND_CRELSPND;
    }

    irrx_reg->cpnd = flag_pending_bit;
}

AT(.com_periph.irrx.data)
uint32_t irrx_get_data(irrx_typedef *irrx_reg)
{
    return irrx_reg->rxdata;
}

void irrx_cmd(irrx_typedef *irrx_reg, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        IRRX_REG->con |= IRRX_CON_IREN;
    } else {
        IRRX_REG->con &= ~IRRX_CON_IREN;
    }
}

void irrx_deinit(irrx_typedef *irrx_reg)
{
    irrx_reg->con &= ~(IRRX_CON_IRIE | \
                       IRRX_CON_IRWKEN | \
                       IRRX_CON_IREN);

    clk_gate1_cmd(CLK_GATE1_IRRX, CLK_DIS);
    clk_gate1_cmd(CLK_GATE1_IRTX, CLK_DIS);
}

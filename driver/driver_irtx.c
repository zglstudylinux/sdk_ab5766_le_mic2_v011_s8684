/*
 * @File name    : driver_irtx.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-29
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the IRTX peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#include "driver_ir.h"
#include "driver_gpio.h"


const irtx_timing_typedef Nec        = IRTX_TIMING_NEC;
const irtx_timing_typedef Tc9012     = IRTX_TIMING_TC9012;
const irtx_timing_typedef Rc5        = IRTX_TIMING_RC5;
const irtx_timing_typedef Rc6        = IRTX_TIMING_RC6;

void irtx_timing_set(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param, const irtx_timing_typedef timing)
{
    uint16_t prescale = 1; //1M

    irtx_reg->con &= ~ IRTX_CON_IRTX_SEL_MASK;
    irtx_reg->con |= irtx_param->encode_format;

    irtx_reg->con &= ~(IRTX_CON_IRTX_1MODE_BIT | IRTX_CON_IRTX_0MODE_BIT);
    irtx_reg->con |= (timing.code_1_mode << IRTX_CON_IRTX_1MODE_POS) | (timing.code_0_mode << IRTX_CON_IRTX_0MODE_POS);


    irtx_reg->onetime = ((uint16_t)(timing.code_1_mask * prescale) << IRTX_ONETIME_ONE_MARK_LSB) |
                        (uint16_t)(timing.code_1_space * prescale << IRTX_ONETIME_ONE_SPACE_LSB);

    irtx_reg->zerotime = ((uint16_t)(timing.code_0_mask * prescale) << IRTX_ZEROTIME_ZERO_MARK_LSB) |
                        (uint16_t)(timing.code_0_space * prescale << IRTX_ZEROTIME_ZERO_SPACE_LSB);

    irtx_reg->starttime = ((uint16_t)(timing.start_1_time * prescale) << IRTX_STARTTIME_START1TIME_LSB) |
                        (uint16_t)(timing.start_0_time * prescale << IRTX_STARTTIME_START0TIME_LSB);

    irtx_reg->repeattime = ((uint16_t)(timing.repeat_1_time * prescale) << IRTX_STARTTIME_REPEAT1TIME_LSB) |
                        (uint16_t)(timing.repeat_0_time * prescale << IRTX_STARTTIME_REPEAT0TIME_LSB);

    irtx_reg->repeatcon = ((uint32_t)(timing.repeat_interval * prescale) << IRTX_IRREPEATCON_IRREPTIME_LSB) |
                        ((uint16_t)(timing.end_pulse_time * prescale) << IRTX_IRREPEATCON_IRENDTIME_LSB) |
                        (timing.sync_code << IRTX_IRREPEATCON_IRTX_SYC_POS) |
                        (timing.repeat_mode << IRTX_IRREPEATCON_IRREP_SEL_POS);

    irtx_reg->txlen = timing.tx_len;
}

void irtx_timing_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param)
{
    switch(irtx_param->encode_format)
    {
        case IRTX_ENCODE_FORMAT_NEC:
            irtx_timing_set(irtx_reg, irtx_param, Nec);
            break;

        case IRTX_ENCODE_FORMAT_TC9012:
            irtx_timing_set(irtx_reg, irtx_param, Tc9012);
            break;

        case IRTX_ENCODE_FORMAT_RC5:
            irtx_timing_set(irtx_reg, irtx_param, Rc5);
            break;

        case IRTX_ENCODE_FORMAT_RC6:
            irtx_timing_set(irtx_reg, irtx_param, Rc6);
            break;

        default:
            break;
    }
}

void irtx_carrier_init(irtx_typedef *irtx_reg, const irtx_carrier_config_typedef *irtx_carrier_param)
{
    clk_irtx_mod_clk_set(irtx_carrier_param->mod_clksel);

    irtx_reg->con &= ~ IRTX_CON_IRCW_DUTY_MASK;
    irtx_reg->con |= irtx_carrier_param->ircw_duty << IRTX_CON_IRCW_DUTY_LSB;

    irtx_reg->con &= ~ IRTX_CON_IRCW_LENGTH_MASK;
    irtx_reg->con |= irtx_carrier_param->ircw_length << IRTX_CON_IRCW_LENGTH_LSB;

    irtx_reg->con &= ~ IRTX_CON_IRTX_CARRIER_EN_BIT;
    irtx_reg->con |= (irtx_carrier_param->carrier_en << IRTX_CON_IRTX_CARRIER_EN_POS);
}

void ir_capture_init(irtx_typedef *irtx_reg, const ir_capture_config_typedef *irtx_capture_param)
{
    irtx_reg->con &= ~IRTX_CON_IREDGE_SEL;
    irtx_reg->con |= irtx_capture_param->capture_edge;

    irtx_reg->dma_osize &= ~IRDMAOSIZE_DMA_OSIZE;
    irtx_reg->dma_osize = (irtx_capture_param->cap_dma_size - 1);

    irtx_reg->dma_oadr = irtx_capture_param->cap_dma_addr;

    irtx_reg->dma_con &= ~IRDMACON_ODMA_EN_BIT;
    irtx_reg->dma_con |= (irtx_capture_param->cap_dma_en << IRDMACON_ODMA_EN_POS);

    irtx_reg->con &= ~IRTX_CON_IR_CAPEN_BIT;
    irtx_reg->con |= (irtx_capture_param->capture_en << IRTX_CON_IR_CAPEN_POS);
}

void irtx_hw_enc_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param)
{
    irtx_timing_init(irtx_reg, irtx_param);

    irtx_carrier_init(irtx_reg, &irtx_param->carrier);
}

void irtx_learn_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param)
{
    irtx_carrier_init(irtx_reg, &irtx_param->carrier);
}

void irtx_learn_cmd(irtx_typedef *irtx_reg, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        irtx_reg->con |= IRTX_CON_IRTXLEN_EN;
    } else {
        irtx_reg->con &= (uint32_t)(~IRTX_CON_IRTXLEN_EN);
    }
}

void irtx_cmd(irtx_typedef *irtx_reg, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        irtx_reg->con |= IRTX_CON_IRTXMOD_EN;
    } else {
        irtx_reg->con &= (uint32_t)(~IRTX_CON_IRTXMOD_EN);
    }
}

void irtx_set_repeat(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en)
{
    irtx_reg->repeatcon = (irtx_reg->repeatcon & ~IRTX_IRREPEATCON_IRTX_KEY) | (IRTX_IRREPEATCON_IRTX_KEY * en);
}

bool irtx_is_repeat(irtx_typedef *irtx_reg)
{
    if (irtx_reg->repeatcon & IRTX_IRREPEATCON_IRTX_KEY) {
        return 1;
    } else {
        return 0;
    }
}

void irtx_set_invert(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en)
{
    irtx_reg->con = (irtx_reg->con & ~IRTX_CON_IRTX_INV) | (IRTX_CON_IRTX_INV * en);
}

bool irtx_is_invert(irtx_typedef *irtx_reg)
{
    if (irtx_reg->con & IRTX_CON_IRTX_INV) {
        return 1;
    } else {
        return 0;
    }
}

void irtx_set_syc(irtx_typedef *irtx_reg, bool syc)
{
    irtx_reg->repeatcon = (irtx_reg->repeatcon & ~IRTX_IRREPEATCON_IRTX_SYC_BIT) | (syc << IRTX_IRREPEATCON_IRTX_SYC_POS);
}

bool irtx_is_syc(irtx_typedef *irtx_reg)
{
    if (irtx_reg->repeatcon & IRTX_IRREPEATCON_IRTX_SYC_BIT) {
        return 1;
    } else {
        return 0;
    }
}

uint16_t irtx_get_capture_size(irtx_typedef *irtx_reg)
{
    return irtx_reg->dma_osize >> 16;
}

AT(.com_periph.set_data)
void irtx_set_data (irtx_typedef *irtx_reg, uint32_t code)
{
    irtx_reg->txdata = code;
}

AT(.com_periph.kick)
void irtx_learn_kick(irtx_typedef *irtx_reg, void *buf, uint32_t len, FUNCTIONAL_STATE en)
{
    irtx_reg->dma_iadr = (uint32_t)buf;
    irtx_reg->dma_isize = len - 1;
    irtx_reg->dma_con |= (irtx_reg->dma_con & ~IRDMACON_IDMA_EN_BIT) | (en << IRDMACON_IDMA_EN_POS);
    irtx_reg->con = (irtx_reg->con & ~IRTX_CON_IRTX_KST_BIT) | (en << IRTX_CON_IRTX_KST_POS);
}

AT(.com_periph.kick)
void irtx_hw_enc_kick(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en)
{
    irtx_reg->con = (irtx_reg->con & ~IRTX_CON_IRTX_KST_BIT) | (en << IRTX_CON_IRTX_KST_POS);
}

void irtx_kick_wait(irtx_typedef *irtx_reg)
{
    while (!(irtx_reg->pend & IRTX_IRTX_PND));
    irtx_reg->pend |= IRTX_IRTX_PND;
}

void irtx_pic_config(irtx_typedef *irtx_reg, isr_t isr, int pr, IRTX_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_tx_bit = 0;
    u32 flag_dma_bit = 0;

    if (flag_type == 0) {
        return;
    }

    if (flag_type & IRTX_FLAG_TX_PND) {
        flag_tx_bit |= IRTX_CON_IRTX_IE;
    }

    if (flag_type & IRTX_FLAG_ODMA_ALL_PND) {
        flag_dma_bit |= IRDMACON_ODMA_APND_IE;
    }

    if (flag_type & IRTX_FLAG_ODMA_HALF_PND) {
        flag_dma_bit |= IRDMACON_ODMA_HPND_IE;
    }

    if (flag_type & IRTX_FLAG_IDMA_ALL_PND) {
        flag_dma_bit |= IRDMACON_IDMA_APND_IE;
    }

    if (flag_type & IRTX_FLAG_IDMA_HALF_PND) {
        flag_dma_bit |= IRDMACON_IDMA_HPND_IE;
    }

    if (state != DISABLE) {
        if (flag_tx_bit) {
            irtx_reg->con |= flag_tx_bit;
        }
        if (flag_dma_bit) {
            irtx_reg->dma_con |= flag_dma_bit;
        }
        sys_irq_init(IRQn_IR_QDEC_LEDC, pr, isr);
    } else {
        if (flag_tx_bit) {
            irtx_reg->con &= ~flag_tx_bit;
        }
        if (flag_dma_bit) {
            irtx_reg->dma_con &= ~flag_dma_bit;
        }
    }
}

AT(.com_periph.irtx.flag)
FLAG_STATE irtx_get_flag(irtx_typedef *irtx_reg, IRTX_FLAG_TYPEDEF flag_type)
{
    u32 flag_tx_bit = 0;
    u32 flag_dma_bit = 0;

    switch (flag_type) {
        case IRTX_FLAG_TX_PND:
            flag_tx_bit |= IRTX_IRTX_PND;
            break;

        case IRTX_FLAG_ODMA_ALL_PND:
            flag_dma_bit |= IRDMAPEND_ODMA_APND;
            break;

        case IRTX_FLAG_ODMA_HALF_PND:
            flag_dma_bit |= IRDMAPEND_ODMA_HPND;
            break;

        case IRTX_FLAG_IDMA_ALL_PND:
            flag_dma_bit |= IRDMAPEND_IDMA_APND;
            break;

        case IRTX_FLAG_IDMA_HALF_PND:
            flag_dma_bit |= IRDMAPEND_IDMA_HPND;
            break;

        default:
            break;
    }

    if (flag_tx_bit && (irtx_reg->pend & flag_tx_bit)) {
        return SET;
    } else if (flag_dma_bit && (irtx_reg->dma_pend & flag_dma_bit)) {
        return SET;
    } else {
        return RESET;
    }
}


AT(.com_periph.irtx.flag)
void irtx_clear_flag(irtx_typedef *irtx_reg, IRTX_FLAG_TYPEDEF flag_type)
{
    u32 flag_tx_bit = 0;
    u32 flag_dma_bit = 0;

    if (flag_type & IRTX_FLAG_TX_PND) {
        flag_tx_bit |= IRTX_IRTX_PND;
    }

    if (flag_type & IRTX_FLAG_ODMA_ALL_PND) {
        flag_dma_bit |= IRDMAPEND_ODMA_APND;
    }

    if (flag_type & IRTX_FLAG_ODMA_HALF_PND) {
        flag_dma_bit |= IRDMAPEND_ODMA_HPND;
    }

    if (flag_type & IRTX_FLAG_IDMA_ALL_PND) {
        flag_dma_bit |= IRDMAPEND_IDMA_APND;
    }

    if (flag_type & IRTX_FLAG_IDMA_HALF_PND) {
        flag_dma_bit |= IRDMAPEND_IDMA_HPND;
    }

    if (flag_tx_bit) {
        irtx_reg->pend |= flag_tx_bit;
    }

    if (flag_dma_bit) {
        irtx_reg->dma_pend |= flag_dma_bit;
    }
}

void irtx_deinit(irtx_typedef *irtx_reg)
{
    irtx_reg->con = 0;

    irtx_reg->dma_con = 0;


    clk_gate1_cmd(CLK_GATE1_IRTX, CLK_DIS);
    clk_gate2_cmd(CLK_GATE2_IRFLT, CLK_DIS);
}

/*
 * @File name    : driver_qdec.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-01
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the QDEC peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#include "driver_qdec.h"

void qdec_init(qdec_typedef *qdec, qdec_init_typedef *qdec_init_struct)
{
    //---------- qdec controller register ----------//
    qdec->con &= ~(uint32_t)QDECxCON_TIME_MODE;
    qdec->con |= ((qdec_init_struct->tm_en) << 15) & QDECxCON_TIME_MODE;

    qdec->con &= ~(uint32_t)QDECxCON_CM;
    qdec->con |= ((qdec_init_struct->cm_sel) << 12) & QDECxCON_CM;

    qdec->con &= ~(uint32_t)QDECxCON_WKUP_EN;
    qdec->con |= ((qdec_init_struct->wakup_en) << 4) & QDECxCON_WKUP_EN;

    if (QDEC_TM_ENABLE == qdec_init_struct->tm_en) {
        qdec->con &= ~(uint32_t)QDECxCON_SMP;
        qdec->con |= ((qdec_init_struct->smp_num_sel) << 16) & QDECxCON_SMP;
    }

    qdec->con &= ~(uint32_t)QDECxCON_FIL_LEN;
    qdec->con |= ((qdec_init_struct->fil_len) << 5) & QDECxCON_FIL_LEN;
}

void qdec_deinit(qdec_typedef *qdec)
{
    qdec->con = 0;

    if (qdec == QDECX_REG) {
        clk_gate1_cmd(CLK_GATE1_QDECX, CLK_DIS);
    }
}

AT(.com_periph.qdec.cmd)
void qdec_cmd(qdec_typedef *qdec, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        qdec->con |= QDECCON_EN;
    } else {
        qdec->con &= (uint32_t)(~QDECCON_EN);
    }
}

AT(.com_periph.qdec.get_data)
uint32_t qdec_get_data(qdec_typedef *qdec)
{
    return (uint32_t)(qdec->dat);
}

AT(.com_periph.qdec.clr_data)
void qdec_clear_data(qdec_typedef *qdec)
{
    qdec->con |= QDECCON_DAT_CLR;
}

void qdec_pic_config(qdec_typedef *qdec, isr_t isr, int pr, QDEC_IT_TYPEDEF interrup_type, FUNCTIONAL_STATE state)
{
    uint8_t irq_vector;
    uint32_t qdec_interrupt_bit = 0;
    uint32_t all_interrupt_type_mask = QDEC_IT_SAMPLE_COUNT | QDEC_IT_REVERSE_DIR | QDEC_IT_FORWARD_DIR;

    if (interrup_type == 0) {
        return;
    }

    /*--- Get interrupt vector ---*/
    irq_vector = IRQn_IR_QDEC_LEDC;

    /*--- Get interrupt ctrl bit ---*/
    if (interrup_type & QDEC_IT_SAMPLE_COUNT) {
        qdec_interrupt_bit |= QDECCON_SMPIE;
    }

    if (interrup_type & QDEC_IT_REVERSE_DIR) {
        qdec_interrupt_bit |= QDECCON_RDIE;
    }

    if (interrup_type & QDEC_IT_FORWARD_DIR) {
        qdec_interrupt_bit |= QDECCON_FDIE;
    }
    /*--- Execute configured ---*/
    if (state != DISABLE) {
        qdec->con |= qdec_interrupt_bit;
        sys_irq_init(irq_vector, pr, isr);
    } else {
        qdec->con &= ~qdec_interrupt_bit;
        if (interrup_type == all_interrupt_type_mask) {
            PICEN &= ~BIT(irq_vector);
        }
    }
}

AT(.com_periph.qdec.get)
FLAG_STATE qdec_get_flag(qdec_typedef *qdec, QDEC_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_bit = 0;

    if (interrupt_type & QDEC_REVERSE_DIR_PND) {
        interrupt_bit |= QDECCON_REVERSE_PND;
    }
    if (interrupt_type & QDEC_FORWARD_DIR_PND) {
        interrupt_bit |= QDECCON_FORWARD_PND;
    }
    if (interrupt_type & QDEC_OV_PND) {
        interrupt_bit |= QDECCON_OV_PND;
    }
    if (interrupt_type & QDEC_SMP_PND) {
        interrupt_bit |= QDECCON_SMP_PND;
    }
    if (interrupt_type & QDEC_TERR_PND) {
        interrupt_bit |= QDECCON_TERR_PND;
    }
    
    if ((qdec->con & interrupt_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.qdec.clear)
void qdec_clear_flag(qdec_typedef *qdec, QDEC_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_bit = 0;

    if (interrupt_type & QDEC_REVERSE_DIR_PND) {
        interrupt_bit |= QDECCON_REVERSE_PND;
    }

    if (interrupt_type & QDEC_FORWARD_DIR_PND) {
        interrupt_bit |= QDECCON_FORWARD_PND;
    }

    if (interrupt_type & QDEC_OV_PND) {
        interrupt_bit |= QDECCON_OV_PND;
    }

    if (interrupt_type & QDEC_SMP_PND) {
        interrupt_bit |= QDECCON_SMP_PND;
    }
    
    if (interrupt_type & QDEC_TERR_PND) {
        interrupt_bit |= QDECCON_TERR_PND;
    }

    qdec->cpnd |= interrupt_bit;
}


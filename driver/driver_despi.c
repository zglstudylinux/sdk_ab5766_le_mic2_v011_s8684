#include "include.h"
#include "driver_despi.h"

/*
 * @File name    : driver_despi.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-09
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the DESPI peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_despi.h"

void despi_init(despi_typedef* despi, despi_init_typedef* despi_init_struct)
{
    uint32_t temp = 0;

    /* DESPI_CLK freq configuration */
    despi->baud = sys_clk_nhz_get() / despi_init_struct->baud - 1;

    temp |= (uint32_t)(despi_init_struct->bus_mode & DESPICON_BUSMODE);
    temp |= (uint32_t)(despi_init_struct->color_order & DESPICON_CL_ORD);
    temp |= (uint32_t)(despi_init_struct->role_mode & DESPICON_SPISM);
    temp |= (uint32_t)(despi_init_struct->output_data_edge & DESPICON_SMPS);
    temp |= (uint32_t)(despi_init_struct->sample_clk_edge & DESPICON_SPILOSS);
    temp |= (uint32_t)(despi_init_struct->clkids & DESPICON_CLKIDS);

    despi->con = temp;
}

void despi_deinit(despi_typedef* despi)
{
    if (despi == DESPI_REG) {
        despi->con = 0;
        clk_gate2_cmd(CLK_GATE2_DESPI, CLK_DIS);
    }
}

void despi_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_EN;
    } else {
        despi->con &= (uint32_t)~DESPICON_EN;
    }
}

void despi_dir_sel(despi_typedef* despi, DESPI_DIR_TYPEDEF direction)
{
    if(direction == DESPI_DIR_RX) {
        despi->con |= BIT(4);
    } else {
        despi->con &= ~BIT(4);
    }
}

void despi_it_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_IE;
    } else {
        despi->con &= (uint32_t)~DESPICON_IE;
    }
}

void despi_lfsr_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_SPILF_EN;
    } else {
        despi->con &= (uint32_t)~DESPICON_SPILF_EN;
    }
}

void despi_multi_bit_bus_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_SPIMBEN;
    } else {
        despi->con &= (uint32_t)~DESPICON_SPIMBEN;
    }
}

void despi_hold_rx_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_HOLDENRX;
    } else {
        despi->con &= (uint32_t)~DESPICON_HOLDENRX;
    }
}

void despi_hold_tx_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_HOLDENTX;
    } else {
        despi->con &= (uint32_t)~DESPICON_HOLDENTX;
    }
}

void despi_bus_mode_sel(despi_typedef* despi, DESPI_BUS_MODE_TYPEDEF mode)
{
    despi->con = (despi->con & ~DESPICON_BUSMODE) | mode;
}

void despi_dual_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_DUAL_EN;
    } else {
        despi->con &= (uint32_t)~DESPICON_DUAL_EN;
    }
}

void despi_dcx_reuse_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_DCX_REUSE;
    } else {
        despi->con &= (uint32_t)~DESPICON_DCX_REUSE;
    }
}

void despi_dcx_9bit_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_3W_CTR;
    } else {
        despi->con &= (uint32_t)~DESPICON_3W_CTR;
    }
}

void despi_dcx_value_cmd(despi_typedef* despi, FUNCTIONAL_STATE state)
{
    if (state) {
        despi->con |= (uint32_t)DESPICON_3W_9BIT;
    } else {
        despi->con &= (uint32_t)~DESPICON_3W_9BIT;
    }
}

AT(.com_periph.despi.get)
FLAG_STATE despi_get_flag(despi_typedef* despi, uint32_t despi_flag)
{
    if ((despi->con & despi_flag) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.despi.clear)
void despi_clear_flag(despi_typedef* despi, uint32_t despi_flag)
{
    despi->cpnd |= (uint32_t)despi_flag;
}

void despi_set_dma_cnt(despi_typedef* despi, uint32_t cnt)
{
    despi->dma_cnt = cnt;
}

void despi_set_dma_addr(despi_typedef* despi, uint32_t addr)
{
    despi->dma_adr = (uint32_t)addr;
}

uint8_t despi_receive_data(despi_typedef* despi)
{
    return (uint8_t)despi->buf;
}

void despi_send_data(despi_typedef* despi, uint8_t data)
{
    despi->buf = data;
}

void despi_pic_config(despi_typedef* despi, isr_t isr, int pr, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        sys_irq_init(IRQn_DE_IIS_TDNN, pr, isr);
        despi->con |= DESPICON_IE;
    } else {
        despi->con &= ~DESPICON_IE;
        PICEN &= ~BIT(IRQn_DE_IIS_TDNN);
    }
}

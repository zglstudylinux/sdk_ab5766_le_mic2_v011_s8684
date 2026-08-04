/*
 * @File name    : driver_iis.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-28
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the IIS peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_iis.h"

void iis_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    iis_reg->con0 = (iis_cb->ch_depth_sel << 26) |\
                    (iis_cb->ch_cnt_sel << 23)   |\
                    ((iis_cb->rx_bit_depth-1) << 18) |\
                    ((iis_cb->tx_bit_depth-1) << 13) |\
                    ((iis_cb->word_width-1) << 8)    |\
                    iis_cb->ws_pulse_sel |\
                    iis_cb->tx_dat_src   |\
                    iis_cb->dat_mode     |\
                    iis_cb->role         |\
                    (iis_cb->rx_en << 2) |\
                    (iis_cb->tx_en << 1);

    iis_reg->con1 = (iis_cb->slv_lose_ch_hdl_ws * IISCON1_LOSE_CH_HDL_EN) |\
                    (iis_cb->slv_dat_sync_ws * IISCON1_DAT_SYNC_WS)           |\
                    (iis_cb->rx_bit_rev * IISCON1_RX_BIT_REV)                 |\
                    (iis_cb->tx_bit_rev * IISCON1_TX_BIT_REV)                 |\
                    (iis_cb->slv_di_flt_en * IISCON1_DI_FLT_EN)               |\
                    (iis_cb->slv_ws_flt_en * IISCON1_WS_FLT_EN)               |\
                    (iis_cb->slv_bclk_flt_en * IISCON1_BCLK_FLT_EN)           |\
                    (iis_cb->ws_inv * IISCON1_WS_INV)                         |\
                    (iis_cb->bclk_inv * IISCON1_BCLK_INV)                     |\
                    (iis_cb->mclk_out_en * IISCON1_MCLKO_EN)                  |\
                    (iis_cb->one_wire_dir)                                    |\
                    (iis_cb->one_wire_en * IISCON1_1W_EN);

    if (iis_cb->one_wire_en == DISABLE) {
        iis_reg->con1 |= IISCON1_DI0_EN;
    }
    iis_reg->con1 |= IISCON1_DO0_EN;

    iis_io_map_init(iis_reg, iis_cb);

    if (iis_cb->tx_dat_src == IIS_DAT_SRC_DAC) {
        iis_data_src_init(iis_reg, iis_cb);
    }
}

void iis_io_map_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    if (iis_cb->iomap == IIS_IOMAP_0) {
        GPIOAFEN |= BIT(8) | BIT(0) | BIT(1) | BIT(2) | BIT(3);
        GPIOADE  |= BIT(8) | BIT(0) | BIT(1) | BIT(2) | BIT(3);

        if (iis_cb->one_wire_en == ENABLE) {
            if (iis_cb->one_wire_dir == IIS_ONE_WIRE_OUTPUT) {
                GPIOADIR &= ~BIT(2);
            } else {
                GPIOADIR |= BIT(2);
            }
        } else {
            GPIOADIR &= ~BIT(2);
            GPIOADIR |=  BIT(3);
            FUNI_IIS_DI(4);//DI PA3
        }

        if (iis_cb->role == IIS_ROLE_MASTER) {
            GPIOADIR &= ~(BIT(8) | BIT(0) | BIT(1));
        } else {
            GPIOADIR |= (BIT(8) | BIT(0) | BIT(1));
        }

        FUNO_PA8SEL(3);//MCLK
        FUNO_PA0SEL(3);//LRCLK
        FUNO_PA1SEL(3);//BCLK
        FUNO_PA2SEL(3);//DO
    } else {
        GPIOAFEN |= BIT(8) | BIT(4) | BIT(5) | BIT(6) | BIT(7);
        GPIOADE |= BIT(8) | BIT(4) | BIT(5) | BIT(6) | BIT(7);
        if (iis_cb->one_wire_en == ENABLE) {
            if (iis_cb->one_wire_dir == IIS_ONE_WIRE_OUTPUT) {
                GPIOADIR &= ~BIT(6);
            } else {
                GPIOADIR |= BIT(6);
            }
        } else {
            GPIOADIR &= ~BIT(6);
            GPIOADIR |=  BIT(7);
            FUNI_IIS_DI(8);//DI PA7
        }

        if (iis_cb->role == IIS_ROLE_MASTER) {
            GPIOADIR &= ~(BIT(8) | BIT(4) | BIT(5));
        } else {
            GPIOADIR |= (BIT(8) | BIT(4) | BIT(5));
        }

        FUNO_PA8SEL(3);//MCLK
        FUNO_PA4SEL(3);//LRCLK
        FUNO_PA5SEL(3);//BCLK
        FUNO_PA6SEL(3);//DO
    }
}

void iis_dma_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    if (iis_cb->rx_en == ENABLE) {
        iis_reg->dma_rx_adr0 = (uint32_t)iis_cb->dma_cfg.dma_rxbuf0_ptr;
        iis_reg->dma_rx_adr1 = (uint32_t)iis_cb->dma_cfg.dma_rxbuf1_ptr;
    }

    if (iis_cb->tx_en == ENABLE) {
        iis_reg->dma_tx_adr0 = (uint32_t)iis_cb->dma_cfg.dma_txbuf0_ptr;
        iis_reg->dma_tx_sz0  = IISTXSIZE0_TX_KICK | (iis_cb->dma_cfg.tx_samples - 1);
        iis_reg->dma_tx_adr0 = (uint32_t)iis_cb->dma_cfg.dma_txbuf1_ptr;
        iis_reg->dma_tx_sz0  = IISTXSIZE0_TX_KICK | (iis_cb->dma_cfg.tx_samples - 1);
    }
}

void iis_data_src_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    DACEXTCON = (DACEXTCON &~ (0x7 << 1)) | (iis_cb->dac_out_sel << 1);  //dac src0
    DACEXTCON = (DACEXTCON &~ (0x1 << 4)) | (1 << 4);  //0:96k  1:48k(如果开了src bypass，该输出采样率跟随dac音源采样率)
    DACEXTCON = (DACEXTCON &~ (0x1 << 5)) | (1 << 5);  //wish只支持单声道，此处左右声道数据一样
    DACEXTCON |= BIT(0);  //dac out to iis en
}

void iis_cmd(iis_typedef *iis_reg, FUNCTIONAL_STATE en)
{
    iis_reg->con0 = (iis_reg->con0 & ~IISCON0_IIS_EN) | (en * IISCON0_IIS_EN);
}

AT(.com_periph.iis.dma_tx_kick)
void iis_dma_tx_kick(iis_typedef *iis_reg, iis_init_typedef *iis_cb, uint8_t *buf, uint16_t buf_len)
{
    //dma_tx_adr1是当前发送的buf，新数据要copy到dma_tx_adr0，当前的buf发送完后，内部会自动把adr0指向的buf地址更新给adr1
    if (iis_reg->dma_tx_adr1 == (uint32_t)iis_cb->dma_cfg.dma_txbuf0_ptr) {
        memcpy(iis_cb->dma_cfg.dma_txbuf1_ptr, buf, buf_len);
        iis_reg->dma_tx_adr0 = (uint32_t)iis_cb->dma_cfg.dma_txbuf1_ptr;

    } else {
        memcpy(iis_cb->dma_cfg.dma_txbuf0_ptr, buf, buf_len);
        iis_reg->dma_tx_adr0 = (uint32_t)iis_cb->dma_cfg.dma_txbuf0_ptr;
    }

    iis_reg->dma_tx_sz0  = IISTXSIZE0_TX_KICK | (iis_cb->dma_cfg.tx_samples - 1);
}

void iis_pic_config(iis_typedef *iis_reg, isr_t isr, int pr, IIS_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_pending_bit = 0;

    if (flag_type == 0) {
        return;
    }

    if (flag_type & IIS_FLAG_RX_PND) {
        flag_pending_bit |= IISCON0_RX_INT_EN;
    }

    if (flag_type & IIS_FLAG_TX_PND) {
        flag_pending_bit |= IISCON0_TX_INT_EN;
    }

    if (state != DISABLE) {
        iis_reg->con0 |= flag_pending_bit;
        sys_irq_init(IRQn_DE_IIS_TDNN, pr, isr);
    } else {
        iis_reg->con0 &= ~flag_pending_bit;
    }
}

AT(.com_periph.iis.flag)
FLAG_STATE iis_get_flag(iis_typedef *iis_reg, IIS_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & IIS_FLAG_RX_PND) {
        flag_pending_bit |= IISCPND_RX_DMA_PND;
    } else if (flag_type & IIS_FLAG_TX_PND) {
        flag_pending_bit |= IISCPND_TX_DMA_PND;
    }

    if ((iis_reg->cpnd & flag_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.irrx.flag)
void iis_clear_flag(iis_typedef *iis_reg, IIS_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & IIS_FLAG_RX_PND) {
        flag_pending_bit |= IISCPND_RX_DMA_PND;
    }

    if (flag_type & IIS_FLAG_TX_PND) {
        flag_pending_bit |= IISCPND_TX_DMA_PND;
    }

    iis_reg->cpnd = flag_pending_bit;
}

void iis_deinit(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    iis_reg->con0 = 0;
    iis_reg->con1 = 0;
    clk_gate1_cmd(CLK_GATE1_IIS, CLK_DIS);
    clk_gate1_cmd(CLK_GATE1_IISM, CLK_DIS);
}


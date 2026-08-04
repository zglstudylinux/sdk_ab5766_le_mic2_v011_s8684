/*
 * @File name    : driver_spi.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-08
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the SPI peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_spi.h"


void spi_init(spi_typedef* spix, spi_init_typedef* spi_init_struct)
{
    uint32_t temp = 0;

    /* SPI_CLK freq configuration */
    spix->baud = sys_clk_nhz_get() / spi_init_struct->baud - 1;

    temp |= (uint32_t)(spi_init_struct->bus_mode & SPIxCON_BUSMODE);
    temp |= (uint32_t)(spi_init_struct->role_mode & SPIxCON_SPISM);
    temp |= (uint32_t)(spi_init_struct->output_data_edge & SPIxCON_SMPS);
    temp |= (uint32_t)(spi_init_struct->sample_clk_edge & SPIxCON_SPIOSS);
    temp |= (uint32_t)(spi_init_struct->clkids & SPIxCON_CLKIDS);

    spix->con = temp;
}

void spi_deinit(spi_typedef* spix)
{
    if (spix == SPI1_REG) {
        spix->con = 0;
        clk_gate1_cmd(CLK_GATE1_SPI1, CLK_DIS);
    }
}


void spi_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_SPIEN;
    } else {
        spix->con &= (uint32_t)~SPIxCON_SPIEN;
    }
}

void spi_dir_sel(spi_typedef* spix, SPI_DIR_TYPEDEF direction)
{
    if(direction == SPI_DIR_RX) {
        spix->con |= SPIxCON_RXSEL;
    } else {
        spix->con &= ~SPIxCON_RXSEL;
    }
}


void spi_it_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_SPIIE;
    } else {
        spix->con &= (uint32_t)~SPIxCON_SPIIE;
    }
}


void spi_multi_bit_bus_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_SPIMBEN;
    } else {
        spix->con &= (uint32_t)~SPIxCON_SPIMBEN;
    }
}

/**
  * @brief  Enable or disable the specified SPI hold when bt rx.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_hold_rx_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_HOLDENRX;
    } else {
        spix->con &= (uint32_t)~SPIxCON_HOLDENRX;
    }
}

/**
  * @brief  Enable or disable the specified SPI hold when bt tx.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_hold_tx_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_HOLDENTX;
    } else {
        spix->con &= (uint32_t)~SPIxCON_HOLDENTX;
    }
}

/**
  * @brief  Enable or disable the specified SPI software hold.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_soft_hold_cmd(spi_typedef* spix, FUNCTIONAL_STATE state)
{
    if (state) {
        spix->con |= (uint32_t)SPIxCON_HOLDENSW;
    } else {
        spix->con &= (uint32_t)~SPIxCON_HOLDENSW;
    }
}


AT(.com_periph.spi.get)
FLAG_STATE spi_get_flag(spi_typedef* spix, SPI_FLAG_TYPEDEF spi_flag)
{
    if ((spix->con & spi_flag) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}


AT(.com_periph.spi.clear)
void spi_clear_flag(spi_typedef* spix, SPI_FLAG_TYPEDEF spi_flag)
{
    spix->cpnd |= (uint32_t)spi_flag;
}


void spi_set_dma_cnt(spi_typedef* spix, uint32_t cnt)
{
    spix->dma_cnt = cnt;
}


void spi_set_dma_addr(spi_typedef* spix, uint32_t addr)
{
    spix->dma_adr = (uint32_t)addr;
}


uint8_t spi_receive_data(spi_typedef* spix)
{
    return (uint8_t)spix->buf;
}


void spi_send_data(spi_typedef* spix, uint8_t data)
{
    spix->buf = data;
}


void spi_pic_config(spi_typedef* spix, isr_t isr, int pr, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        sys_irq_init(IRQn_SPIx, pr, isr);
        spix->con |= SPIxCON_SPIIE;
    } else {
        spix->con &= ~SPIxCON_SPIIE;
        PICEN &= ~BIT(IRQn_SPIx);
    }
}

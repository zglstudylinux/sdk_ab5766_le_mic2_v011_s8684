/*
 * @File name    : driver_hsuart.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-09
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the HSUART_REG peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_hsuart.h"

/**
  * @brief  Initializes the hsuart peripheral according to the specified
  *         parameters in the hsuart_init_struct.
  * @param  hsuart_init_struct: pointer to a hsuart_init_typedef structure that
  *         contains the configuration information for the specified HSUART_REG peripheral.
  * @retval None
  */
void hsuart_init(hsuart_typedef *huartx, hsuart_init_typedef *hsuart_init_struct)
{
    uint32_t tmp_reg, clk_val, baud_reg;

    /*--- Enable model function ---*/
    RSTCON0 |= 1<<2;

    tmp_reg = huartx->con;
    //---------- hsuart Tx configuration ----------//
    tmp_reg &= ~(uint32_t)(HSUT0CON_SPBITSEL | HSUT0CON_TXBITSEL | HSUT0CON_TXTRSMODE);
    tmp_reg |= hsuart_init_struct->tx_stop_bit;
    tmp_reg |= hsuart_init_struct->tx_word_len;
    tmp_reg |= hsuart_init_struct->tx_mode;

    //---------- hsuart Rx configuration ----------//
    tmp_reg &= ~(HSUT0CON_RXBITSEL | HSUT0CON_RXTRSMODE);
    tmp_reg |= hsuart_init_struct->rx_word_len;
    tmp_reg |= hsuart_init_struct->rx_mode;

    /* hsuart one line configuration */
    tmp_reg &= ~HSUT0CON_ONELINE;
    tmp_reg |= hsuart_init_struct->one_line_en;

    /* hsuart Rx dma mode loop buf configuration */
    if (hsuart_init_struct->rx_mode == HSUT_RX_DMA_MODE) {
        tmp_reg &= ~HSUT0CON_RXLPBUFEN;
        tmp_reg |= hsuart_init_struct->rx_dma_buf_mode;
    }

    huartx->con = tmp_reg;

    clk_val = clk_hsut0_clk_get(CLK_VALUE_MODE_FREQ);

    if (hsuart_init_struct->baud == 0) {
        return;
    }

    baud_reg = (clk_val / hsuart_init_struct->baud) - 1;
    tmp_reg = (uint32_t)((baud_reg << 16) | baud_reg);
    huartx->baud = tmp_reg;
}

/**
 * @brief  De-initialize the specified HSUART_REG peripheral.
 * @retval None
 */
void hsuart_deinit(hsuart_typedef *huartx)
{
    huartx->con = 0;
    clk_gate0_cmd(CLK_GATE0_HSUART, CLK_DIS);
}

/**
  * @brief  Enable or disable the specified HSUART_REG peripheral.
  * @param  rec_tra_sel: select whether Rx or Tx needs to be configured.
  *         this parameter cna be: HSUT_TRANSMIT or HSUT_RECEIVE.
  * @param  state: the state of the HSUART_REG peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void hsuart_cmd(hsuart_typedef *huartx, HSUT_REC_TRA_CMD rec_tra_sel, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        huartx->con |= (uint32_t)rec_tra_sel;
    } else {
        huartx->con &= ~(uint32_t)rec_tra_sel;
    }
}

/**
  * @brief  Transmits data through the HSUART_REG buf mode.
  * @param  data: the data to transmit.
  * @retval None
  */
AT(.com_periph.hsuart.send)
void hsuart_send_data(hsuart_typedef *huartx, uint16_t data)
{
    if (huartx->con & HSUT0CON_TXBITSEL) {
        huartx->data = (uint32_t)(data & 0x1ff);
    } else {
        huartx->data = (uint32_t)(data & 0xff);
    }
}

/**
  * @brief  Return the received data lastly by the buf mode.
  * @retval The received data.
  */
AT(.com_periph.hsuart.recv)
uint16_t hsuart_receive_data(hsuart_typedef *huartx)
{
    return (huartx->fifo & 0xff);
}

/**
 * @brief  Get the receive data from Rx DMA FIFO.
 * @param  buf: A buf pointer used to return data.
 * @param  len: The data number need to read.
 * @retval Result of func execution. It will be SUCCESS or FAILED.
 */
AT(.com_periph.hsuart.receive_data)
STATUS_STATE hsuart_receive_data_from_fifo(hsuart_typedef *huartx, uint8_t *buf, uint8_t len)
{
    u8 i = 0;

    if (len > huartx->fifo_cnt || !buf) {
        return FAILED;
    }

    while (len--) {
        huartx->fifo |= (1 << 8);
        while (!(huartx->fifo & (1 << 9)));
        buf[i++] = huartx->fifo & 0xff;
    }

    return SUCCESS;
}

/**
 * @brief  Get Rx DMA mode FIFO counter value.
 * @retval The value of FIFO counter.
 */
AT(.com_periph.hsuart.fifo_get)
uint16_t hsuart_get_fifo_counter(hsuart_typedef *huartx)
{
    return huartx->fifo_cnt;
}

/**
 * @brief  Clear fifo counter.
 * @retval None
 */
void hsuart_clear_fifo_counter(hsuart_typedef *huartx)
{
    huartx->cpnd |= (uint32_t)(1 << 17);
}

/**
 * @brief  Change the HSUART_REG baud.
 * @param  baud: Specifies the baud rateof the serial port.
 * @retval None.
 */
void hsuart_baud_config(hsuart_typedef *huartx, uint32_t baud)
{
    u32 baud_reg, clk_val;

    //---------- hsuart baud and clock configuration ----------//
    clk_val = clk_hsut0_clk_get(CLK_VALUE_MODE_FREQ);

    baud_reg = (clk_val / baud) - 1;
    huartx->baud = (uint32_t)((baud_reg << 16) | baud_reg);
}

/**
  * @brief  Enable or disable the specified HSUART_REG interrupt.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  interrup_type: specifies the HSUART_REG interrupt sources to be enable or disable.
  *         this parameter can be one of the following values:
  *             @arg HSUART_IT_RX_DMA_HF: RX DMA half-full interrupt enable bit.
  *             @arg HSUART_IT_TX: Transmit single or n bytes data finish interrupt enable bit.
  *             @arg HSUART_IT_RX: Receive single or n bytes data finish interrupt enable bit.
  * @param  state: the state of the HSUART_REG peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void hsuart_pic_config(hsuart_typedef *huartx, isr_t isr, int pr, HSUART_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state)
{
    uint32_t interrupt_pending_bit = 0;
    uint32_t all_interrupt_type_mask = HSUART_IT_RX_DMA_HF | HSUART_IT_TX | HSUART_IT_RX;

    if (interrupt_type == 0) {
        return;
    }

    if (interrupt_type & HSUART_IT_RX) {
        interrupt_pending_bit |= HSUT0CON_RXIE;
    }

    if (interrupt_type & HSUART_IT_TX) {
        interrupt_pending_bit |= HSUT0CON_TXIE;
    }

    if (interrupt_type & HSUART_IT_RX_DMA_HF) {
        interrupt_pending_bit |= HSUT0CON_RXHF_IE;
    }

    if (state != DISABLE) {
        sys_irq_init(IRQn_HSUART, pr, isr);
        huartx->con |= interrupt_pending_bit;
    } else {
        huartx->con &= ~interrupt_pending_bit;
        if (interrupt_type == all_interrupt_type_mask) {
            PICEN &= ~BIT(IRQn_HSUART);
        }
    }
}

/**
  * @brief  Check the specified HSUART_REG flag is set or not.
  * @param  interrupt_type: specifies the flag to check.
  *         this parameter can be one of the following values:
  *             @arg HSUART_IT_RX_DMA_HF: RX DMA half-full interrupt pending bit.
  *             @arg HSUART_IT_RX_TMR_OV: RX timer overflow interrupt pending bit.
  *             @arg HSUART_IT_TX: Transmit single or n bytes data finish interrupt pending bit.
  *             @arg HSUART_IT_RX: Receive single or n bytes data finish interrupt pending bit.
  * @retval The state of uart_flag (SET or RESET).
  */
AT(.com_periph.hsuart.get_flag)
FLAG_STATE hsuart_get_flag(hsuart_typedef *huartx, HSUART_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & HSUART_IT_RX) {
        interrupt_pending_bit |= HSUT0CON_RXPND;
    }

    if (interrupt_type & HSUART_IT_TX){
        interrupt_pending_bit |= HSUT0CON_TXPND;
    }

    if (interrupt_type & HSUART_IT_RX_TMR_OV){
        interrupt_pending_bit |= HSUT0CON_TMROV;
    }

    if (interrupt_type & HSUART_IT_RX_DMA_HF){
        interrupt_pending_bit |= HSUT0CON_RXHF_PND;
    }

    if ((huartx->con & interrupt_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

/**
  * @brief  Clear the HSUART_REG's pending flag.
  * @param  hsuart_flag: specifies the flag to check.
  *         this parameter can be one of the following values:
  *             @arg HSUART_IT_RX_DMA_HF: RX DMA half-full interrupt pending bit.
  *             @arg HSUART_IT_RX_TMR_OV: RX timer overflow interrupt pending bit.
  *             @arg HSUART_IT_TX: Transmit single or n bytes data finish interrupt pending bit.
  *             @arg HSUART_IT_RX: Receive single or n bytes data finish interrupt pending bit.
  * @retval None
  */
AT(.com_periph.hsuart.clear)
void hsuart_clear_flag(hsuart_typedef *huartx, HSUART_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_pending_bit = 0;

    if (interrupt_type & HSUART_IT_RX) {
        interrupt_pending_bit |= HSUT0CPND_CRXPND;
    }

    if (interrupt_type & HSUART_IT_TX){
        interrupt_pending_bit |= HSUT0CPND_CTXPND;
    }

    if (interrupt_type & HSUART_IT_RX_TMR_OV){
        interrupt_pending_bit |= HSUT0CPND_CTMROV;
    }

    if (interrupt_type & HSUART_IT_RX_DMA_HF){
        interrupt_pending_bit |= HSUT0CPND_CRXHFPND;
    }

    huartx->cpnd |= interrupt_pending_bit;
}

/**
 * @brief  Config idle timer that can trigger an interrupt when Rx idle.
 * @param  idle_time: The maximum number of idle clocks.
 * @param  state: the state of the HSUART_REG peripheral.
*                 this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void hsuart_rx_idle_config(hsuart_typedef *huartx, uint16_t idle_time, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        HSUART_REG->tmr_cnt = idle_time;
        HSUART_REG->con |= HSUT0CON_HSUTTMREN;
    } else {
        HSUART_REG->con &= ~HSUT0CON_HSUTTMREN;
    }
}

/**
  * @brief  Start send or receive dma. When receiving data of indefinite
  *         length, need to enable the idle timer.
  * @param  rec_tra_sel: Tx or Rx to configure.
  *         this parameter can be: (HSUT_TRANSMIT or HSUT_RECEIVE)
  * @param  addr: DMA start address.
  * @param  len: DMA transmit len.
  * @retval None
  */
AT(.com_periph.hsuart.dma)
void hsuart_dma_start(hsuart_typedef *huartx, HSUT_REC_TRA_CMD rec_tra_sel, uint32_t addr, uint16_t len)
{
    if (rec_tra_sel == HSUT_RECEIVE) {
        huartx->rx_adr = addr;
        huartx->rx_cnt = (uint32_t)len;
    } else if (rec_tra_sel == HSUT_TRANSMIT) {
        huartx->tx_adr = addr;
        huartx->tx_cnt = (uint32_t)len;
    }
}

/**
  * @brief  stop send or receive dma.
  * @param  rec_tra_sel: Tx or Rx to configure.
  *         this parameter can be: (HSUT_TRANSMIT or HSUT_RECEIVE)
  * @retval None
  */
AT(.com_periph.hsuart.stop)
void hsuart_dma_stop(hsuart_typedef *huartx, HSUT_REC_TRA_CMD rec_tra_sel)
{
    if (rec_tra_sel == HSUT_RECEIVE) {
        huartx->rx_cnt = 0;
    } else if (rec_tra_sel == HSUT_TRANSMIT) {
        huartx->tx_cnt = 0;
    }
}

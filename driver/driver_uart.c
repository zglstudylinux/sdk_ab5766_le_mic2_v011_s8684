/*
 * @File name    : driver_uart.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-08
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the UART peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_uart.h"

void uart_init(uart_typedef *uartx, uart_init_typedef* uart_init_struct)
{
    uint32_t tmp_reg, uart_clk_val, baud_reg;

    //---------- UART Controller Register ----------//
    tmp_reg = uartx->con;
    tmp_reg &= (uint32_t)(~UARTxCON_RXEN);
    tmp_reg &= (uint32_t)(~UARTxCON_ONELINE);
    tmp_reg &= (uint32_t)(~UARTxCON_BIT9EN);
    tmp_reg &= (uint32_t)(~UARTxCON_SB2EN);
    tmp_reg &= (uint32_t)(~UARTxCON_FIXBAUD);
    tmp_reg |= (uint32_t)(uart_init_struct->mode |
                          uart_init_struct->one_line_enable |
                          uart_init_struct->stop_bits |
                          uart_init_struct->word_len |
                          uart_init_struct->baud_fix);

    uartx->con = tmp_reg;

    if (uart_init_struct->baud_fix == UART_BAUD_FIX) {
        uart_clk_val = clk_uart_clk_get(uartx, CLK_VALUE_MODE_FREQ);
        baud_reg = (uint16_t)((uart_clk_val / uart_init_struct->baud) - 1);
        uartx->baud = baud_reg;
    }
}


void uart_deinit(uart_typedef *uartx)
{
    uartx->con = 0;

    if (uartx == UART0_REG) {
        clk_gate0_cmd(CLK_GATE0_UART0, CLK_DIS);
    } else if (uartx == UART1_REG) {
        clk_gate1_cmd(CLK_GATE0_UART1, CLK_DIS);
    }
}


void uart_cmd(uart_typedef *uartx, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        uartx->con |= UARTxCON_UTEN;
    } else {
        uartx->con &= (uint32_t)(~UARTxCON_UTEN);
    }
}


AT(.com_periph.uart.send)
void uart_send_data(uart_typedef *uartx, uint16_t data)
{
    uartx->data = (data & (uint16_t)0xff);
}


AT(.com_periph.uart.recv)
uint16_t uart_receive_data(uart_typedef *uartx)
{
    if (uartx->con & UARTxCON_BIT9EN) {
        return (uartx->data & (uint16_t)0x1ff);
    } else {
        return (uartx->data & (uint16_t)0xff);
    }
}


void uart_pic_config(uart_typedef *uartx, isr_t isr, int pr, UART_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state)
{
    uint32_t interrupt_con_bit = 0;
    uint32_t all_interrupt_type_mask = UART_IT_TX | UART_IT_RX;

    if (interrupt_type == 0) {
        return;
    }

    if (interrupt_type & UART_IT_TX) {
        interrupt_con_bit |= UARTxCON_TXIE;
    }

    if (interrupt_type & UART_IT_RX) {
        interrupt_con_bit |= UARTxCON_RXIE;
    }

    if (state != DISABLE) {
        sys_irq_init(IRQn_UART, pr, isr);
        uartx->con |= interrupt_con_bit;
    } else {
        uartx->con &= ~interrupt_con_bit;
        if (interrupt_type == all_interrupt_type_mask) {
            PICEN &= ~BIT(IRQn_UART);
        }
    }
}


AT(.com_periph.uart.get_flag)
FLAG_STATE uart_get_flag(uart_typedef *uartx, UART_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_con_bit = 0;

    if (interrupt_type & UART_IT_TX) {
        interrupt_con_bit |= UARTxCON_TXPND;
    }

    if (interrupt_type & UART_IT_RX) {
        interrupt_con_bit |= UARTxCON_RXPND;
    }

    if ((uartx->con & interrupt_con_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.uart.clear_flag)
void uart_clear_flag(uart_typedef *uartx, UART_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_con_bit = 0;

    if (interrupt_type & UART_IT_TX) {
        interrupt_con_bit |= UARTxCPND_CTXPND;
    }

    if (interrupt_type & UART_IT_RX) {
        interrupt_con_bit |= UARTxCPND_CRXPND;
    }

    uartx->cpnd |= interrupt_con_bit;
}


void uart_baud_config(uart_typedef *uartx, uint32_t baud)
{
    uint32_t clk_val, baud_reg;

    if (uartx->con & UARTxCON_UTEN)
    while ((uartx->con & UARTxCON_TXPND) == 0);

    if (uartx->con & UARTxCON_FIXBAUD) {
        clk_val = clk_uart_clk_get(uartx, CLK_VALUE_MODE_FREQ);
        baud_reg = (uint16_t)((clk_val / baud) - 1);
        uartx->baud = baud_reg;
    }
}

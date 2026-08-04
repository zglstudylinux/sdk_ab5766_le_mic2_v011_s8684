/*
 * @File name    : driver_uart.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-8
 * @Description  : This file contains all the functions prototypes for the UART library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_UART_H
#define _DRIVER_UART_H

#include "driver_com.h"

/**
 * @brief UART IT Flag Definition
 */
typedef enum {
    UART_IT_TX          = 0x01,
    UART_IT_RX          = 0x02,
} UART_IT_TYPEDEF;

/**
 * @brief Rx Enable enumeration
 */
typedef enum {
    UART_SEND_MODE      = 0x00000000,
    UART_SEND_RECV_MODE = 0x00000080,
} UART_MODE_TYPEDEF;

/**
 * @brief One-line Mode Enumeration
 */
typedef enum {
    UART_SEPARATE       = 0x00000000,
    UART_ONE_LINE       = 0x00000040,
} UART_ONELINE_TYPEDEF;


/**
 * @brief Stop Bit Select Enumeration
 */
typedef enum {
    UART_STOP_BIT_1BIT  = 0x00000000,
    UART_STOP_BIT_2BIT  = 0x00000010,
} UART_STOP_BIT_TYPEDEF;

/**
 * @brief Word Length Enumeration
 */
typedef enum {
    UART_WORD_LENGTH_8b = 0x00000000,
    UART_WORD_LENGTH_9b = 0x00000002,
} UART_WORD_LEN_TYPEDEF;

/**
 * @brief Baud Fix Enumeration.
 */
typedef enum {
    UART_BAUD_AUTO      = 0x00000000,
    UART_BAUD_FIX       = 0x00000020,
} UART_BAUD_FIX_TYPEDEF;

/**
 * @brief UART Init Structure
 */
typedef struct {
    uint32_t                baud;
    UART_MODE_TYPEDEF       mode;
    UART_ONELINE_TYPEDEF    one_line_enable;
    UART_STOP_BIT_TYPEDEF   stop_bits;
    UART_WORD_LEN_TYPEDEF        word_len;
    UART_BAUD_FIX_TYPEDEF   baud_fix;
} uart_init_typedef;

/************************* Function used to configure UART *************************/
/**
  * @brief  Initializes the uart peripheral according to the specified
  *         parameters in the uart_init_struct.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  uart_init_struct: pointer to a uart_init_typedef structure that
  *         contains the configuration information for the specified UART peripheral.
  * @retval None
  */
void uart_init(uart_typedef *uartx, uart_init_typedef *uart_init_struct);

/**
 * @brief  De-initialize the specified uart peripheral.
 * @param  uartx: which uart need to config. it should the one of [UART or UDET].
 * @retval None
 */
void uart_deinit(uart_typedef *uartx);

/**
  * @brief  Enable or disable the specified UART peripheral.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  state: the state of the UARTx peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void uart_cmd(uart_typedef *uartx, FUNCTIONAL_STATE state);

/**
  * @brief  Transmits data through the UARTx peripheral.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  data: the data to transmit.
  * @retval None
  */
void uart_send_data(uart_typedef *uartx, uint16_t data);

/**
  * @brief  Return the received data lastly by the UARTx peripheral.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @retval The received data.
  */
uint16_t uart_receive_data(uart_typedef *uartx);

/**
  * @brief  Enable or disable the specified UARTx interrupt.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  interrupt_type: specifies the UART interrupt sources to be enable or disable.
  *         this parameter can be one of the following values:
  *             @arg UART_IT_TX: Transmit single data finish interrupt enable bit.
  *             @arg UART_IT_RX: Receive single data finish interrupt enable bit.
  * @param  state: the state of the UARTx peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void uart_pic_config(uart_typedef *uartx, isr_t isr, int pr, UART_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state);

/**
  * @brief  Check the specified UART flag is set or not.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  interrupt_type: specifies the flag to check.
  *         this parameter can be one of the following values:
  *             @arg UART_IT_TX: Transmit single data finish interrupt enable bit.
  *             @arg UART_IT_RX: Receive single data finish interrupt enable bit.
  * @retval The state of uart_flag (SET or RESET).
  */
FLAG_STATE uart_get_flag(uart_typedef *uartx, UART_IT_TYPEDEF interrupt_type);

/**
  * @brief  Clear the UART's pending flag.
  * @param  uartx: where x can be (0..1) to select the uartx peripheral.
  * @param  interrupt_type: specifies the flag to check.
  *         this parameter can be one of the following values:
  *             @arg UART_IT_TX: Transmit single data finish interrupt enable bit.
  *             @arg UART_IT_RX: Receive single data finish interrupt enable bit.
  *             @arg UART_IT_ERR: Raise error interrupt enable bit.
  *             @arg UART_IT_KEY: Match key interrupt enable bit.
  *             @arg UART_IT_RSTKEY: Reset match key interrupt enable bit.
  * @retval None
  */
void uart_clear_flag(uart_typedef *uartx, UART_IT_TYPEDEF interrupt_type);

/**
 * @brief  Config the baud rate of the serial port.
 * @param  uartx: The uart that need to config.
 * @param  baud: The baudrate value need to set.
 * @retval None
 */
void uart_baud_config(uart_typedef *uartx, uint32_t baud);

#endif // _DRIVER_UART_H

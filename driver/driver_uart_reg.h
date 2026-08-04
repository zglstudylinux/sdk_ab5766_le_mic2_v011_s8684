/*
 * @File name    : driver_uart_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-8
 * @Description  : This file contains all the functions prototypes for the UART library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_UART_REG_H
#define _DRIVER_UART_REG_H

#define UART0_BASE                          (&UART0CON)
#define UART1_BASE                          (&UART1CON)

#define UART0_REG                           ((uart_typedef *) UART0_BASE)
#define UART1_REG                           ((uart_typedef *) UART1_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     UART                                        */
/*---------------------------------------------------------------------------------*/
/*********************** Bit definition for UARTxCON register **********************/
#define UARTxCON_UTEN                       ((uint32_t)0x00000001)
#define UARTxCON_BIT9EN                     ((uint32_t)0x00000002)
#define UARTxCON_RXIE                       ((uint32_t)0x00000004)
#define UARTxCON_TXIE                       ((uint32_t)0x00000008)
#define UARTxCON_SB2EN                      ((uint32_t)0x00000010)
#define UARTxCON_FIXBAUD                    ((uint32_t)0x00000020)
#define UARTxCON_ONELINE                    ((uint32_t)0x00000040)
#define UARTxCON_RXEN                       ((uint32_t)0x00000080)
#define UARTxCON_TXPND                      ((uint32_t)0x00000100)
#define UARTxCON_RXPND                      ((uint32_t)0x00000200)
#define UARTxCON_KEYMAT                     ((uint32_t)0x00000400)
#define UARTxCON_RXBCNT                     ((uint32_t)0x00007800)
#define UARTxCON_RXBCNT_0                   ((uint32_t)0x00000800)
#define UARTxCON_RXBCNT_1                   ((uint32_t)0x00001000)
#define UARTxCON_RXBCNT_2                   ((uint32_t)0x00002000)
#define UARTxCON_RXBCNT_3                   ((uint32_t)0x00004000)
#define UARTxCON_RX4BUFERROR                ((uint32_t)0x00008000)

/*********************** Bit definition for UARTxCPND register *********************/
#define UARTxCPND_ALL                       ((uint32_t)0x00048700)
#define UARTxCPND_CTXPND                    ((uint32_t)0x00000100)
#define UARTxCPND_CRXPND                    ((uint32_t)0x00000200)
#define UARTxCPND_CKEYMAT                   ((uint32_t)0x00000400)
#define UARTxCPND_RX4BUF_ERR_CLR            ((uint32_t)0x00008000)
#define UARTxCPND_CBDCFM                    ((uint32_t)0x00040000)

/**
 * @brief Uart Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 cpnd;
    volatile u32 baud;
    volatile u32 data;
} uart_typedef;

#endif

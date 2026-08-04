/*
 * @File name    : driver_hsuart_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-09
 * @Description  : This file contains all the functions prototypes for the HSUART_REG library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_HSUART_REG_H
#define _DRIVER_HSUART_REG_H

#define HSUART_BASE                         (&HSUT0CON)

#define HSUART_REG                          ((hsuart_typedef *) HSUART_BASE)

/*---------------------------------------------------------------------------------*/
/*                                    HUART                                        */
/*---------------------------------------------------------------------------------*/
/*********************** Bit definition for HSUT0CON register **********************/
#define HSUT0CON_URXEN                      ((uint32_t)0x00000001)
#define HSUT0CON_UTXEN                      ((uint32_t)0x00000002)
#define HSUT0CON_RXIE                       ((uint32_t)0x00000004)
#define HSUT0CON_TXIE                       ((uint32_t)0x00000008)
#define HSUT0CON_RXTRSMODE                  ((uint32_t)0x00000010)
#define HSUT0CON_RXBITSEL                   ((uint32_t)0x00000020)
#define HSUT0CON_RXLPBUFEN                  ((uint32_t)0x00000040)
#define HSUT0CON_TXTRSMODE                  ((uint32_t)0x00000080)
#define HSUT0CON_TXBITSEL                   ((uint32_t)0x00000100)
#define HSUT0CON_SPBITSEL                   ((uint32_t)0x00000200)
#define HSUT0CON_HSUTTMREN                  ((uint32_t)0x00000400)
#define HSUT0CON_RXHF_PND                   ((uint32_t)0x00000800)
#define HSUT0CON_RXPND                      ((uint32_t)0x00001000)
#define HSUT0CON_TXPND                      ((uint32_t)0x00002000)
#define HSUT0CON_RXFAIL                     ((uint32_t)0x00004000)
#define HSUT0CON_TMROV                      ((uint32_t)0x00008000)
#define HSUT0CON_RXOVE_PND                  ((uint32_t)0x00010000)
#define HSUT0CON_RXHF_IE                    ((uint32_t)0x00020000)
#define HSUT0CON_ONELINE                    ((uint32_t)0x00040000)

/*********************** Bit definition for HSUT0CPND register *********************/
#define HSUT0CPND_CUTRX                     ((uint32_t)0x00000001)
#define HSUT0CPND_CUTTX                     ((uint32_t)0x00000002)
#define HSUT0CPND_CCTSPND                   ((uint32_t)0x00000400)
#define HSUT0CPND_CRXHFPND                  ((uint32_t)0x00000800)
#define HSUT0CPND_CRXPND                    ((uint32_t)0x00001000)
#define HSUT0CPND_CTXPND                    ((uint32_t)0x00002000)
#define HSUT0CPND_CRXFAIL                   ((uint32_t)0x00004000)
#define HSUT0CPND_CTMROV                    ((uint32_t)0x00008000)
#define HSUT0CPND_CRXOVEPND                 ((uint32_t)0x00010000)
#define HSUT0CPND_CRXLBBUF                  ((uint32_t)0x00020000)

/*********************** Bit definition for HSUT0FCCON register ********************/
#define HSUT0CCON_UCTS_EN                   ((uint32_t)0x00000001)
#define HSUT0CCON_URTS_EN                   ((uint32_t)0x00000002)
#define HSUT0CCON_CTSIE                     ((uint32_t)0x00000004)
#define HSUT0CCON_RTS_MODE                  ((uint32_t)0x00000008)
#define HSUT0CCON_CTSPND                    ((uint32_t)0x00000010)
#define HSUT0CCON_CTSS                      ((uint32_t)0x00000020)
#define HSUT0CCON_RTSS                      ((uint32_t)0x00000040)
#define HSUT0CCON_RTS_SW                    ((uint32_t)0x00000080)

/**
 * @brief High Speed UART Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 cpnd;
    volatile u32 baud;
    volatile u32 data;
    volatile u32 tx_cnt;
    volatile u32 tx_adr;
    volatile u32 rx_cnt;
    volatile u32 rx_adr;
    volatile u32 fifo_cnt;
    volatile u32 fifo;
    volatile u32 fifo_adr;
    volatile u32 tmr_cnt;
    volatile u32 flow_ctrl;
} hsuart_typedef;

#endif

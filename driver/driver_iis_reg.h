/*
 * @File name    : driver_iis_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-28
 * @Description  : This file contains all the register definition for the IIS.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_IIS_REG_H
#define _DRIVER_IIS_REG_H

#define IIS_BASE                            (SFR5_BASE + 0x34*4)

#define IIS_REG                             ((iis_typedef *) IIS_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     IIS                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for IISCON0 register **********************/
#define IISCON0_RX_INT_EN                   ((uint32_t)0x80000000)
#define IISCON0_TX_INT_EN                   ((uint32_t)0x40000000)
#define IISCON0_CH_DEPTH_SEL                ((uint32_t)0x1c000000)
#define IISCON0_CH_CNT_SEL                  ((uint32_t)0x03800000)
#define IISCON0_RX_BIT_DEPTH                ((uint32_t)0x007c0000)
#define IISCON0_TX_BIT_DEPTH                ((uint32_t)0x0003e000)
#define IISCON0_WORD_WIDTH                  ((uint32_t)0x00001f00)
#define IISCON0_WS_PULSE_SEL                ((uint32_t)0x000000c0)
#define IISCON0_TX_DAT_SEL                  ((uint32_t)0x00000020)
#define IISCON0_DDLY_EN                     ((uint32_t)0x00000010)
#define IISCON0_SLV_MODE                    ((uint32_t)0x00000008)
#define IISCON0_RX_EN                       ((uint32_t)0x00000004)
#define IISCON0_TX_EN                       ((uint32_t)0x00000002)
#define IISCON0_IIS_EN                      ((uint32_t)0x00000001)

/************************ Bit definition for IISCON1 register **********************/
#define IISCON1_IIS_RUN                     ((uint32_t)0x00800000)
#define IISCON1_SLV_READY                   ((uint32_t)0x00400000)
#define IISCON1_RX_ADR_SEL                  ((uint32_t)0x00200000)
#define IISCON1_TX_ADR_SEL                  ((uint32_t)0x00100000)
#define IISCON1_LOSE_CH_HDL_EN              ((uint32_t)0x00080000)
#define IISCON1_DAT_SYNC_WS                 ((uint32_t)0x00040000)
#define IISCON1_RX_BIT_REV                  ((uint32_t)0x00020000)
#define IISCON1_TX_BIT_REV                  ((uint32_t)0x00010000)
#define IISCON1_DI_FLT_EN                   ((uint32_t)0x00008000)
#define IISCON1_WS_FLT_EN                   ((uint32_t)0x00004000)
#define IISCON1_BCLK_FLT_EN                 ((uint32_t)0x00002000)
#define IISCON1_WS_INV                      ((uint32_t)0x00001000)
#define IISCON1_BCLK_INV                    ((uint32_t)0x00000800)
#define IISCON1_MCLKO_EN                    ((uint32_t)0x00000400)
#define IISCON1_1W_DIR                      ((uint32_t)0x00000200)
#define IISCON1_1W_EN                       ((uint32_t)0x00000100)
#define IISCON1_DI1_EN                      ((uint32_t)0x00000020)
#define IISCON1_DI0_EN                      ((uint32_t)0x00000010)
#define IISCON1_DO1_EN                      ((uint32_t)0x00000002)
#define IISCON1_DO0_EN                      ((uint32_t)0x00000001)

/************************ Bit definition for IISMCLKDIV register *******************/
#define IISMCLKDIV_DIV                      ((uint32_t)0x000000ff)

/************************ Bit definition for IISCPND register **********************/
#define IISCPND_LOSE_CH_PND                 ((uint32_t)0x00000040)
#define IISCPND_RX_LOSE_BIT                 ((uint32_t)0x00000020)
#define IISCPND_TX_LOSE_BIT                 ((uint32_t)0x00000010)
#define IISCPND_RX_BUF_ERR                  ((uint32_t)0x00000008)
#define IISCPND_TX_BUF_ERR                  ((uint32_t)0x00000004)
#define IISCPND_RX_DMA_PND                  ((uint32_t)0x00000002)
#define IISCPND_TX_DMA_PND                  ((uint32_t)0x00000001)

/************************ Bit definition for IISTXSIZE0 register **********************/
#define IISTXSIZE0_TX_KICK                  ((uint32_t)0x80000000)
#define IISTXSIZE0_TX_SIZE                  ((uint32_t)0x00000fff)

/**
 * @brief IIS Register Structure
 */
typedef struct {
    volatile u32 con0;
    volatile u32 con1;
    volatile u32 dma_tx_adr0;
    volatile u32 dma_tx_adr1;
    volatile u32 dma_rx_adr0;
    volatile u32 dma_rx_adr1;
    volatile u32 dma_tx_sz0;
    volatile u32 dma_tx_sz1;
    volatile u32 mclk_div;
    volatile u32 cpnd;
} iis_typedef;

#endif

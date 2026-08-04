/*
 * @File name    : driver_iic.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-26
 * @Description  : This file contains all the functions prototypes for the IIC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_IIC_REG_H
#define _DRIVER_IIC_REG_H

#define IIC_BASE                            (&IICCON0)

#define IIC_REG                             ((iic_typedef *) IIC_BASE)


/*---------------------------------------------------------------------------------*/
/*                                     IIC                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for IICCON0 register **********************/
#define IICCON0_DONE                        ((uint32_t)0x80000000)
#define IICCON0_ACKSTATUS                   ((uint32_t)0x40000000)
#define IICCON0_CLR_DONE                    ((uint32_t)0x20000000)
#define IICCON0_KS                          ((uint32_t)0x10000000)
#define IICCON0_CLR_ALL                     ((uint32_t)0x08000000)
#define IICCON0_RX_NACK_EN                  ((uint32_t)0x00004000)
#define IICCON0_SMP_SEL                     ((uint32_t)0x00002000)
#define IICCON0_MODE_BIT                    ((uint32_t)0x00001000)
#define IICCON0_MODE_POS                    12

#define IICCON0_POSDIV_MASK                 ((uint32_t)0x000003f0)
#define IICCON0_POSDIV_LSB                  4

#define IICCON0_HOLDCNT_MASK                ((uint32_t)0x0000000c)
#define IICCON0_HOLDCNT_LSB                 2

#define IICCON0_INTEN                       ((uint32_t)0x00000002)
#define IICCON0_IICEN                       ((uint32_t)0x00000001)

/************************ Bit definition for IICCON1 register **********************/
#define IICCON1_DATA_CNT                    ((uint32_t)0x00000007)

#define IICCON1_START0_EN                   ((uint32_t)0x00000008)
#define IICCON1_CTL0_EN                     ((uint32_t)0x00000010)
#define IICCON1_ADR0_EN                     ((uint32_t)0x00000020)
#define IICCON1_ADR1_EN                     ((uint32_t)0x00000040)
#define IICCON1_START1_EN                   ((uint32_t)0x00000080)
#define IICCON1_CTL1_EN                     ((uint32_t)0x00000100)
#define IICCON1_RDAT_EN                     ((uint32_t)0x00000200)
#define IICCON1_WDAT_EN                     ((uint32_t)0x00000400)
#define IICCON1_STOP_EN                     ((uint32_t)0x00000800)
#define IICCON1_TXNAK_EN                    ((uint32_t)0x00001000)

/************************ Bit definition for IICSSTS register **********************/
#define IICSSTS_RXVLD                       ((uint32_t)0x00000001)
#define IICSSTS_TXVLD                       ((uint32_t)0x00000002)
#define IICSSTS_START                       ((uint32_t)0x00000004)
#define IICSSTS_RESTART                     ((uint32_t)0x00000008)
#define IICSSTS_STOP                        ((uint32_t)0x00000010)
#define IICSSTS_SRX                         ((uint32_t)0x00000020)
#define IICSSTS_STX                         ((uint32_t)0x00000040)

#define IICSSTS_DATA_CNT_MASK               ((uint32_t)0x00000700)
#define IICSSTS_DATA_CNT_LSB                8

#define IICSSTS_RXVLD_SET                   ((uint32_t)0x00010000)
#define IICSSTS_TXVLD_SET                   ((uint32_t)0x00020000)
#define IICSSTS_START_CLR                   ((uint32_t)0x00040000)
#define IICSSTS_RESTART_CLR                 ((uint32_t)0x00080000)
#define IICSSTS_STOP_CLR                    ((uint32_t)0x00100000)

/************************ Bit definition for IICDMACNT register **********************/
#define IICDMACNT_DMA_EN                    ((uint32_t)0x00000001)
#define IICDMACNT_DMA_ED_EN                 ((uint32_t)0x00000002)
#define IICDMACNT_DMA_DONE                  ((uint32_t)0x00000004)
#define IICDMACNT_DMA_CNT_MASK              ((uint32_t)0xffff0000)
#define IICDMACNT_DMA_CNT_LSB               16

/**
 * @brief IIC Register Structure
 */
typedef struct {
    volatile u32 con0;
    volatile u32 con1;
    volatile u32 cmd_addr;
    volatile u32 data;
    volatile u32 dma_addr;
    volatile u32 dma_cnt;
    volatile u32 slave_sta;
} iic_typedef;

#endif

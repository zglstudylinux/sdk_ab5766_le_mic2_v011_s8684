/*
 * @File name    : driver_spi_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-09
 * @Description  : This file contains all the register definition for the DESPI.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_DESPI_REG_H
#define _DRIVER_DESPI_REG_H

#define DESPI_BASE                           (SFR12_BASE + 0x1e*4)

#define DESPI_REG                            ((despi_typedef *) DESPI_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     DESPI                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for DESPICON register **********************/
#define DESPICON_CL_ORD                      ((uint32_t)0x80000000)
#define DESPICON_OUT_FMT                     ((uint32_t)0x60000000)
#define DESPICON_IN_FMT                      ((uint32_t)0x18000000)
#define DESPICON_DUAL_EN                     ((uint32_t)0x04000000)
#define DESPICON_DCX_REUSE                   ((uint32_t)0x00100000)
#define DESPICON_3W_9BIT                     ((uint32_t)0x00080000)
#define DESPICON_3W_CTR                      ((uint32_t)0x00040000)
#define DESPICON_SPIPND                      ((uint32_t)0x00010000)
#define DESPICON_HOLDENTX                    ((uint32_t)0x00001000)
#define DESPICON_HOLDENRX                    ((uint32_t)0x00000800)
#define DESPICON_SPILOSS                     ((uint32_t)0x00000400)
#define DESPICON_SPIMBEN                     ((uint32_t)0x00000200)
#define DESPICON_SPILF_EN                    ((uint32_t)0x00000100)
#define DESPICON_IE                          ((uint32_t)0x00000080)
#define DESPICON_SMPS                        ((uint32_t)0x00000040)
#define DESPICON_CLKIDS                      ((uint32_t)0x00000020)
#define DESPICON_RXSEL                       ((uint32_t)0x00000010)
#define DESPICON_BUSMODE                     ((uint32_t)0x0000000c)
#define DESPICON_SPISM                       ((uint32_t)0x00000002)
#define DESPICON_EN                          ((uint32_t)0x00000001)

/************************ Bit definition for DESPICPND register **********************/
#define DESPICPND_CPND                       ((uint32_t)0x00010000)

/**
 * @brief DESPI Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 buf;
    volatile u32 baud;
    volatile u32 cpnd;
    volatile u32 dma_cnt;
    volatile u32 dma_adr;
} despi_typedef;

#endif

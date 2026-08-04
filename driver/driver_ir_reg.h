/*
 * @File name    : driver_ir_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-11
 * @Description  : This file contains all the register definition for the IR.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */

#ifndef _DRIVER_IR_REG_H_
#define _DRIVER_IR_REG_H_

#define IRTX_BASE                           (&IRONETIME)
#define IRRX_BASE                           (&IRRXCON)

#define IRTX_REG                            ((irtx_typedef *) IRTX_BASE)
#define IRRX_REG                            ((irrx_typedef *) IRRX_BASE)

/*---------------------------------------------------------------------------------*/
/*                                      IRTX                                       */
/*---------------------------------------------------------------------------------*/
/********************* Bit definition for IRTXCON register ********************/
#define IRTX_CON_IRCW_DUTY_MASK             ((uint32_t)0x0ff00000)
#define IRTX_CON_IRCW_DUTY_LSB              20
#define IRTX_CON_IRCW_LENGTH_MASK           ((uint32_t)0x000ff000)
#define IRTX_CON_IRCW_LENGTH_LSB            12
#define IRTX_CON_IRTX_CARRIER_EN_BIT        ((uint32_t)0x00000800)
#define IRTX_CON_IRTX_CARRIER_EN_POS        11
#define IRTX_CON_IREDGE_SEL                 ((uint32_t)0x00000400)
#define IRTX_CON_IRTX_SEL_MASK              ((uint32_t)0x00000300)
#define IRTX_CON_IRTX_SEL_LSB               8
#define IRTX_CON_IRTX_IE                    ((uint32_t)0x00000080)
#define IRTX_CON_IR_CAPEN_BIT               ((uint32_t)0x00000040)
#define IRTX_CON_IR_CAPEN_POS               6
#define IRTX_CON_IRTX_1MODE_BIT             ((uint32_t)0x00000020)
#define IRTX_CON_IRTX_1MODE_POS             5
#define IRTX_CON_IRTX_0MODE_BIT             ((uint32_t)0x00000010)
#define IRTX_CON_IRTX_0MODE_POS             4
#define IRTX_CON_IRTX_INV                   ((uint32_t)0x00000008)
#define IRTX_CON_IRTX_KST_BIT               ((uint32_t)0x00000004)
#define IRTX_CON_IRTX_KST_POS               2
#define IRTX_CON_IRTXLEN_EN                 ((uint32_t)0x00000002)
#define IRTX_CON_IRTXMOD_EN                 ((uint32_t)0x00000001)

/********************* Bit definition for IRONETIME register ********************/
#define IRTX_ONETIME_ONE_MARK_MASK          ((uint32_t)0xffff0000)
#define IRTX_ONETIME_ONE_MARK_LSB           16
#define IRTX_ONETIME_ONE_SPACE_MASK         ((uint32_t)0x0000ffff)
#define IRTX_ONETIME_ONE_SPACE_LSB          0

/********************* Bit definition for IRZEROTIME register *******************/
#define IRTX_ZEROTIME_ZERO_MARK_MASK        ((uint32_t)0xffff0000)
#define IRTX_ZEROTIME_ZERO_MARK_LSB         16
#define IRTX_ZEROTIME_ZERO_SPACE_MASK       ((uint32_t)0x0000ffff)
#define IRTX_ZEROTIME_ZERO_SPACE_LSB        0

/********************* Bit definition for IRSTARTTIME register ******************/
#define IRTX_STARTTIME_START1TIME_MASK      ((uint32_t)0xffff0000)
#define IRTX_STARTTIME_START1TIME_LSB       16
#define IRTX_STARTTIME_START0TIME_MASK      ((uint32_t)0x0000ffff)
#define IRTX_STARTTIME_START0TIME_LSB       0

/********************* Bit definition for IRREPEATTIME register ******************/
#define IRTX_STARTTIME_REPEAT1TIME_MASK     ((uint32_t)0xffff0000)
#define IRTX_STARTTIME_REPEAT1TIME_LSB      16
#define IRTX_STARTTIME_REPEAT0TIME_MASK     ((uint32_t)0x0000ffff)
#define IRTX_STARTTIME_REPEAT0TIME_LSB      0

/********************* Bit definition for IRREPEATCON register ******************/
#define IRTX_IRREPEATCON_IRREPTIME_MASK     ((uint32_t)0xffffe000)
#define IRTX_IRREPEATCON_IRREPTIME_LSB      13
#define IRTX_IRREPEATCON_IRENDTIME_MASK     ((uint32_t)0x00001ff8)
#define IRTX_IRREPEATCON_IRENDTIME_LSB      3
#define IRTX_IRREPEATCON_IRTX_SYC_BIT       ((uint32_t)0x00000004)
#define IRTX_IRREPEATCON_IRTX_SYC_POS       2
#define IRTX_IRREPEATCON_IRREP_SEL_BIT      ((uint32_t)0x00000002)
#define IRTX_IRREPEATCON_IRREP_SEL_POS      1
#define IRTX_IRREPEATCON_IRTX_KEY           ((uint32_t)0x00000001)

/********************* Bit definition for IRTXDATA register ******************/
#define IRTX_IRTXDATA                       ((uint32_t)0xffffffff)

/********************* Bit definition for IRTXLEN register ******************/
#define IRTX_IRTXLEN                        ((uint32_t)0xffffffff)

/********************* Bit definition for IRTXPEND register ******************/
#define IRTX_IRTX_PND                       ((uint32_t)0x00000001)

/********************* Bit definition for IRDMACON register ******************/
#define IRDMACON_ODMA_HPND_IE               ((uint32_t)0x00000040)
#define IRDMACON_ODMA_APND_IE               ((uint32_t)0x00000020)
#define IRDMACON_ODMA_EN_BIT                ((uint32_t)0x00000010)
#define IRDMACON_ODMA_EN_POS                4
#define IRDMACON_IDMA_HPND_IE               ((uint32_t)0x00000004)
#define IRDMACON_IDMA_APND_IE               ((uint32_t)0x00000002)
#define IRDMACON_IDMA_EN_BIT                ((uint32_t)0x00000001)
#define IRDMACON_IDMA_EN_POS                0

/********************* Bit definition for IRDMAISIZE register ******************/
#define IRDMAISIZE_DMA_ICNT                   ((uint32_t)0xffff0000)
#define IRDMAISIZE_DMA_ISIZE                  ((uint32_t)0x0000ffff)

/********************* Bit definition for IRDMAIADR register ******************/
#define IRDMAIADR_DMA_IADR                    ((uint32_t)0xffffffff)

/********************* Bit definition for IRDMAOSIZE register ******************/
#define IRDMAOSIZE_DMA_OCNT                   ((uint32_t)0xffff0000)
#define IRDMAOSIZE_DMA_OSIZE                  ((uint32_t)0x0000ffff)

/********************* Bit definition for IRDMAOADR register ******************/
#define IRDMAOADR_DMA_OADR                    ((uint32_t)0xffffffff)

/********************* Bit definition for IRDMAPEND register ******************/
#define IRDMAPEND_ODMA_HPND                   ((uint32_t)0x00020000)
#define IRDMAPEND_ODMA_APND                   ((uint32_t)0x00010000)
#define IRDMAPEND_IDMA_HPND                   ((uint32_t)0x00000002)
#define IRDMAPEND_IDMA_APND                   ((uint32_t)0x00000001)

/*---------------------------------------------------------------------------------*/
/*                                      IRRX                                       */
/*---------------------------------------------------------------------------------*/
/********************* Bit definition for IRRFCON register ********************/
#define IRRX_CON_KEYRELS                      ((uint32_t)0x00020000)
#define IRRX_CON_RXPND                        ((uint32_t)0x00010000)
#define IRRX_CON_IRWKEN                       ((uint32_t)0x00000010)
#define IRRX_CON_IRRXSEL                      ((uint32_t)0x00000004)
#define IRRX_CON_IRIE                         ((uint32_t)0x00000002)
#define IRRX_CON_IREN                         ((uint32_t)0x00000001)

/********************* Bit definition for IRRXCPND register ********************/
#define IRRX_CPND_CRELSPND                    ((uint32_t)0x00020000)
#define IRRX_CPND_CRXPND                      ((uint32_t)0x00010000)

/********************* Bit definition for IRRXDAT register ********************/
#define IRRX_RXDAT                            ((uint32_t)0xffffffff)

/********************* Bit definition for IRRXPR0 register ********************/
#define IRRX_PR0_RPTPR_MASK                   ((uint32_t)0x7fff0000)
#define IRRX_PR0_RPTPR_LSB                    16
#define IRRX_PR0_DATPR_MASK                   ((uint32_t)0x00007fff)
#define IRRX_PR0_DATPR_LSB                    0

/********************* Bit definition for IRRXPR1 register ********************/
#define IRRX_PR1_ONEPR_MASK                   ((uint32_t)0x7fff0000)
#define IRRX_PR1_ONEPR_LSB                    16
#define IRRX_PR1_ZEROPR                       ((uint32_t)0x00007fff)
#define IRRX_PR1_ZEROPR_LSB                   0

/********************* Bit definition for IRRXERR0 register ********************/
#define IRRX_ERR0_RPTPR_MASK                  ((uint32_t)0x0fff0000)
#define IRRX_ERR0_RPTPR_LSB                   16
#define IRRX_ERR0_DATPR_MASK                  ((uint32_t)0x00000fff)
#define IRRX_ERR0_DATPR_LSB                   0

/********************* Bit definition for IRRXERR1 register ********************/
#define IRRX_ERR1_TOPR_MASK                   ((uint32_t)0xfff00000)
#define IRRX_ERR1_TOPR_LSB                    20
#define IRRX_ERR1_ONEPR_MASK                  ((uint32_t)0x000ffc00)
#define IRRX_ERR1_ONEPR_LSB                   10
#define IRRX_ERR1_ZEROPR_MASK                 ((uint32_t)0x000003ff)
#define IRRX_ERR1_ZEROPR_LSB                  0

/********************* Bit definition for IRFLTCON register ********************/
#define IRFLT_CON_LEN_MASK                    ((uint32_t)0x0000ff00)
#define IRFLT_CON_LEN_LSB                     8
#define IRFLT_CON_SRCS                        ((uint32_t)0x00000002)
#define IRFLT_CON_EN_BIT                      ((uint32_t)0x00000001)
#define IRFLT_CON_EN_POS                      0



/**
 * @brief IRRX Register Structure
 */
typedef struct {
    // IR RX Control Register
    volatile u32 con;
    // IR RX Data Register
    volatile u32 rxdata;
    // IR RX Clear Pending Register
    volatile u32 cpnd;
    // IR RX Error Config Register0
    volatile u32 err0;
    // IR RX Error Config Register1
    volatile u32 err1;
    // IR RX Period Config Register0
    volatile u32 period0;
    // IR RX Period Config Register1
    volatile u32 period1;
} irrx_typedef;

/**
 * @brief IRTX Register Structure
 */
typedef struct {
    // IR One Code Time Register
    volatile u32 onetime;
    // IR One Zero Time Register
    volatile u32 zerotime;
    // IR Start Code Time Register
    volatile u32 starttime;
    // IR Repeat Code Time Register
    volatile u32 repeattime;
    // IR Repeat Code Control Register
    volatile u32 repeatcon;
    // IR TX Control Register
    volatile u32 con;
    // IR TX Data Register
    volatile u32 txdata;
    // IR TX Data Length Register
    volatile u32 txlen;
    // IR TX Pending Register
    volatile u32 pend;
    // IR TX DMA Control Register
    volatile u32 dma_con;
    // IR TX DMA Input Address Register
    volatile u32 dma_iadr;
    // IR TX DMA Input Size Register
    volatile u32 dma_isize;
    // IR TX DMA Output Address Register
    volatile u32 dma_oadr;
    // IR TX DMA Output Size Register
    volatile u32 dma_osize;
    // IR TX DMA Pending Register
    volatile u32 dma_pend;
} irtx_typedef;

#endif

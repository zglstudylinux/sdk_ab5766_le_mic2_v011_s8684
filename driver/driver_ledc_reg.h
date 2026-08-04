/*
 * @File name    : driver_ledc_reg.h
 * @Author       : Bluetrum IOT Team K.
 * @Date         : 2025-08-27
 * @Description  : This file contains all the register definition for the LEDC.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_LEDC_REG_H
#define _DRIVER_LEDC_REG_H

#define LEDC_BASE                           (&LEDCCON)

#define LEDC_REG                            ((ledc_typedef *) LEDC_BASE)

/*---------------------------------------------------------------------------------*/
/*                                    LEDC                                         */
/*---------------------------------------------------------------------------------*/
/*********************** Bit definition for LEDCCON register ***********************/
#define LEDC_CON_DMA_INT_EN                 ((uint32_t)0x04000000)
#define LEDC_CON_OUT_INV                    ((uint32_t)0x02000000)
#define LEDC_CON_RST_INT_EN                 ((uint32_t)0x01000000)

#define LEDC_CON_BAUD_MASK                  ((uint32_t)0x00ff0000)
#define LEDC_CON_BAUD_LSB                   16

#define LEDC_CON_DELAY_MASK                 ((uint32_t)0x0000ff00)
#define LEDC_CON_DELAY_LSB                  8

#define LEDC_CON_IN_FORMAT                  ((uint32_t)0x00000080)
#define LEDC_CON_INT_EN                     ((uint32_t)0x00000040)
#define LEDC_CON_IDLE_LEVEL                 ((uint32_t)0x00000020)
#define LEDC_CON_VLD_INV                    ((uint32_t)0x00000010)

#define LEDC_CON_BYTE_INV                   ((uint32_t)0x0000000c)
#define LEDC_CON_LEDC_KICK                  ((uint32_t)0x00000002)
#define LEDC_CON_EN                         ((uint32_t)0x00000001)

/*********************** Bit definition for LEDCPEND register **********************/
#define LEDC_PEND_DMA_PND                   ((uint32_t)0x00000004)
#define LEDC_PEND_RST_PND                   ((uint32_t)0x00000002)
#define LEDC_PEND_LEDC_PND                  ((uint32_t)0x00000001)

/*********************** Bit definition for LEDCTIX register **********************/
#define LEDC_TIX_T0L_LSB                    0
#define LEDC_TIX_T0H_LSB                    8
#define LEDC_TIX_T1L_LSB                    16
#define LEDC_TIX_T1H_LSB                    24

/*********************** Bit definition for LEDCRSTX register **********************/
#define LEDC_RSTX_RSTH_LSB                  0
#define LEDC_RSTX_RSTL_LSB                  16

/*********************** Bit definition for LEDCCNT register **********************/
#define LEDC_CNT_DMA_KST                    ((uint32_t)0x80000000)


/**
 * @brief LEDC Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 fd;
    volatile u32 lp;
    volatile u32 tix;
    volatile u32 rstx;
    volatile u32 addr;
    volatile u32 cnt;
    volatile u32 pend;
} ledc_typedef;

#endif

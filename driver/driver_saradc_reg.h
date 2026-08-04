/*
 * @File name    : driver_saradc_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the functions prototypes for the SARADC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SARADC_REG_H
#define _DRIVER_SARADC_REG_H

#define SARADC_BASE                         (&SADCDAT0)

#define SARADC_REG                          ((saradc_typedef *) SARADC_BASE)

/*---------------------------------------------------------------------------------*/
/*                                    SARADC                                       */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for SADCCON register **********************/
#define SADCCON_CH0PUEN                     ((uint32_t)0x00000001)
#define SADCCON_CH1PUEN                     ((uint32_t)0x00000002)
#define SADCCON_CH2PUEN                     ((uint32_t)0x00000004)
#define SADCCON_CH3PUEN                     ((uint32_t)0x00000008)
#define SADCCON_CH4PUEN                     ((uint32_t)0x00000010)
#define SADCCON_CH5PUEN                     ((uint32_t)0x00000020)
#define SADCCON_CH6PUEN                     ((uint32_t)0x00000040)
#define SADCCON_CH7PUEN                     ((uint32_t)0x00000080)
#define SADCCON_CH8PUEN                     ((uint32_t)0x00000100)
#define SADCCON_CH9PUEN                     ((uint32_t)0x00000200)
#define SADCCON_CH10PUEN                    ((uint32_t)0x00000400)
#define SADCCON_CH11PUEN                    ((uint32_t)0x00000800)
#define SADCCON_CH12PUEN                    ((uint32_t)0x00001000)
#define SADCCON_CH13PUEN                    ((uint32_t)0x00002000)
#define SADCCON_CH14PUEN                    ((uint32_t)0x00004000)
#define SADCCON_CH15PUEN                    ((uint32_t)0x00008000)
#define SADCCON_ADCEN                       ((uint32_t)0x00010000)
#define SADCCON_ADCIE                       ((uint32_t)0x00020000)
#define SADCCON_ADCANGIOEN                  ((uint32_t)0x00040000)
#define SADCCON_ADCAEN                      ((uint32_t)0x00080000)
#define SADCCON_ADCMODE                     ((uint32_t)0x00100000)

/************************ Bit definition for SADCST register **********************/
#define SADCST_CH_ST_MASK                   ((uint32_t)0x00000003)
#define SADCST_CH_ST_LEN                    2
#define SADCST_CH_ST_OFFSET                 0

/************************ Bit definition for SADCCH register **********************/
#define SADCCH_ADCPND                       ((uint32_t)0x00010000)

/**
 * @brief SARADC Register Structure
 */
typedef struct {
    volatile u32 dat[16];
    volatile u32 con;
    volatile u32 ch;
    volatile u32 cst;
    volatile u32 baud;
} saradc_typedef;

#endif

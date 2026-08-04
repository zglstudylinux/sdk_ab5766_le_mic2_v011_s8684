/*
 * @File name    : driver_sdadc_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the register definition for the SDADC.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#ifndef _DRIVER_SDADC_REG_H
#define _DRIVER_SDADC_REG_H

#define SDADC_BASE                          (&SDADCCON)

#define SDADC_REG                           ((sdadc_typedef *) SDADC_BASE)

/*---------------------------------------------------------------------------------*/
/*                                    SDADC                                        */
/*---------------------------------------------------------------------------------*/
/*********************** Bit definition for SDADCCON register *********************/
#define SDADCCON_SWCLK_DUTY                 ((uint32_t)0xffc00000)
#define SDADCCON_SWCLK_PERI                 ((uint32_t)0x003ff000)
#define SDADCCON_SWCLK_EN                   ((uint32_t)0x00000800)
#define SDADCCON_ADC0_EN                    ((uint32_t)0x00000002)
#define SDADCCON_TOTAL_EN                   ((uint32_t)0x00000001)

/*********************** Bit definition for SDADC0CON register *********************/
#define SDADC0CON_INV_EN                    ((uint32_t)0x00400000)
#define SDADC0CON_N6DB_EN                   ((uint32_t)0x00200000)
#define SDADC0CON_BITS_SEL                  ((uint32_t)0x001e0000)
#define SDADC0CON_FSOUT_SEL                 ((uint32_t)0x0000000f)

/*********************** Bit definition for SDADC0GAIN register *********************/
#define SDADC0GAIN_SW_DC                    ((uint32_t)0xffff0000)
#define SDADC0GAIN_ADC_GAIN                 ((uint32_t)0x0000ffff)

/*********************** Bit definition for SDADC0RMDCCON register *********************/
#define SDADC0RMDCCON_GET_DC_OUT            ((uint32_t)0xffffff00)
#define SDADC0RMDCCON_SW_DC                 ((uint32_t)0xffffff00)
#define SDADC0RMDCCON_GETDC_FLAG            ((uint32_t)0x00000080)
#define SDADC0RMDCCON_GETDC_EN              ((uint32_t)0x00000040)
#define SDADC0RMDCCON_SW_DC_EN              ((uint32_t)0x00000020)
#define SDADC0RMDCCON_RMDC_SEL              ((uint32_t)0x0000001c)
#define SDADC0RMDCCON_RMDC_ORDER            ((uint32_t)0x00000002)
#define SDADC0RMDCCON_RMDC_EN               ((uint32_t)0x00000001)

/*********************** Bit definition for SDADC0DMACON register *********************/
#define SDADC0DMACON_DMA_SAMPLE_IE          ((uint32_t)0x00000100)
#define SDADC0DMACON_DMA_HALF_IE            ((uint32_t)0x00000080)
#define SDADC0DMACON_DMA_ALL_IE             ((uint32_t)0x00000040)
#define SDADC0DMACON_DMA_DATA_MODE          ((uint32_t)0x00000004)
#define SDADC0DMACON_DMA_EN                 ((uint32_t)0x00000001)

/*********************** Bit definition for SDADC0DMAADDR register *********************/
#define SDADC0DMAADDR_DMA_ADDR              ((uint32_t)0xffffffff)

/*********************** Bit definition for SDADC0DMASIZE register *********************/
#define SDADC0DMASIZE_ADC_DOUT              ((uint32_t)0xffff0000)
#define SDADC0DMASIZE_DMA_SIZE              ((uint32_t)0x0000ffff)

/*********************** Bit definition for SDADCDMAFLAG register *********************/
#define SDADCDMAFLAG_DMA_SAMPLE_DONE        ((uint32_t)0x00000004)
#define SDADCDMAFLAG_DMA_HALF_DONE          ((uint32_t)0x00000002)
#define SDADCDMAFLAG_DMA_ALL_DONE           ((uint32_t)0x00000001)

/*********************** Bit definition for SDADCDMACLR register *********************/
#define SDADCDMACLR_DMA_SAMPLE_DONE         ((uint32_t)0x00000004)
#define SDADCDMACLR_DMA_HALF_DONE           ((uint32_t)0x00000002)
#define SDADCDMACLR_DMA_ALL_DONE            ((uint32_t)0x00000001)

/*********************** Bit definition for SDADCDCCON0 register *********************/
#define SDADCDCCON0_ANA_CFG                 ((uint32_t)0x40000000)
#define SDADCDCCON0_OUTPUT_FLAG             ((uint32_t)0x00008000)
#define SDADCDCCON0_SIGN_SEL                ((uint32_t)0x00004000)
#define SDADCDCCON0_DONE_CNT                ((uint32_t)0x00003c00)
#define SDADCDCCON0_CH_EN                   ((uint32_t)0x000003e0)
#define SDADCDCCON0_DC_FSOUT_SEL            ((uint32_t)0x0000001c)
#define SDADCDCCON0_AUTO_KICK               ((uint32_t)0x00000002)
#define SDADCDCCON0_DC_MODE                 ((uint32_t)0x00000001)

/*********************** Bit definition for SDADCDCCON1 register *********************/
#define SDADCDCCON1_THRESHOLD               ((uint32_t)0xffff0000)
#define SDADCDCCON1_HVIO_HOLD               ((uint32_t)0x00008000)
#define SDADCDCCON1_CMP_PND_CLR             ((uint32_t)0x00001000)
#define SDADCDCCON1_CMP_PND                 ((uint32_t)0x00000800)
#define SDADCDCCON1_SMP_PND_CLR             ((uint32_t)0x00000400)
#define SDADCDCCON1_SMP_PND                 ((uint32_t)0x00000200)
#define SDADCDCCON1_SMP_IE                  ((uint32_t)0x00000100)
#define SDADCDCCON1_CMP_IE                  ((uint32_t)0x00000080)
#define SDADCDCCON1_CMP_DIR                 ((uint32_t)0x00000040)
#define SDADCDCCON1_CH_CMP_EN               ((uint32_t)0x0000003e)
#define SDADCDCCON1_DC_MANUAL_KICK          ((uint32_t)0x00000001)

/**
 * @brief SDADC Register Structure
 */
typedef struct {
    volatile u32 con;
    //audio mic
    volatile u32 adc0_con;
    volatile u32 adc0_gain_con;
    volatile u32 adc0_rm_dc_con;

    //common
    volatile u32 adc0_dma_con;
    volatile u32 adc0_dma_addr;
    volatile u32 adc0_dma_size;
    volatile u32 dma_pend;
    volatile u32 dma_cpend;

    //dc measurement
    volatile u32 dc_con0;
    volatile u32 dc_con1;
    volatile u32 dc_ang_con;
    volatile u32 ram_test_addr;
    volatile u32 ram_test_data;
    volatile u32 dc_data0;
    volatile u32 dc_data1;
    volatile u32 dc_data2;
    volatile u32 dc_data3;
    volatile u32 dc_data4;
} sdadc_typedef;

#endif

/*
 * @File name    : driver_touch_key_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the functions prototypes for the Touch library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */

#ifndef _TOUCH_KEY_REG_H_
#define _TOUCH_KEY_REG_H_

#define TOUCH_KEY_BASE                      (&TKCON)

#define TOUCH_KEY_REG                       ((touch_key_typedef *) TOUCH_KEY_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     TOUCH KEY                                   */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for INPCON register **********************/
#define INPCON_TKNUM                        ((uint32_t)0xC0000000)
#define INPCON_PTOPND                       ((uint32_t)0x20000000)
#define INPCON_TELPND                       ((uint32_t)0x10000000)
#define INPCON_TESPND                       ((uint32_t)0x08000000)
#define INPCON_TEBCNTPND                    ((uint32_t)0x04000000)
#define INPCON_TESTA                        ((uint32_t)0x02000000)
#define INPCON_TKTOPND                      ((uint32_t)0x01000000)
#define INPCON_TKRPND                       ((uint32_t)0x00800000)
#define INPCON_TKLPND                       ((uint32_t)0x00400000)
#define INPCON_TKSPND                       ((uint32_t)0x00200000)
#define INPCON_TKBCNTPND                    ((uint32_t)0x00100000)
#define INPCON_TKSTA                        ((uint32_t)0x00080000)
#define INPCON_TKERRPND                     ((uint32_t)0x00040000)
#define INPCON_TKOVPND                      ((uint32_t)0x00020000)
#define INPCON_TKPND                        ((uint32_t)0x00010000)
#define INPCON_TKVPND                       ((uint32_t)0x00008000)
#define INPCON_TKVPNDIE                     ((uint32_t)0x00004000)
#define INPCON_PTOPNDIE                     ((uint32_t)0x00002000)
#define INPCON_TELPNDIE                     ((uint32_t)0x00001000)
#define INPCON_TESPNDIE                     ((uint32_t)0x00000800)
#define INPCON_TEBCNTPNDI                   ((uint32_t)0x00000400)
#define INPCON_TESTAIE                      ((uint32_t)0x00000200)
#define INPCON_TKTOPNDIE                    ((uint32_t)0x00000100)
#define INPCON_TKRPNDIE                     ((uint32_t)0x00000080)
#define INPCON_TKLPNDIE                     ((uint32_t)0x00000040)
#define INPCON_TKSPNDIE                     ((uint32_t)0x00000020)
#define INPCON_TKBCNTPNDIE                  ((uint32_t)0x00000010)
#define INPCON_TSKTAIE                      ((uint32_t)0x00000008)
#define INPCON_TKERRPNDIE                   ((uint32_t)0x00000004)
#define INPCON_TKOVPNDIE                    ((uint32_t)0x00000002)
#define INPCON_TKPNDIE                      ((uint32_t)0x00000001)


/************************ Bit definition for INPCPND register **********************/
#define INPCPND_CTEEPND                     ((uint32_t)0x08000000)
#define INPCPND_CTEBPND                     ((uint32_t)0x04000000)
#define INPCPND_CTESTA                      ((uint32_t)0x02000000)
#define INPCPND_CTKEPND                     ((uint32_t)0x00200000)
#define INPCPND_CTKBPND                     ((uint32_t)0x00100000)
#define INPCPND_CTKSTA                      ((uint32_t)0x00080000)
#define INPCPND_CTKPND                      ((uint32_t)0x00010000)
#define INPCPND_CTKVPND                     ((uint32_t)0x00008000)

/************************ Bit definition for TKCON register **********************/
#define TKCON_TERUPDBEN                     ((uint32_t)0x80000000)
#define TKCON_TELUPDBEN                     ((uint32_t)0x40000000)
#define TKCON_TESUPDBEN                     ((uint32_t)0x20000000)
#define TKCON_TEBCNTVAL                     ((uint32_t)0x10000000)
#define TKCON_TKBCNTVAL                     ((uint32_t)0x08000000)
#define TKCON_TKTOUPDBEN                    ((uint32_t)0x04000000)
#define TKCON_TKRUPDBEN                     ((uint32_t)0x02000000)
#define TKCON_TKLUPDBEN                     ((uint32_t)0x01000000)
#define TKCON_TKSUPDBEN                     ((uint32_t)0x00800000)
#define TKCON_TESTAWKEN                     ((uint32_t)0x00400000)
#define TKCON_TKSTAWKEN                     ((uint32_t)0x00200000)
#define TKCON_TKPNDWKEN                     ((uint32_t)0x00100000)
#define TKCON_TKPTOUPDBEN                   ((uint32_t)0x00080000)
#define TKCON_TKMODE                        ((uint32_t)0x00040000)
#define TKCON_TKSWSEL                       ((uint32_t)0x00020000)
#define TKCON_TKCH_FIX                      ((uint32_t)0x00010000)
#define TKCON_TKRST_DIS                     ((uint32_t)0x00008000)
#define TKCON_TKAEN_ATSEL                   ((uint32_t)0x00004000)
#define TKCON_TK3EN                         ((uint32_t)0x00002000)
#define TKCON_TK2EN                         ((uint32_t)0x00001000)
#define TKCON_TK1EN                         ((uint32_t)0x00000800)
#define TKCON_TK0EN                         ((uint32_t)0x00000400)
#define TKCON_TK3PNDEN                      ((uint32_t)0x00000200)
#define TKCON_TK2PNDEN                      ((uint32_t)0x00000100)
#define TKCON_TK1PNDEN                      ((uint32_t)0x00000080)
#define TKCON_TK0PNDEN                      ((uint32_t)0x00000040)
#define TKCON_TKSEL                         ((uint32_t)0x00000030)
#define TKCON_TESEL                         ((uint32_t)0x0000000C)
#define TKCON_BCNTSEL                       ((uint32_t)0x00000002)
#define TKCON_TKEN                          ((uint32_t)0x00000001)

/************************ Bit definition for TKCON1 register **********************/
#define TKCON1_TKKST                        ((uint32_t)0x80000000)
#define TKCON1_TEBCNTVALSEL                 ((uint32_t)0x40000000)
#define TKCON1_TKBCNTVALSEL                 ((uint32_t)0x10000000)
#define TKCON1_CNT_SEL                      ((uint32_t)0x04000000)
#define TKCON1_DIV2SEL                      ((uint32_t)0x02000000)
#define TKCON1_TECEN                        ((uint32_t)0x01000000)
#define TKCON1_TKCEN                        ((uint32_t)0x00800000)
#define TKCON1_TKVCEN                       ((uint32_t)0x00400000)
#define TKCON1_TKCLK2MDIV                   ((uint32_t)0x001F0000)
#define TKCON1_TK_CHSEL_SW                  ((uint32_t)0x0000C000)
#define TKCON1_TKAEN_SW                     ((uint32_t)0x00002000)
#define TKCON1_TKPWUP                       ((uint32_t)0x00001F80)
#define TKCON1_TKARTRIM                     ((uint32_t)0x00000007)

/************************ Bit definition for TKCON2 register **********************/
#define TKCON2_TO_BCNT_THD                  ((uint32_t)0xF0000000)
#define TKCON2_RELS_VARI_THD                ((uint32_t)0x0FFF0000)
#define TKCON2_PRESS_VARI_THD               ((uint32_t)0x0000FFF0)
#define TKCON2_RELS_FIL_SEL                 ((uint32_t)0x00000008)
#define TKCON2_PRESS_FIL_SEL                ((uint32_t)0x00000004)
#define TKCON2_RELS_VARI_EN                 ((uint32_t)0x00000002)
#define TKCON2_PRESS_VARI_EN                ((uint32_t)0x00000001)

/************************ Bit definition for TKACON0 register **********************/
#define TKACON0_TK_ENCHN1                  ((uint32_t)0x00800000)
#define TKACON0_TK_ITRIM1                  ((uint32_t)0x007C0000)
#define TKACON0_TK_CTRIM1                  ((uint32_t)0x0001F000)
#define TKACON0_TK_ENCHN0                  ((uint32_t)0x00000800)
#define TKACON0_TK_ITRIM0                  ((uint32_t)0x000007C0)
#define TKACON0_TK_CTRIM0                  ((uint32_t)0x0000001F)

/************************ Bit definition for TKACON1 register **********************/
#define TKACON1_TK_ENCHN3                  ((uint32_t)0x00800000)
#define TKACON1_TK_ITRIM3                  ((uint32_t)0x007C0000)
#define TKACON1_TK_CTRIM3                  ((uint32_t)0x0001F000)
#define TKACON1_TK_ENCHN2                  ((uint32_t)0x00000800)
#define TKACON1_TK_ITRIM2                  ((uint32_t)0x000007C0)
#define TKACON1_TK_CTRIM2                  ((uint32_t)0x0000001F)

/************************ Bit definition for TKCNT register **********************/
#define TKCNT_CNT                          ((uint32_t)0x00000FFF)

/************************ Bit definition for TKCDPR register **********************/
#define TKCDPR_TKCDPR1                     ((uint32_t)0x03FF0000)
#define TKCDPR_TKCDPR0                     ((uint32_t)0x000003FF)

/************************ Bit definition for TKCDPR1 register **********************/
#define TKCDPR1_TKCDPR3                    ((uint32_t)0x03FF0000)
#define TKCDPR1_TKCDPR2                    ((uint32_t)0x000003FF)

/************************ Bit definition for TKTMR register **********************/
#define TKTMR_TO_EXCEPT                    ((uint32_t)0xFFF00000)
#define TKTMR_FIL_VAL                      ((uint32_t)0x000F0000)
#define TKTMR_FIL_EXCEPT                   ((uint32_t)0x0000FF00)
#define TKTMR_FIL_HIGH                     ((uint32_t)0x000000F0)
#define TKTMR_FIL_LOW                      ((uint32_t)0x0000000F)

/************************ Bit definition for TKBCNT register **********************/
#define TKBCNT_CNT_AVG                     ((uint32_t)0x00FFF000)
#define TKBCNT_BCNT                        ((uint32_t)0x00000FFF)

/************************ Bit definition for TKPTHD register **********************/
#define TKPTHD_PTO_EXCEPT                  ((uint32_t)0xF0000000)
#define TKPTHD_TKRTHD                      ((uint32_t)0x0FFF0000)
#define TKPTHD_TKPTHD                      ((uint32_t)0x00000FFF)

/************************ Bit definition for TKVARI register **********************/
#define TKVARI_VARI                        ((uint32_t)0xFFFF0000)
#define TKVARI_VTHD                        ((uint32_t)0x0000FFFF)

/************************ Bit definition for TKVARITHD (IN RANGE) register **********************/
#define TKVARITHD_TKBADD                   ((uint32_t)0xFF000000)  //IN RANGE add vaule
#define TKVARITHD_TKFIL                    ((uint32_t)0x00FF0000)  //IN RANGE filter
#define TKVARITHD_TKAETHD                  ((uint32_t)0x0000F000)  //IN RANGE equal threshould (min)
#define TKVARITHD_TKARTHD                  ((uint32_t)0x00000FFF)  //IN RANGE range threshould (max)



/**
 * @brief Touch Key Register Structure
 */
typedef struct {
    // Touch Key Control Register
    volatile u32 tkcon;
    // Touch Key Control Register 1
    volatile u32 tkcon1;
    // Touch Key Counter Register
    volatile u32 tkcnt;
    // Touch Key Charge And Discharge Period Register 0
    volatile u32 tkcdpr0;
    // Touch Key Charge And Discharge Period Register 1
    volatile u32 tkcdpr1;
    // Touch Key Times Register
    volatile u32 tktmr;
    // Touch Ear Times Register
    volatile u32 tetmr;
    // Touch Key Base Counter Register
    volatile u32 tkbcnt;
    // Touch Key Press Threshold Register
    volatile u32 tkpthd;
    // Touch Key Exception Threshold Register
    volatile u32 tkethd;
    // Touch Ear Base Counter Register
    volatile u32 tebcnt;
    // Touch Ear Press Threshold Register
    volatile u32 tepthd;
    // Touch Ear Exception Threshold Register
    volatile u32 teethd;
    // Touch Key Control Register 2
    volatile u32 tkcon2;
    // Touch Key Variance Register
    volatile u32 tkvari;
    // Touch Key Variance Threshold Register
    volatile u32 tkvarithd;
    // Touch Key Analog Control Register 0
    volatile u32 tkacon0;
    // Touch Key Analog Control Register 1
    volatile u32 tkacon1;
} touch_key_typedef;

#endif

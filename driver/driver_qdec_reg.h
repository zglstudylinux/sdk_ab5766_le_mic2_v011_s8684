/*
 * @File name    : driver_ledc_reg.h
 * @Author       : Bluetrum IOT Team K.
 * @Date         : 2025-08-27
 * @Description  : This file contains all the register definition for the QDEC.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef __DRIVER_QDEC_REG_H__
#define __DRIVER_QDEC_REG_H__

#define QDECX_BASE                          (SFR6_BASE + 0x20*4)

#define QDECX_REG                           ((qdec_typedef *)QDECX_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     QDEC                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for QDECxCON register **********************/
#define QDECCON_REVERSE_PND                 ((uint32_t)0x80000000)
#define QDECCON_FORWARD_PND                 ((uint32_t)0x40000000)
#define QDECCON_DAT_CLR                     ((uint32_t)0x00400000)
#define QDECCON_OV_PND                      ((uint32_t)0x00200000)
#define QDECCON_SMP_PND                     ((uint32_t)0x00100000)
#define QDECxCON_SMP                        ((uint32_t)0x000F0000)
#define QDECxCON_TIME_MODE                  ((uint32_t)0x00008000)
#define QDECCON_TERR_PND                    ((uint32_t)0x00004000)
#define QDECxCON_CM                         ((uint32_t)0x00003000)
#define QDECxCON_FIL_LEN                    ((uint32_t)0x00000FE0)
#define QDECxCON_WKUP_EN                    ((uint32_t)0x00000010)
#define QDECCON_SMPIE                       ((uint32_t)0x00000008)
#define QDECCON_RDIE                        ((uint32_t)0x00000004)
#define QDECCON_FDIE                        ((uint32_t)0x00000002)
#define QDECCON_EN                          ((uint32_t)0x00000001)

/**
 * @brief Quadrature Decoder Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 cpnd;
    volatile u32 dat;
} qdec_typedef;

#endif

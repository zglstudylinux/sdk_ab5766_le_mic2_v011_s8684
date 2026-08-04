/*
 * @File name    : driver_pwm_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-15
 * @Description  : This file contains all the functions prototypes for the TMR library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_PWM_REG_H
#define _DRIVER_PWM_REG_H

#define PWM_TMR0_BASE                       (SFR9_BASE + 0x29*4)
#define PWM_TMR1_BASE                       (SFR9_BASE + 0x2e*4)
#define PWM_TMR2_BASE                       (SFR9_BASE + 0x33*4)
#define PWM_TMR3_BASE                       (SFR9_BASE + 0x38*4)
#define PWM_BASE                            (SFR9_BASE + 0x3d*4)
#define PWM_DZ0_BASE                        (SFR9_BASE + 0x3e*4)
#define PWM_DZ1_BASE                        (SFR9_BASE + 0x3f*4)

#define PWM_REG                             ((pwm_typedef *) PWM_BASE)
#define PWM_DZ0_REG                         ((pwm_dz_typedef *) PWM_DZ0_BASE)
#define PWM_DZ1_REG                         ((pwm_dz_typedef *) PWM_DZ1_BASE)
#define PWM_TMR0_REG                        ((pwm_tmr_typedef *) PWM_TMR0_BASE)
#define PWM_TMR1_REG                        ((pwm_tmr_typedef *) PWM_TMR1_BASE)
#define PWM_TMR2_REG                        ((pwm_tmr_typedef *) PWM_TMR2_BASE)
#define PWM_TMR3_REG                        ((pwm_tmr_typedef *) PWM_TMR3_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     PWM TMR                                     */
/*---------------------------------------------------------------------------------*/
/*********************** Bit definition for PWMCON register ***********************/
#define PWMCON_TMR0_EN                      ((uint32_t)0x00000001)
#define PWMCON_TMR1_EN                      ((uint32_t)0x00000002)
#define PWMCON_TMR2_EN                      ((uint32_t)0x00000004)
#define PWMCON_TMR3_EN                      ((uint32_t)0x00000008)
#define PWMCON_TMR_EN_MASK                  1
#define PWMCON_TMR_EN_OFFSET                0
#define PWMCON_TMR_EN_LEN                   1

#define PWMCON_CH0_EN_BIT                   ((uint32_t)0x00000010)
#define PWMCON_CH0_EN_POS                   4
#define PWMCON_CH1_EN_BIT                   ((uint32_t)0x00000020)
#define PWMCON_CH1_EN_POS                   5
#define PWMCON_CH2_EN_BIT                   ((uint32_t)0x00000040)
#define PWMCON_CH2_EN_POS                   6
#define PWMCON_CH3_EN_BIT                   ((uint32_t)0x00000080)
#define PWMCON_CH3_EN_POS                   7
#define PWMCON_CH_EN_MASK                   1
#define PWMCON_CH_EN_OFFSET                 4
#define PWMCON_CH_EN_LEN                    1

#define PWMCON_CH0_INV_BIT                  ((uint32_t)0x00000100)
#define PWMCON_CH0_INV_POS                  8
#define PWMCON_CH1_INV_BIT                  ((uint32_t)0x00000200)
#define PWMCON_CH1_INV_POS                  9
#define PWMCON_CH2_INV_BIT                  ((uint32_t)0x00000400)
#define PWMCON_CH2_INV_POS                  10
#define PWMCON_CH3_INV_BIT                  ((uint32_t)0x00000800)
#define PWMCON_CH3_INV_POS                  11
#define PWMCON_CH_INV_MASK                  1
#define PWMCON_CH_INV_OFFSET                8
#define PWMCON_CH_INV_LEN                   1

#define PWMCON_CH0_TMR_SEL_MASK             ((uint32_t)0x00030000)
#define PWMCON_CH0_TMR_SEL_LSB              16
#define PWMCON_CH1_TMR_SEL_MASK             ((uint32_t)0x000c0000)
#define PWMCON_CH1_TMR_SEL_LSB              18
#define PWMCON_CH2_TMR_SEL_MASK             ((uint32_t)0x00300000)
#define PWMCON_CH2_TMR_SEL_LSB              20
#define PWMCON_CH3_TMR_SEL_MASK             ((uint32_t)0x00c00000)
#define PWMCON_CH3_TMR_SEL_LSB              22
#define PWMCON_CH_TMR_SEL_MASK              3
#define PWMCON_CH_TMR_SEL_OFFSET            16
#define PWMCON_CH_TMR_SEL_LEN               2

/*********************** Bit definition for PTMRxCON register ***********************/
#define PTMRxCON_EN                         ((uint32_t)0x00000001)
#define PTMRxCON_PCLKSEL                    ((uint32_t)0x0000000e)
#define PTMRxCON_PRESEL                     ((uint32_t)0x00000070)
#define PTMRxCON_IE                         ((uint32_t)0x00000080)
#define PTMRxCON_PND                        ((uint32_t)0x00000200)
#define PTMRxCON_MODE                       ((uint32_t)0x0000C000)

/*********************** Bit definition for PTMRxCPND register ***********************/
#define PTMRxCPND_PCLR                      ((uint32_t)0x00000200)
#define PTMRxCPND_PDCNT_CLR                 ((uint32_t)0x00000400)

/*********************** Bit definition for PWMDZCON register *************/
#define PWMDZCONx_EN                        ((uint32_t)0x00000001)
#define PWMDZCONx_MODE                      ((uint32_t)0x00000002)
#define PWMDZCONx_LEN_MASK                  ((uint32_t)0x0003fffc)
#define PWMDZCONx_LEN_LSB                   2


/**
 * @brief PWM TIMER Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 cpnd;
    volatile u32 cnt;
    volatile u32 period;
    volatile u32 duty;
} pwm_tmr_typedef;

/**
 * @brief PWM Register Structure
 */
typedef struct {
    volatile u32 con;
} pwm_typedef;

/**
 * @brief PWM DZ Register Structure
 */
typedef struct {
    volatile u32 con;
} pwm_dz_typedef;

#endif

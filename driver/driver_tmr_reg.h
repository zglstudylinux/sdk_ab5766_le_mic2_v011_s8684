/*
 * @File name    : driver_tmr_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-20
 * @Description  : This file contains all the functions prototypes for the TMR library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_TMR_REG_H
#define _DRIVER_TMR_REG_H

#define TMR0_BASE                           (&TMR0CON)
#define TMR1_BASE                           (&TMR1CON)
#define TMR2_BASE                           (&TMR2CON)
#define TMR3_BASE                           (&TMR3CON)

#define TMR0_REG                            ((tmr_typedef *) TMR0_BASE)
#define TMR1_REG                            ((tmr_typedef *) TMR1_BASE)
#define TMR2_REG                            ((tmr_typedef *) TMR2_BASE)
#define TMR3_REG                            ((tmr_typedef *) TMR3_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     TMR                                         */
/*---------------------------------------------------------------------------------*/
/******************** Bit definition for TMRxCON register ********************/
#define TMRxCON_TMREN                       ((uint32_t)0x00000001)
#define TMRxCON_PCLKSEL                     ((uint32_t)0x0000000e)
#define TMRxCON_PDIVSEL                     ((uint32_t)0x00000070)
#define TMRxCON_TIE                         ((uint32_t)0x00000080)
#define TMRxCON_CPT_IE                      ((uint32_t)0x00000100)
#define TMRxCON_TPND                        ((uint32_t)0x00000200)
#define TMRxCON_CPTPND                      ((uint32_t)0x00000400)
#define TMRxCON_HOLDEN                      ((uint32_t)0x00001000)
#define TMRxCON_TICKEN                      ((uint32_t)0x00002000)
#define TMRxCON_MODE_SEL                    ((uint32_t)0x0000c000)

/******************** Bit definition for TMRxCPND register *******************/
#define TMRxCPND_TPCLR                      ((uint32_t)0x00000200)
#define TMRxCPND_CPTPCLR                    ((uint32_t)0x00000400)

/**
 * @brief Timer Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 cpnd;
    volatile u32 cnt;
    volatile u32 period;
    volatile u32 capture_val;
} tmr_typedef;

#endif

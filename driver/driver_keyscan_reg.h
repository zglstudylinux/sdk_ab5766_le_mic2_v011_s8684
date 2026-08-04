/*
 * @File name    : driver_keyscan_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-21
 * @Description  : This file contains all the register definition for the KEYSCAN.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_KEYSCAN_REG_H
#define _DRIVER_KEYSCAN_REG_H

#define KEYSCAN_BASE                        (&KEYSCAN_CON0)

#define KEYSCAN_REG                         ((keyscan_typedef *) KEYSCAN_BASE)

/*---------------------------------------------------------------------------------*/
/*                                   KEYSCAN                                       */
/*---------------------------------------------------------------------------------*/
/********************* Bit definition for KEYSCAN_CON0 register ********************/
#define KEYSCAN_CON0_KEYVALID_PND           ((uint32_t)0x80000000)
#define KEYSCAN_CON0_KEYINVALID_PND         ((uint32_t)0x40000000)

#define KEYSCAN_CON0_KEYPORT_EN             ((uint32_t)0x0000ff00)
#define KEYSCAN_CON0_KEYPORT_EN_0           ((uint32_t)0x00000100)
#define KEYSCAN_CON0_PD_SEL                 ((uint32_t)0x00040000)
#define KEYSCAN_CON0_PU_SEL                 ((uint32_t)0x00020000)
#define KEYSCAN_CON0_IOGROUP_SEL            ((uint32_t)0x00010000)

#define KEYSCAN_CON0_KEYSCAN_SWRSTN         ((uint32_t)0x00000080)
#define KEYSCAN_CON0_KEYINVALID_INT_EN      ((uint32_t)0x00000020)
#define KEYSCAN_CON0_KEYVALID_INT_EN        ((uint32_t)0x00000010)
#define KEYSCAN_CON0_KEYVALID_WKUP_EN       ((uint32_t)0x00000008)
#define KEYSCAN_CON0_KEYSCAN_LP             ((uint32_t)0x00000004)
#define KEYSCAN_CON0_KEYSCAN_MODE           ((uint32_t)0x00000002)
#define KEYSCAN_CON0_KEYSCAN_EN             ((uint32_t)0x00000001)

/********************* Bit definition for KEYSCAN_CON1 register ********************/
#define KEYSCAN_CON1_KEYPORT_STAB_TIME      ((uint32_t)0x00ff0000)
#define KEYSCAN_CON1_KEYPORT_STAB_TIME_0    ((uint32_t)0x00010000)

#define KEYSCAN_CON1_KEY_DEBO_TIMES         ((uint32_t)0x0000ff00)
#define KEYSCAN_CON1_KEY_DEBO_TIMES_0       ((uint32_t)0x00000100)

#define KEYSCAN_CON1_KEY_WAIT_TIMES         ((uint32_t)0x000000ff)
#define KEYSCAN_CON1_KEY_WAIT_TIMES_0       ((uint32_t)0x00000001)

/********************* Bit definition for KEYSCAN_CLR register *********************/
#define KEYSCAN_CLR_KEYVALID_PND_CLR        ((uint32_t)0x80000000)
#define KEYSCAN_CLR_KEYINVALID_PND_CLR      ((uint32_t)0x40000000)

/********************* Bit definition for KEYSCAN_INFO0 register *******************/
#define KEYSCAN_INFO0_KD7_KD0               ((uint32_t)0xff000000)
#define KEYSCAN_INFO0_KD7_KD0_0             ((uint32_t)0x01000000)

#define KEYSCAN_INFO0_KC7_KC0               ((uint32_t)0x00ff0000)
#define KEYSCAN_INFO0_KC7_KC0_0             ((uint32_t)0x00010000)

#define KEYSCAN_INFO0_KB7_KB0               ((uint32_t)0x0000ff00)
#define KEYSCAN_INFO0_KB7_KB0_0             ((uint32_t)0x00000100)

#define KEYSCAN_INFO0_KA7_KA0               ((uint32_t)0x000000ff)
#define KEYSCAN_INFO0_KA7_KA0_0             ((uint32_t)0x00000001)

/********************* Bit definition for KEYSCAN_INFO1 register *******************/
#define KEYSCAN_INFO1_KH7_KH0               ((uint32_t)0xff000000)
#define KEYSCAN_INFO1_KH7_KH0_0             ((uint32_t)0x01000000)

#define KEYSCAN_INFO1_KG7_KG0               ((uint32_t)0x00ff0000)
#define KEYSCAN_INFO1_KG7_KG0_0             ((uint32_t)0x00010000)

#define KEYSCAN_INFO1_KF7_KF0               ((uint32_t)0x0000ff00)
#define KEYSCAN_INFO1_KF7_KF0_0             ((uint32_t)0x00000100)

#define KEYSCAN_INFO1_KE7_KE0               ((uint32_t)0x000000ff)
#define KEYSCAN_INFO1_KE7_KE0_0             ((uint32_t)0x00000001)

/********************* Bit definition for KEYSCAN_INFO2 register *******************/
#define KEYSCAN_INFO2_KI7_KI0               ((uint32_t)0x0000ff00)
#define KEYSCAN_INFO2_KI7_KI0_0             ((uint32_t)0x00000100)

#define KEYSCAN_INFO2_KJ7_KJ0               ((uint32_t)0x000000ff)
#define KEYSCAN_INFO2_KJ7_KJ0_0             ((uint32_t)0x00000001)

/********************* Bit definition for KEYSCAN_CON1 register ********************/
#define KEYSCAN_CON1_KEYPORT_STAB_TIME      ((uint32_t)0x00ff0000)
#define KEYSCAN_CON1_KEYPORT_STAB_TIME_0    ((uint32_t)0x00010000)

#define KEYSCAN_CON1_KEY_DEBO_TIMES         ((uint32_t)0x0000ff00)
#define KEYSCAN_CON1_KEY_DEBO_TIMES_0       ((uint32_t)0x00000100)

#define KEYSCAN_CON1_KEY_WAIT_TIMES         ((uint32_t)0x000000ff)
#define KEYSCAN_CON1_KEY_WAIT_TIMES_0       ((uint32_t)0x00000001)

/**
 * @brief KEYSCAN Register Structure
 */
typedef struct {
    volatile u32 con0;
    volatile u32 con1;
    volatile u32 clr;
    volatile u32 info0;
    volatile u32 info1;
    volatile u32 info2;
} keyscan_typedef;

#endif

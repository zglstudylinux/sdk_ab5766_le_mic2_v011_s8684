/*
 * @File name    : driver_wdt_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the functions prototypes for the WDT library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_WDT_REG_H
#define _DRIVER_WDT_REG_H

#define WDT_BASE                            (&WDTCON)

#define WDT_REG                             ((wdt_typedef *) WDT_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     WDT                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for WDTCON register ***********************/
#define WDTCON_WDTCLR_MASK                  ((uint32_t)0x0000000f)
#define WDTCON_WDTCLR_LSB                   0

#define WDTCON_WDTEN_MASK                   ((uint32_t)0x000000f0)
#define WDTCON_WDTEN_LSB                    4

#define WDTCON_RSTEN_MASK                   ((uint32_t)0x00000f00)
#define WDTCON_RSTEN_LSB                    8

#define WDTCON_WDTIE_MASK                   ((uint32_t)0x0000f000)
#define WDTCON_WDTIE_LSB                    12

#define WDTCON_WDTCSEL_MASK                 ((uint32_t)0x000f0000)
#define WDTCON_WDTCSEL_LSB                  16

#define WDTCON_TMRSEL_MASK                  ((uint32_t)0x00700000)
#define WDTCON_TMRSEL_LSB                   20

#define WDTCON_TMRSEL_WR_MASK               ((uint32_t)0x0f000000)
#define WDTCON_TMRSEL_WR_LSB                24

#define WDTCON_WDTPND                       ((uint32_t)0x80000000)


/**
 * @brief WDT Register Structure
 */
typedef struct {
    volatile u32 con;
} wdt_typedef;

#endif

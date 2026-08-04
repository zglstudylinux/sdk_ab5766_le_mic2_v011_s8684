/*
 * @File name    : driver_sddac_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the register definition for the SDDAC.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */


#ifndef _DRIVER_SDDAC_REG_H
#define _DRIVER_SDDAC_REG_H


#define SDDAC_BASE                          (&DACDIGCON0)

#define SDDAC_REG                           ((sddac_typedef *) SDDAC_BASE)

/**
 * @brief SDDAC Register Structure
 */
typedef struct {
    volatile u32 dma_con;
    volatile u32 dma_addr;
    volatile u32 dma_size;
    volatile u32 dma_pend;
    volatile u32 dma_cpnd;
    volatile u32 con;
} sddac_typedef;


#endif

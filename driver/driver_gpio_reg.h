/*
 * @File name    : driver_gpio_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-08
 * @Description  : This file contains all the functions prototypes for the GPIO library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_GPIO_REG_H
#define _DRIVER_GPIO_REG_H


#define GPIOA_BASE                          (&GPIOASET)
#define GPIOB_BASE                          (&GPIOBSET)
#define FUNCMCON_BASE                       (&FUNCOMCON0)


#define GPIOA_REG                           ((gpio_typedef *) GPIOA_BASE)
#define GPIOB_REG                           ((gpio_typedef *) GPIOB_BASE)
#define FUNCMAP                             ((func_map_typedef *) FUNCMCON_BASE)

/*********************** Bit definition for WKPINMAP register **********************/
#define WKPINMAP_WK0MAP_MASK                ((uint32_t)0x00000003)
#define WKPINMAP_WK0MAP_LSB                 0
#define WKPINMAP_WK1MAP_MASK                ((uint32_t)0x0000000c)
#define WKPINMAP_WK1MAP_LSB                 2
#define WKPINMAP_WK2MAP_MASK                ((uint32_t)0x00000030)
#define WKPINMAP_WK2MAP_LSB                 4
#define WKPINMAP_WK3MAP_MASK                ((uint32_t)0x000000c0)
#define WKPINMAP_WK3MAP_LSB                 6
#define WKPINMAP_WK4MAP_MASK                ((uint32_t)0x00000300)
#define WKPINMAP_WK4MAP_LSB                 8
#define WKPINMAP_WK5MAP_MASK                ((uint32_t)0x00000c00)
#define WKPINMAP_WK5MAP_LSB                 10
#define WKPINMAP_WK6MAP_MASK                ((uint32_t)0x00003000)
#define WKPINMAP_WK6MAP_LSB                 12
#define WKPINMAP_WK7MAP_MASK                ((uint32_t)0x0000c000)
#define WKPINMAP_WK7MAP_LSB                 14
#define WKPINMAP_WKMAP_MASK                 3
#define WKPINMAP_WKMAP_LEN                  2
#define WKPINMAP_WKMAP_OFFSET               0

/*********************** Bit definition for WKUPCON register **********************/
#define WKUPCON_WKEN_MASK                   ((uint32_t)0x000000ff)
#define WKUPCON_WKIE_BIT                    ((uint32_t)0x00010000)
#define WKUPCON_WKSLPEN_BIT                 ((uint32_t)0x00020000)
#define WKUPCON_PFALL_FIL_MASK              ((uint32_t)0x00300000)
#define WKUPCON_PFALL_FIL_LSB               20
#define WKUPCON_PRISE_FIL_MASK              ((uint32_t)0x00c00000)
#define WKUPCON_PRISE_FIL_LSB               22
#define WKUPCON_RSTPINSEL_MASK              ((uint32_t)0x1F000000)
#define WKUPCON_RSTPINSEL_LSB               24
#define WKUPCON_RSTPINSELWV_BIT             ((uint32_t)0x20000000)

/*********************** Bit definition for WKUPEDG register **********************/
#define WKUPEDG_WKEDG_MASK                  ((uint32_t)0x000000ff)
#define WKUPEDG_WKPND_MASK                  ((uint32_t)0x00ff0000)
#define WKUPEDG_WKPND_LSB                   16

/*********************** Bit definition for WKUPCPND register **********************/
#define WKUPCPND_WKCPND_MASK                    ((uint32_t)0x00ff0000)

/**
 * @brief GPIO Register Structure
 */
typedef struct {
    volatile u32 io_set;
    volatile u32 io_clr;
    volatile u32 data;
    volatile u32 dir;
    volatile u32 de;
    volatile u32 fen;
    volatile u32 fdir;
    volatile u32 drv;
    volatile u32 pupd[6];
} gpio_typedef;

/**
 * @brief Function and IO Mapping Register Structure
 */
typedef struct {
    volatile u32 func_output_map[8];
    volatile u32 func_input_map[7];
} func_map_typedef;

#endif

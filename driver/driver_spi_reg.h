/*
 * @File name    : driver_spi_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the functions prototypes for the SPI library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SPI_REG_H
#define _DRIVER_SPI_REG_H

#define SPI1_BASE                           (&SPI1CON)

#define SPI1_REG                            ((spi_typedef *) SPI1_BASE)

/*---------------------------------------------------------------------------------*/
/*                                     SPI                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for SPIxCON register **********************/
#define SPIxCON_SPIEN                       ((uint32_t)0x00000001)
#define SPIxCON_SPISM                       ((uint32_t)0x00000002)

#define SPIxCON_BUSMODE                     ((uint32_t)0x0000000c)

#define SPIxCON_RXSEL                       ((uint32_t)0x00000010)
#define SPIxCON_CLKIDS                      ((uint32_t)0x00000020)
#define SPIxCON_SMPS                        ((uint32_t)0x00000040)
#define SPIxCON_SPIIE                       ((uint32_t)0x00000080)
#define SPIxCON_SPILF_EN                    ((uint32_t)0x00000100)
#define SPIxCON_SPIMBEN                     ((uint32_t)0x00000200)
#define SPIxCON_SPIOSS                      ((uint32_t)0x00000400)
#define SPIxCON_HOLDENRX                    ((uint32_t)0x00000800)
#define SPIxCON_HOLDENTX                    ((uint32_t)0x00001000)
#define SPIxCON_HOLDENSW                    ((uint32_t)0x00002000)
#define SPIxCON_SPIPND                      ((uint32_t)0x00010000)

/**
 * @brief SPI Register Structure
 */
typedef struct {
    volatile u32 con;
    volatile u32 buf;
    volatile u32 baud;
    volatile u32 cpnd;
    volatile u32 dma_cnt;
    volatile u32 dma_adr;
} spi_typedef;

#endif

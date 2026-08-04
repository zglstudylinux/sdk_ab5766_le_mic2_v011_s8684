/*
 * @File name    : driver_spi.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-08
 * @Description  : This file contains all the functions prototypes for the SPI library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SPI_H
#define _DRIVER_SPI_H

#include "driver_com.h"


/**
 * @brief SPI IT Flag Definition
 */
typedef enum {
    SPI_FLAG_PENDING          = 0x00010000,
} SPI_FLAG_TYPEDEF;

/**
 * @brief SPI Role Mode Enumeration
 */
typedef enum {
    SPI_ROLE_MODE_MASTER                    = 0x00000000,
    SPI_ROLE_MODE_SLAVE                     = 0x00000002,
} SPI_ROLE_MODE_TYPEDEF;

/**
 * @brief SPI Bus Mode Enumeration
 */
typedef enum {
    SPI_3WIRE_MODE                          = 0x00000000,
    SPI_2WIRE_MODE                          = 0x00000004,
    SPI_2BIT_BID_MODE                       = 0x00000008,
    SPI_4BIT_BID_MODE                       = 0x0000000c,
} SPI_BUS_MODE_TYPEDEF;

/**
 * @brief SPI Output Data Edge Enumeration
 */
typedef enum {
    SPI_OUTPUT_DATA_EDGE_FALLING            = 0x00000000,
    SPI_OUTPUT_DATA_EDGE_RISING             = 0x00000040,
} SPI_OUTPUT_DATA_EDGE_TYPEDEF;

/**
 * @brief SPI Sample Clock Enumeration
 */
typedef enum {
    SPI_SAMPLE_AND_OUTPUT_CLOCK_DIFF        = 0x00000000,
    SPI_SAMPLE_AND_OUTPUT_CLOCK_SAME        = 0x00000400,
} SPI_SAMPLE_AND_OUTPUT_CLOCK_TYPEDEF;

/**
 * @brief SPI Direction
 */
typedef enum {
    SPI_DIR_TX                              = 0x00,
    SPI_DIR_RX                              = 0x01,
} SPI_DIR_TYPEDEF;

/**
 * @brief SPI Clock State When Idle
 */
typedef enum {
    SPI_CLOCK_IDLE_LOW                      = 0x00000000,
    SPI_CLOCK_IDLE_HIGH                     = 0x00000020,
} SPI_CLOCK_IDLE_STATE_TYPEDEF;

/* SPI Init Structure */
typedef struct{
    uint32_t baud;
    SPI_ROLE_MODE_TYPEDEF role_mode;
    SPI_BUS_MODE_TYPEDEF bus_mode;
    SPI_OUTPUT_DATA_EDGE_TYPEDEF output_data_edge;
    SPI_SAMPLE_AND_OUTPUT_CLOCK_TYPEDEF sample_clk_edge;
    SPI_CLOCK_IDLE_STATE_TYPEDEF clkids;    //if SPI1BAUD is equal to 0, then CLKIDS forces 0 by designed
} spi_init_typedef;

/**
  * @brief  Initializes the spix peripheral according to the specified
  *         parameters in the spi_init_struct.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  spi_init_struct: pointer to a spi_init_typedef structure that
  *         contains the configuration information for the specified SPI peripheral.
  * @retval None
  */
void spi_init(spi_typedef* spix, spi_init_typedef* spi_init_struct);

/**
 * @brief  De-initialize the specified SPI peripheral.
 * @param  spix: where x can be (0..1) to select the SPI peripheral.
 * @retval None
 */
void spi_deinit(spi_typedef* spix);

/**
  * @brief  Enable or disable the specified SPI peripheral.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_cmd(spi_typedef* spix, FUNCTIONAL_STATE state);

/**
  * @brief  Select the specified SPI direction when in DMA mode or 2-wire mode.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  direction: transmit or receive.
            This parameter can be: SPI_DIR_TX or SPI_DIR_RX.
  * @retval None
  */
void spi_dir_sel(spi_typedef* spix, SPI_DIR_TYPEDEF direction);

/**
  * @brief  Enable or disable the specified SPI interrupts.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_it_cmd(spi_typedef* spix, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified SPI multiple bit bus.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  state: state of the SPIx interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_multi_bit_bus_cmd(spi_typedef* spix, FUNCTIONAL_STATE state);

/**
  * @brief  Get the SPI pending.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  spi_flag: specifies the flag to set.
  *         this parameter can be one of the following values:
  *             @arg SPI_FLAG_PENDING: SPI(0..1) SPI pending.
  * @retval The state of spi_flag (SET or RESET).
  */
FLAG_STATE spi_get_flag(spi_typedef* spix, SPI_FLAG_TYPEDEF spi_flag);

/**
  * @brief  Clear the SPI pending.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  spi_flag: specifies the flag to clear.
  *         this parameter can be one of the following values:
  *             @arg SPI_FLAG_PENDING: SPI(0..1) SPI pending.
  * @retval None
  */
void spi_clear_flag(spi_typedef* spix, SPI_FLAG_TYPEDEF spi_flag);

/**
  * @brief  Set the SPI counter register value.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  cnt: specifies the counter register new value.
  * @retval None
  */
void spi_set_dma_cnt(spi_typedef* spix, uint32_t cnt);

/**
  * @brief  Set the SPI DMA address register value.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  addr: specifies the SPI DMA address register new value.
  * @retval None
  */
void spi_set_dma_addr(spi_typedef* spix, uint32_t addr);

/**
  * @brief  Get the spi receive data.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @retval spi receive data.
  */
uint8_t spi_receive_data(spi_typedef* spix);

/**
  * @brief  Send the spi receive data.
  * @param  spix: where x can be (0..1) to select the SPI peripheral.
  * @param  data: specifies the SPI data to be sent.
  * @retval None.
  */
void spi_send_data(spi_typedef* spix, uint8_t data);

/**
  * @brief  Enable or disable the specified SPIx interrupt.
  * @param  uartx: where x can be (0,1) to select the SPI peripheral.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  state: the state of the SPIx peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void spi_pic_config(spi_typedef* spix, isr_t isr, int pr, FUNCTIONAL_STATE state);

#endif // _DRIVER_SPI_H

/*
 * @File name    : driver_despi.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-10
 * @Description  : This file contains all the functions prototypes for the DESPI library.
 * 
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_DESPI_H
#define _DRIVER_DESPI_H

#include "driver_com.h"

//--------------- DESPI[0..1] Flag Definition ---------------//
#define DESPI_FLAG_PENDING                    ((uint32_t)0x00010000)

/**
 * @brief DESPI Color order Enumeration
 */
typedef enum {
    DESPI_COLOR_ORDER_RGB                     = 0 << 31,
    DESPI_COLOR_ORDER_BGR                     = 1 << 31,
} DESPI_CL_ORD_TYPEDEF;

/**
 * @brief DESPI Role Mode Enumeration
 */
typedef enum {
    DESPI_ROLE_MODE_MASTER                    = 0 << 1,
    DESPI_ROLE_MODE_SLAVE                     = 1 << 1,
} DESPI_ROLE_MODE_TYPEDEF;

/**
 * @brief DESPI Bus Mode Enumeration
 */
typedef enum {
    DESPI_3WIRE_MODE                          = 0 << 2,  //CLK--DI--DO
    DESPI_2WIRE_MODE                          = 1 << 2,  //CLK--DI/O
    DESPI_2BIT_BID_MODE                       = 2 << 2,  //CLK--DI/O--DI/O
    DESPI_4BIT_BID_MODE                       = 3 << 2,  //CLK--DI/O--DI/O--DI/O--DI/O
} DESPI_BUS_MODE_TYPEDEF;

/**
 * @brief DESPI Output Data Edge Enumeration
 */
typedef enum {
    DESPI_OUTPUT_DATA_EDGE_FALLING            = 0 << 6,
    DESPI_OUTPUT_DATA_EDGE_RISING             = 1 << 6,
} DESPI_OUTPUT_DATA_EDGE_TYPEDEF;

/**
 * @brief DESPI Sample Clock Enumeration
 */
typedef enum {
    DESPI_SAMPLE_AND_OUTPUT_CLOCK_DIFF        = 0 << 10,
    DESPI_SAMPLE_AND_OUTPUT_CLOCK_SAME        = 1 << 10,
} DESPI_SAMPLE_AND_OUTPUT_CLOCK_TYPEDEF;

/**
 * @brief DESPI Direction
 */
typedef enum {
    DESPI_DIR_TX                              = 0x00,
    DESPI_DIR_RX                              = 0x01,
} DESPI_DIR_TYPEDEF;

/**
 * @brief DESPI Clock State When Idle
 */
typedef enum {
    DESPI_CLOCK_IDLE_LOW                      = 0 << 5,
    DESPI_CLOCK_IDLE_HIGH                     = 1 << 5,
} DESPI_CLOCK_IDLE_STATE_TYPEDEF;

/* DESPI Init Structure */
typedef struct{
    uint32_t baud;
    DESPI_ROLE_MODE_TYPEDEF role_mode;
    DESPI_CL_ORD_TYPEDEF color_order;
    DESPI_BUS_MODE_TYPEDEF bus_mode;
    DESPI_OUTPUT_DATA_EDGE_TYPEDEF output_data_edge;
    DESPI_SAMPLE_AND_OUTPUT_CLOCK_TYPEDEF sample_clk_edge;
    DESPI_CLOCK_IDLE_STATE_TYPEDEF clkids;    //if DESPI1BAUD is equal to 0, then CLKIDS forces 0 by designed
} despi_init_typedef;

/**
  * @brief  Initializes the despi peripheral according to the specified
  *         parameters in the despi_init_struct.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  despi_init_struct: pointer to a despi_init_typedef structure that
  *         contains the configuration information for the specified DESPI peripheral.
  * @retval None
  */
void despi_init(despi_typedef* despix, despi_init_typedef* despi_init_struct);

/**
 * @brief  De-initialize the specified DESPI peripheral.
 * @param  despi: where x can be (0..1) to select the DESPI peripheral.
 * @retval None
 */
void despi_deinit(despi_typedef* despix);

/**
  * @brief  Enable or disable the specified DESPI peripheral.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Select the specified DESPI direction when in DMA mode or 2-wire mode.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  direction: transmit or receive.
            This parameter can be: DESPI_DIR_TX or DESPI_DIR_RX.
  * @retval None
  */
void despi_dir_sel(despi_typedef* despix, DESPI_DIR_TYPEDEF direction);

/**
  * @brief  Enable or disable the specified DESPI interrupts.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_it_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI LFSR.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_lfsr_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI multiple bit bus.
  * @brief  DESPI_2BIT_BID_MODE or DESPI_2BIT_BID_MODE require enable.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_multi_bit_bus_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI hold when bt rx.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_hold_rx_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI hold when bt tx.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: state of the DESPI interrupts.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_hold_tx_cmd(despi_typedef* despix, FUNCTIONAL_STATE state);

/**
  * @brief  Select the specified DESPI bus mode.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  mode: This parameter can be: DESPI_BUS_MODE_TYPEDEF.
  * @retval None
  */
void despi_bus_mode_sel(despi_typedef* despi, DESPI_BUS_MODE_TYPEDEF mode);

/**
  * @brief  Enable or disable the specified DESPI dual.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: 0:Output DCX once for each byte. 1:Output DCX once for each pixel.
  * @retval None
  */
void despi_dual_cmd(despi_typedef* despi, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI dcx reuse.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: 0:The DCS signal is inserted into the data. 1:Use the D1 pin alone as the DCX signal pin.
  * @retval None
  */
void despi_dcx_reuse_cmd(despi_typedef* despi, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI 9bit mode.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: 0:disable 9bit mode. 1: enable 9bit mode (bit(0) is DCX).
  * @retval None
  */
void despi_dcx_9bit_cmd(despi_typedef* despi, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the specified DESPI dcx value.
  * @brief  When dcx_reuse is enabled and dcx_9bit is disabled, the dcx_value controls the high and low levels of pin D1.
  * @brief  When dcx_reuse is disabled and dcx_9bit is enabled, the dcx_value controls the dcx signal within the 9-bit data.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  state: 0:DCX is low. 1: DCX is high.
  * @retval None
  */
void despi_dcx_value_cmd(despi_typedef* despi, FUNCTIONAL_STATE state);

/**
  * @brief  Get the DESPI pending.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  despi_flag: specifies the flag to set.
  *         this parameter can be one of the following values:
  *             @arg DESPI_FLAG_PENDING: DESPI(0..1) DESPI pending.
  * @retval The state of despi_flag (SET or RESET).
  */
FLAG_STATE despi_get_flag(despi_typedef* despix, uint32_t despi_flag);

/**
  * @brief  Clear the DESPI pending.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  despi_flag: specifies the flag to clear.
  *         this parameter can be one of the following values:
  *             @arg DESPI_FLAG_PENDING: DESPI(0..1) DESPI pending.
  * @retval None
  */
void despi_clear_flag(despi_typedef* despix, uint32_t despi_flag);

/**
  * @brief  Set the DESPI counter register value.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  cnt: specifies the counter register new value.
  * @retval None
  */
void despi_set_dma_cnt(despi_typedef* despix, uint32_t cnt);

/**
  * @brief  Set the DESPI DMA address register value.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  addr: specifies the DESPI DMA address register new value.
  * @retval None
  */
void despi_set_dma_addr(despi_typedef* despix, uint32_t addr);

/**
  * @brief  Get the despi receive data.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @retval despi receive data.
  */
uint8_t despi_receive_data(despi_typedef* despix);

/**
  * @brief  Send the despi receive data.
  * @param  despi: where x can be (0..1) to select the DESPI peripheral.
  * @param  data: specifies the DESPI data to be sent.
  * @retval None.
  */
void despi_send_data(despi_typedef* despix, uint8_t data);

/**
  * @brief  Enable or disable the specified DESPI interrupt.
  * @param  uartx: where x can be (0,1) to select the DESPI peripheral.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  state: the state of the DESPI peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void despi_pic_config(despi_typedef* despix, isr_t isr, int pr, FUNCTIONAL_STATE state);

#endif // _DRIVER_DESPI_H

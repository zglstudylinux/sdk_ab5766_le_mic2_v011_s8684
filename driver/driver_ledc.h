/*
 * @File name    : driver_ledc.h
 * @Author       : Bluetrum IOT Team.
 * @Date         : 2025-09-10
 * @Description  : This file contains all the functions protorypes for the LEDC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_LEDC_H
#define _DRIVER_LEDC_H

#include "driver_com.h"

/**
 * @brief Flag Enumeration.
 */
typedef enum {
    LEDC_FLAG_DONE          = 0x01,
    LEDC_FLAG_RST           = 0x02,
    LEDC_FLAG_DMA           = 0x04,
} LEDC_FLAG_TYPEDEF;

/**
 * @brief Output Invert Enumeration.
 */
typedef enum {
    LEDC_OUT_INV_DIS        = 0x00000000,
    LEDC_OUT_INV_EN         = 0x02000000,
} LEDC_OUT_INV_TYPEDEF;

/**
 * @brief DMA Buffer Data Format Enumeration.
 */
typedef enum {
    LEDC_IN_FORMAT_RGB      = 0x00000000,
    LEDC_IN_FORMAT_WRGB     = 0x00000080,
} LEDC_IN_FORMAT_TYPEDEF;

/**
 * @brief Idle Level Enumeration.
 */
typedef enum {
    LEDC_IDLE_LEVEL_LOW     = 0x00000000,
    LEDC_IDLE_LEVEL_HIGH    = 0x00000020,
} LEDC_IDLE_LEVEL_TYPEDEF;

/**
 * @brief Valid Data Direct or Invert Enumeration.
 */
typedef enum {
    LEDC_VALID_DATA_DIR     = 0x00000000,
    LEDC_VALID_DATA_INV     = 0x00000010,
} LEDC_VALID_DATA_TYPEDEF;

/**
 * @brief Valid Data Invert Byte Number Enumeration.
 *LEDC_BYTE_INV_1B:  0x44332211 ->  0x22cc4488. 每1个字节的位序反转：0000 0001 --> 1000 0000
 *LEDC_BYTE_INV_2B:  0x44332211 ->  0xcc228844. 每2个字节的位序反转：0000 0001 0000 0001 --> 1000 0000 1000 0000
 *LEDC_BYTE_INV_4B:  0x44332211 ->  0x8844cc22. 每4个字节的位序反转：0000 0001 0000 0001 0000 0001 0000 0001  -->  1000 0000 1000 0000 1000 0000 1000 0000
 */
typedef enum {
    LEDC_BYTE_INV_NO        = 0x00000000,
    LEDC_BYTE_INV_1B        = 0x00000004,
    LEDC_BYTE_INV_2B        = 0x00000008,
    LEDC_BYTE_INV_4B        = 0x0000000c,
} LEDC_BYTE_INV_TYPEDEF;

/**
 * @brief LEDC Ctrl Initialization Struct.
 */
typedef struct {
    LEDC_OUT_INV_TYPEDEF    output_invert;
    LEDC_IN_FORMAT_TYPEDEF  format;
    LEDC_IDLE_LEVEL_TYPEDEF idle_level;
    LEDC_VALID_DATA_TYPEDEF valid_data_mode;
    LEDC_BYTE_INV_TYPEDEF   byte_inv_number;
} ledc_init_typedef;

/**
 * @brief LEDC Code Element Timing Initialization Struct.
 */
typedef struct {
    ///baud clk = ledc clk / (ledc baud + 1), 1/baud clk = the total time for transmitting one code element
    uint8_t                 baud;
    ///ledc delay between RGB(or WRGB) and RGB(or WRGB), using the ledc clk for counting
    uint8_t                 delay;
    ///ledc code 0 high level time, using the ledc clk for counting, low level time = 1/baud clk - high level time
    uint8_t                 code_0_high;
    ///ledc code 1 high level time, using the ledc clk for counting, low level time = 1/baud clk - high level time
    uint8_t                 code_1_high;
    ///interval between (LEDC last byte end) and (done pending), using the baud clk for counting, must be less than reset_low
    uint16_t                reset_high;
    ///interval between (LEDC last byte end) and (rst pending), using the baud clk for counting
    uint16_t                reset_low;
} ledc_timing_init_typedef;


/**
 * @brief  Initializes the LEDC peripheral according to the specified
 *         parameters in the ledc_timing_init_struct.
 * @param  ledc_timing_init_struct: pointer to a ledc_init_typedef structure that
 *         contains the configuration information for the specified LEDC peripheral.
 * @retval None
 */
void ledc_init(ledc_typedef *ledc, ledc_init_typedef *ledc_init_struct);

/**
 * @brief  Initialize the code element timing accordingto the specified
 *         parameters in the ledc_timing_init_struct.
 * @param  ledc_timing_init_struct: pointer to a ledc_timing_init_typedef structure that
 *         contains the configuration information for the specified LEDC peripheral.
 * @retval None
 */
void ledc_timing_init(ledc_typedef *ledc, ledc_timing_init_typedef *ledc_timing_init_struct);

/**
 * @brief  De-initialize the specified LEDC peripheral.
 * @retval None
 */
void ledc_deinit(ledc_typedef *ledc);

/**
 * @brief  Enable or disable the specified LEDC peripheral.
 * @param  state: the state of the LEDC peripheral.
 *         This parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void ledc_cmd(ledc_typedef *ledc, FUNCTIONAL_STATE state);

/**
 * @brief  Enable the LEDC DMA function.
 * @param  addr: DMA start address.
 * @param  len: DMA transmit len.
 * @retval None
 */
void ledc_dma_kick(ledc_typedef *ledc, uint32_t addr, uint16_t len);

/**
 * @brief  Start ledc transmit.
 * @retval None
 */
void ledc_kick(ledc_typedef *ledc);

/**
 * @brief  Enable or disable the specified LEDC interrupt.
 * @param  isr: Function to be executed for service interruption.
 * @param  pr: Priority of service interruption.
 * @param  flag_type: specifies the LEDC interrupt sources to be enable or disable.
 *         This parameter can be one of the following values:
 *             @arg LEDC_FLAG_DONE: One frame has tx done.
 *             @arg LEDC_FLAG_RST: One frame has tx done and reset time has finished.
 *             @arg LEDC_FLAG_DMA: One DMA transmit finished.
 * @param  state: the new state need to config.
 * @retval None
 */
void ledc_pic_config(ledc_typedef *ledc, isr_t isr, int pr, LEDC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
 * @brief  Check the specified LEDC flag is set or not.
 * @param  flag_type: specifies the flag to check.
 *         the parameter can be one of the following values:
 *             @arg LEDC_FLAG_DONE: One frame has tx done.
 *             @arg LEDC_FLAG_RST: One frame has tx done and reset time has finished.
 *             @arg LEDC_FLAG_DMA: One DMA transmit finished.
 * @retval The state of flag_type (SET or RESET).
 */
FLAG_STATE ledc_get_flag(ledc_typedef *ledc, LEDC_FLAG_TYPEDEF flag_type);

/**
 * @brief  Clear the LEDC pending.
 * @param  flag_type: specified flag to clear.
 *             @arg LEDC_FLAG_DONE: One frame has tx done.
 *             @arg LEDC_FLAG_RST: One frame has tx done and reset time has finished.
 *             @arg LEDC_FLAG_DMA: One DMA transmit finished.
 * @retval None
 */
void ledc_clear_flag(ledc_typedef *ledc, LEDC_FLAG_TYPEDEF flag_type);

/**
 * @brief  Set the length of frame length.
 * @param  frame_length: the length param of frame that need to config.
 * @retval None
 */
void ledc_set_frame_length(ledc_typedef *ledc, uint16_t frame_length);

/**
 * @brief  Set the number of loop.
 * @param  loop_n: the number param of loop that need to config.
 * @retval None
 */
void ledc_set_loop_number(ledc_typedef *ledc, uint16_t loop_n);

/**
 * @brief  Get the length of frame length.
 * @retval None
 */
uint16_t ledc_get_frame_length(ledc_typedef *ledc);

/**
 * @brief  Get the number of loop.
 * @retval None
 */
uint16_t ledc_get_loop_number(ledc_typedef *ledc);

#endif // _DRIVER_LEDC_H

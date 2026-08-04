/*
 * @File name    : driver_saradc.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-02
 * @Description  : This file contains all the functions prototypes for the SARADC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SARADC_H
#define _DRIVER_SARADC_H

#include "driver_com.h"

#define SARADC_MAX_CH                   16

/**
 * @brief ADC Channel
 */
#define ADC_CHANNEL_PA2                 0x00000001  // channel 0
#define ADC_CHANNEL_PA3                 0x00000002  // channel 1
#define ADC_CHANNEL_PA4                 0x00000004  // channel 2
#define ADC_CHANNEL_PA5                 0x00000008  // channel 3
#define ADC_CHANNEL_PA6                 0x00000010  // channel 4
#define ADC_CHANNEL_PA7                 0x00000020  // channel 5
#define ADC_CHANNEL_PB0                 0x00000040  // channel 6
#define ADC_CHANNEL_PB1                 0x00000080  // channel 7
#define ADC_CHANNEL_PB3                 0x00000100  // channel 8
#define ADC_CHANNEL_PB4                 0x00000200  // channel 9
#define ADC_CHANNEL_PB8                 0x00000400  // channel 10
#define ADC_CHANNEL_VRTC                0x00000800  // channel 11
#define ADC_CHANNEL_WK0                 0x00001000  // channel 12
#define ADC_CHANNEL_BG                  0x00002000  // channel 13
#define ADC_CHANNEL_VBAT                0x00004000  // channel 14
#define ADC_CHANNEL_VUSB                0x00008000  // channel 15
#define ADC_CHANNEL_TSEN                0x00010000  // channel 15'
#define ADC_CHANNEL_VC2SAR              0x00020000  // channel 15''

#define ADC_CHANNEL_VBG                 ADC_CHANNEL_BG
#define ADC_VOLTAGE_VBG                 pmu_get_vbg_mv()

/**
 * @brief SARADC ANALOG BIT
 */
typedef enum {
    SARADC_ANALOG_BIT_VBAT      = 0x01,
    SARADC_ANALOG_BIT_BG        = 0x02,
    SARADC_ANALOG_BIT_WK0       = 0x04 ,
    SARADC_ANALOG_BIT_VRTC      = 0x08,
    SARADC_ANALOG_BIT_VUSB      = 0x10,
} SARADC_ANALOG_BIT_TYPEDEF;

/**
 * @brief SARADC Flag Definition
 */
typedef enum {
    SARADC_IT_FINISHED                  = 0x01,
} SARADC_IT_TYPEDEF;

/**
 * @brief SARADC Analog Mode Enumeration
 */
typedef enum {
    SARADC_MODE_NORMAL                  = 0x00000000,
    SARADC_MODE_TRIM                    = 0x00100000,
} SARADC_MODE_TYPEDEF;

/**
 * @brief SARADC Auto Enable Analog Enumeration
 */
typedef enum {
    SARADC_AUTO_ENABLE_ANALOG_DIS       = 0x00000000,
    SARADC_AUTO_ENABLE_ANALOG_EN        = 0x00080000,
} SARADC_AUTO_ENABLE_ANALOG_TYPEDEF;

/**
 * @brief SARADC Auto Enable IO Analog Enumeration
 */
typedef enum {
    SARADC_AUTO_ENABLE_ANALOG_IO_DIS    = 0x00000000,
    SARADC_AUTO_ENABLE_ANALOG_IO_EN     = 0x00040000,
} SARADC_AUTO_ENABLE_ANALOG_IO_TYPEDEF;

/**
 * @brief SARADC Internel Pullup Enumeration
 */
typedef enum {
    SARADC_PULLUP_DIS                   = 0x00000000,
    SARADC_PULLUP_EN                    = 0x00000001,
} SARADC_PULLUP_TYPEDEF;

/**
 * @brief SARADC setup timing enumeration
 */
typedef enum {
    SARADC_ST_0_CLK                     = 0x00000000,
    SARADC_ST_2_CLK                     = 0x00000001,
    SARADC_ST_4_CLK                     = 0x00000010,
    SARADC_ST_8_CLK                     = 0x00000011,
} SARADC_SETUP_TYPEDEF;

/**
 * @brief SARADC Base Init Structure
 */
typedef struct {
    ///maximum is 1MHz
    uint32_t                                baud;
    SARADC_MODE_TYPEDEF                     mode;
    SARADC_AUTO_ENABLE_ANALOG_TYPEDEF       auto_analog;
    SARADC_AUTO_ENABLE_ANALOG_IO_TYPEDEF    auto_analog_io;
} saradc_base_init_typedef;

/**
 * @brief SARADC Channel Init structure
 */
typedef struct {
    uint32_t                                channel;
    SARADC_PULLUP_TYPEDEF                   pullup_en;
    SARADC_SETUP_TYPEDEF                    setup_time;
} saradc_channel_init_typedef;

/**
 * @brief  Initializes the saradc peripheral according to the specified
 *         parameters in the saradc_base_init_struct.
 * @param  saradc_base_init_struct: pointer to a sadc_init_typedef structure that
 *         contains the configuration information for the specified SARADC peripheral.
 * @retval None
 */
void saradc_init(saradc_base_init_typedef* saradc_init_struct);

/**
 * @brief  Deinit all channel config.
 * @retval the enumeration of channels that has enable.
 */
uint8_t saradc_deinit(void);

/**
 * @brief  Config saradc specified channel.
 * @param  saradc_channel_init_struct: the struct including the info of specified channel.
 * @retval None
 */
void saradc_channel_init(saradc_channel_init_typedef *saradc_channel_init_struct);

/**
  * @brief  Enable or disable the specified SARADC peripheral.
  * @param  state: state of the SARADC peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void saradc_cmd(FUNCTIONAL_STATE state);

/**
  * @brief  SARADC start kick.
  * @param  adc_ch: the adc channels.
  * @retval None
  */
void saradc_kick_start(uint32_t adc_ch);

/**
  * @brief  Set the SARADC baud register value.
  * @param  baud: specifies the baud register new value.
  * @retval None
  */
void saradc_set_baud(uint32_t baud);

/**
 * @brief  Enable or disable the specified SARADC interrupt.
 * @param  isr: Function to be executed for service interruption.
 * @param  pr: Priority of service interruption.
 * @param  interrupt_type: specifies the SARADC interrupt sources to be enable or disable.
 *         this parameter can be one of the following values:
 *             @arg SARADC_IT_FINISHED: All channel finished flag.
 * @param  state: state of the SARADC peripheral.
 *         this parameter can be: EANBLE or DISABLE.
 * @retval None
 */
void saradc_pic_config(isr_t isr, int pr, SARADC_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state);

/**
 * @brief  Get the SARADC interrupt pending.
 * @param  interrupt_type: specifies the IT to get.
 *         this parameter can be on of the following values:
 *             @arg SARADC_IT_FINISHED: SARADC all channel finished interrupt.
 * @retval The state of interrupt_type (SET or RESET).
 */
FLAG_STATE saradc_get_flag(SARADC_IT_TYPEDEF interrupt_type);

/**
  * @brief  Get the SARADC specifies channel finished statue.
  * @param  adc_ch: the channel that wants to query.
  * @retval The state of adc_ch finish (SET or RESET).
  */
FLAG_STATE saradc_get_channel_flag(uint32_t adc_ch);

/**
 * @brief  Clear the SARADC interrupt pending.
 * @param  interrupt_type: specifies the IT to clear.
 *         this parameter can be on of the following values:
 *             @arg SARADC_IT_FINISHED: SARADC all channel finished interrupt.
 * @retval None
 */
void saradc_clear_flag(SARADC_IT_TYPEDEF interrupt_type);

/**
  * @brief  Get the SARADC specifies channel value.
  * @param  adc_chx: the channel that wants to query.
  * @retval saradc data.
  */
uint32_t saradc_get_data(uint32_t adc_chx);

#endif  // _DRIVER_SARADC_H

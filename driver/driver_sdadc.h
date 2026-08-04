/*
 * @File name    : driver_sdadc.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-07-23
 * @Description  : This file contains all the functions prototypes for the SDADC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SDADC_H
#define _DRIVER_SDADC_H

#include "driver_com.h"

/****************************AUDIO MIC**********************************/

/**
 * @brief Flag Enumeration.
 */
typedef enum {
    SDADC_FLAG_DMA_ALL_DONE     = 0x01,
    SDADC_FLAG_DMA_HALF_DONE    = 0x02,
    SDADC_FLAG_SINGLE_SAMPLE    = 0x04,
} SDADC_FLAG_TYPEDEF;

/**
 * @brief Sample Rate Enumeration.
 */
typedef enum {
    SPR_48000                   = 0x00000000,
    SPR_32000                   = 0x00000001,
    SPR_24000                   = 0x00000002,
    SPR_16000                   = 0x00000003,
    SPR_12000                   = 0x00000004,
    SPR_8000                    = 0x00000005,
    SPR_96000                   = 0x00000006,
    SPR_192000                  = 0x00000007,  //no support digital GAIN
    SPR_384000                  = 0x00000008,  //no support digital GAIN
} SDADC_SAMPLING_TYPEDEF;

/**
 * @brief Bits Stream Select Enumeration.
 */
typedef enum {
    BITS_SEL_ITSEL_CH           = 0x00000000,
    BITS_SEL_DACL_SPK           = 0x000c0000,
    BITS_SEL_DACR_SPK           = 0x000e0000,
} SDADC_BITS_SEL_TYPEDEF;

/**
 * @brief Remove DC Order Select Enumeration.
 */
typedef enum {
    RMDC_ORDER_1               = 0x00000000,
    RMDC_ORDER_2               = 0x00000002,
} SDADC_RMDC_ORDER_TYPEDEF;

/**
 * @brief Remove DC Select Enumeration.
 */
typedef enum {
    RMDC_SEL_11BIT              = 0x00000000,
    RMDC_SEL_6BIT               = 0x00000004,
    RMDC_SEL_8BIT               = 0x00000008,
    RMDC_SEL_9BIT               = 0x0000000c,
    RMDC_SEL_10BIT              = 0x00000010,
    RMDC_SEL_12BIT              = 0x00000014,
    RMDC_SEL_13BIT              = 0x00000018,
    RMDC_SEL_14BIT              = 0x0000000c,
} SDADC_RMDC_SEL_TYPEDEF;
//
///**
// * @brief Digital Gain Select Enumeration.
// */
//typedef enum {
//    DIG_GAIN_400                = 0x0400,
//    DIG_GAIN_800                = 0x0800,
//    DIG_GAIN_1600               = 0x1600,
//    DIG_GAIN_3200               = 0x3200,
//    DIG_GAIN_4800               = 0x4800,
//    DIG_GAIN_6400               = 0x6400,
//    DIG_GAIN_8000               = 0x8000,
//} SDADC_DIGITAL_GAIN_TYPEDEF;

/**
 * @brief DMA Data Mode Enumeration.
 */
typedef enum {
    DMA_DATA_MODE_24BITS        = 0x0000,
    DMA_DATA_MODE_16BITS        = 0x0004,
} DMA_DATA_MODE_TYPEDEF;


/*****************************DC MEASUREMENT**********************************/

/**
 * @brief DC Flag Enumeration.
 */
typedef enum {
    SDADC_DC_FLAG_SMP_DONE     = 0x01,
    SDADC_DC_FLAG_CMP_DONE     = 0x02,
    SDADC_DC_FLAG_OUPUT_DONE   = 0x04,
} SDADC_DC_FLAG_TYPEDEF;

/**
 * @brief DC Sign Select Enumeration.
 */
typedef enum {
    DC_SIGNED_NUMBER           = 0x00000000,
    DC_UNSIGNED_NUMBER         = 0x00004000,
} DC_SIGN_SEL_TYPEDEF;

/**
 * @brief DC Sample Channel Select Enumeration.
 */
typedef enum {
    DC_SAMP_CH_NONE             = 0x00000000,
    DC_SAMP_CH0_VBAT_EN         = 0x00000020,
    DC_SAMP_CH1_PA2_EN          = 0x00000040,
    DC_SAMP_CH2_PA3_EN          = 0x00000080,
} DC_CH_SEL_TYPEDEF;

/**
 * @brief DC Sample Rate Enumeration.
 */
typedef enum {
    DC_SPR_1500000             = 0x00000000,
    DC_SPR_3000000             = 0x00000004,
    DC_SPR_6000000             = 0x00000008,
    DC_SPR_750000              = 0x0000000c,
    DC_SPR_375000              = 0x00000010,
    DC_SPR_187000              = 0x00000011,
} DC_SAMPLING_TYPEDEF;

/**
 * @brief DC Compare Direction Enumeration.
 */
typedef enum {
    DC_CMP_DIR_LESS            = 0x00000000,  //channel value is less than the target value
    DC_CMP_DIR_MORE            = 0x00000040,  //channel value is more than the target value
} DC_CMP_DIR_TYPEDEF;

/**
 * @brief DC Compare Channel Select Enumeration.
 */
typedef enum {
    DC_CMP_CH0_VBAT_EN         = 0x00000002,
    DC_CMP_CH1_PA2_EN          = 0x00000004,
    DC_CMP_CH2_PA3_EN          = 0x00000008,
} DC_CMP_CH_SEL_TYPEDEF;

/**
 * @brief DC Compare Output Pin Select Enumeration.
 */
typedef enum {
    DC_CMP_OP_PIN_NONE         = 0x00000000,
    DC_CMP_OP_PIN_PA0          = 0x00000001,
    DC_CMP_OP_PIN_PA1          = 0x00000002,
} DC_CMP_OP_PIN_TYPEDEF;

/**
 * @brief DC Compare Output Voltage Select Enumeration.
 */
typedef enum {
    DC_CMP_OP_VOLT_VDDIO       = 0x00000000,
    DC_CMP_OP_VOLT_VBAT        = 0x00000001,
} DC_CMP_OP_VOLT_TYPEDEF;

/**
 * @brief SDADC Initialization Struct./
 */
typedef struct {
    //adc mode
    SDADC_MODE_TYPEDEF adc_mode;

    /*************audio******************/
    //adccon
    FUNCTIONAL_STATE swclk_en;
    uint16_t swclk_peri;
    uint16_t swclk_duty;
    //adc0con
    SDADC_SAMPLING_TYPEDEF sample_rate;
    SDADC_BITS_SEL_TYPEDEF bits_sel;
    FUNCTIONAL_STATE n6db_en;
    FUNCTIONAL_STATE inv_en;
    //adc0gain
    uint16_t dig_gain;
    //adc0dccon
    FUNCTIONAL_STATE rmdc_en;
    SDADC_RMDC_ORDER_TYPEDEF rmdc_order;
    SDADC_RMDC_SEL_TYPEDEF rmdc_sel;
    FUNCTIONAL_STATE sw_dc_en;
    FUNCTIONAL_STATE get_dc_en;
    uint32_t sw_dc_value;

    /*************dc measurement******************/
    FUNCTIONAL_STATE dc_auto_ana_en;
    DC_SIGN_SEL_TYPEDEF dc_sign_sel;
    uint8_t  dc_done_cnt;
    DC_CH_SEL_TYPEDEF dc_ch;
    DC_SAMPLING_TYPEDEF dc_spr;
    uint16_t dc_cmp_thresh;
    FUNCTIONAL_STATE dc_hvio_hold_en;
    DC_CMP_DIR_TYPEDEF dc_cmp_dir;
    DC_CMP_CH_SEL_TYPEDEF dc_cmp_ch;
    DC_CMP_OP_PIN_TYPEDEF dc_cmp_op_pin;
    DC_CMP_OP_VOLT_TYPEDEF dc_cmp_op_volt;
} sdadc_init_typedef;

/**
 * @brief  Initializes the SDADC peripheral according to the specified
 *         parameters in the sdadc_init_struct.
 * @param  sdadc: it can be SDADC_REG
 * @param  sdadc_init_struct: pointer to a sdadc_init_typedef structure that
 *         contains the configuration information for the specified SDADC peripheral.
 * @retval None
 */
void sdadc_init(sdadc_typedef* sdadc, sdadc_init_typedef *sdadc_init_struct);

/**
 * @brief  De-initialize the specified SDADC peripheral.
 * @param  sdadc: it can be SDADC_REG
 * @retval None
 */
void sdadc_deinit(sdadc_typedef* sdadc);

/**
 * @brief  Enable or disable the specified SDADC0 peripheral.
 * @param  sdadc: it can be SDADC_REG
 * @param  state: the state of the SDADC peripheral.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void sdadc0_cmd(sdadc_typedef* sdadc, FUNCTIONAL_STATE state);

/**
 * @brief  Enable or disable the specified SDADC peripheral.
 * @param  state: the state of the SDADC peripheral.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void sdadc_total_cmd(sdadc_typedef* sdadc, FUNCTIONAL_STATE state);

/**
 * @brief  Enable or disable the SDADC DMA function.
 * @param  sdadc: it can be SDADC_REG
 * @param  addr: DMA start address.
 * @param  len: DMA transmit len.
 * @param  data_mode: DMA data mode.
 * @param  state: the new state need to config.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void sdadc_dma_cmd(sdadc_typedef* sdadc, u32 addr, u16 len, DMA_DATA_MODE_TYPEDEF data_mode, FUNCTIONAL_STATE state);

/**
 * @brief  Enable or disable the specified SDADC interrupt.
 * @param  sdadc: it can be SDADC_REG
 * @param  isr: Function to be executed for service interruption.
 * @param  pr: Priority of service interruption.
 * @param  flag_type: specifies the SDADC interrupt sources to be enable or disable.
 *         this parameter can be one of the following values:
 *             @arg SDADC_FLAG_SINGLE_SAMPLE: Single sampling complete interrupt.
 *             @arg SDADC_FLAG_DMA_ALL_DONE: DMA full-transfer interrupt.
 *             @arg SDADC_FLAG_DMA_HALF_DONE: DMA half-transfer interrupt.
 * @param  state: the state of the SDADC peripheral's interrupt.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void sdadc_pic_config(sdadc_typedef* sdadc, isr_t isr, int pr, SDADC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
 * @brief  Check the specified SDADC flag is set or not.
 * @param  sdadc: it can be SDADC_REG
 * @param  flag_type: specifies the flag to check.
 *         the parameter can be one of the following values:
 *             @arg SDADC_FLAG_SINGLE_SAMPLE: Single sampling complete interrupt.
 *             @arg SDADC_FLAG_DMA_ALL_DONE: DMA full-transfer interrupt.
 *             @arg SDADC_FLAG_DMA_HALF_DONE: DMA half-transfer interrupt.
 * @retval the state of flag_type (SET or RESET).
 */
FLAG_STATE sdadc_get_flag(sdadc_typedef* sdadc, SDADC_FLAG_TYPEDEF flag_type);

/**
 * @brief  Clear the SDADC pending.
 * @param  flag_type: specifies flag to clear.
 *         the parameter can be one of the following values:
 *             @arg SDADC_FLAG_SINGLE_SAMPLE: Single sampling complete interrupt.
 *             @arg SDADC_FLAG_DMA_ALL_DONE: DMA full-transfer interrupt.
 *             @arg SDADC_FLAG_DMA_HALF_DONE: DMA half-transfer interrupt.
 * @retval None
 */
void sdadc_clear_flag(sdadc_typedef* sdadc, SDADC_FLAG_TYPEDEF flag_type);

/**
 * @brief  Set digital gain function.
 * @param  sdadc: it can be SDADC_REG
 * @param  dig_gain: the value of digital gain need to set.
 * @retval None
 */
void sdadc_digital_gain_set(sdadc_typedef* sdadc, uint16_t dig_gain);

/**
 * @brief  Enable or disable the specified SDADC DC interrupt.
 * @param  sdadc: it can be SDADC_REG
 * @param  isr: Function to be executed for service interruption.
 * @param  pr: Priority of service interruption.
 * @param  flag_type: specifies the SDADC interrupt sources to be enable or disable.
 *         this parameter can be one of the following values:
 *             @arg SDADC_DC_FLAG_SMP_DONE: sampling complete interrupt.
 *             @arg SDADC_DC_FLAG_CMP_DONE: compare complete interrupt.
 *             @arg SDADC_DC_FLAG_OUPUT_DONE: ouput complete interrupt.
 * @param  state: the state of the SDADC peripheral's interrupt.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None
 */
void sdadc_dc_pic_config(sdadc_typedef* sdadc, isr_t isr, int pr, SDADC_DC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
 * @brief  Check the specified SDADC DC flag is set or not.
 * @param  sdadc: it can be SDADC_REG
 * @param  flag_type: specifies the flag to check.
 *         the parameter can be one of the following values:
 *             @arg SDADC_DC_FLAG_SMP_DONE: sampling complete interrupt.
 *             @arg SDADC_DC_FLAG_CMP_DONE: compare complete interrupt.
 *             @arg SDADC_DC_FLAG_OUPUT_DONE: ouput complete interrupt.
 * @retval the state of flag_type (SET or RESET).
 */
FLAG_STATE sdadc_dc_get_flag(sdadc_typedef* sdadc, SDADC_DC_FLAG_TYPEDEF flag_type);

/**
 * @brief  Clear the SDADC DC pending.
 * @param  sdadc: it can be SDADC_REG
 * @param  flag_type: specifies flag to clear.
 *         the parameter can be one of the following values:
 *             @arg SDADC_DC_FLAG_SMP_DONE: sampling complete interrupt.
 *             @arg SDADC_DC_FLAG_CMP_DONE: compare complete interrupt.
 *             @arg SDADC_DC_FLAG_OUPUT_DONE: ouput complete interrupt.
 * @retval None
 */
void sdadc_dc_clear_flag(sdadc_typedef* sdadc, SDADC_DC_FLAG_TYPEDEF flag_type);

/**
 * @brief  SDADC DC KICK function.
 * @param  sdadc: it can be SDADC_REG
 * @param  auto_en: FUNCTIONAL_STATE.
 * @param  kick_en: FUNCTIONAL_STATE.
 * @retval None
 */
void sdadc_dc_kick(sdadc_typedef* sdadc, FUNCTIONAL_STATE auto_en, FUNCTIONAL_STATE kick_en);

/**
 * @brief  Set DC short-circuit protect function.
 * @param  sdadc: it can be SDADC_REG
 * @param  hold_en: FUNCTIONAL_STATE.
 * @retval None
 */
void sdadc_dc_hvio_hold_set(sdadc_typedef* sdadc, FUNCTIONAL_STATE hold_en);

/**
 * @brief  Get DCshort-circuit flag function.
 * @param  sdadc: it can be SDADC_REG
 * @retval FLAG_STATE.
 */
FLAG_STATE sdadc_dc_hvio_hold_flag_get(sdadc_typedef* sdadc);

/**
 * @brief  Get DC channel data function.
 * @param  sdadc: it can be SDADC_REG
 * @param  channel.
 * @retval channel data.
 */
uint16_t sdadc_dc_ch_data_get(sdadc_typedef* sdadc, DC_CH_SEL_TYPEDEF channel);

/**
 * @brief  Get DC channel voltage mv function.
 * @param  sdadc: it can be SDADC_REG
 * @param  channel.
 * @retval channel voltage mv.
 */
uint16_t sdadc_dc_ch_volt_mv_get(sdadc_typedef* sdadc, DC_CH_SEL_TYPEDEF channel);

#endif

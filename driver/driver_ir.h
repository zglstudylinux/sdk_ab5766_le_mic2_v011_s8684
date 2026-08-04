/*
 * @File name    : driver_ir.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-11
 * @Description  : This file contains all the functions prototypes for the IR library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_IR_H_
#define _DRIVER_IR_H_

#include "driver_com.h"

////////// irrx
//1M时钟下NEC对应的解码timing
///tmrcnt(is a counter which clk domin in irrxclk),tmrsub = tmrcnt - xxxPR
///xxxERR is compare tmrsub,if tmrsub less than xxxERR and irrx arrive edge,irrx data update 1bit in data state
///for example, if RPTERR_CNT > (tmrcnt - RPTPR_CNT) and irrx arrive edge,irrx data update 1bit in data state
#define RPTERR_CNT              500
#define DATERR_CNT              500
#define ONEERR_CNT              250
#define ZEROERR_CNT             125
#define TOPR_CNT                1750

///The total duration of repeat signal(9000us + 2250us)
#define RPTPR_CNT               11249
///The total duration of start signal(9000us + 4500us)
#define DATPR_CNT               13499
///The total duration of code 0(560us + 560us)
#define DATA_0_CNT              1119
///The total duration of code 1(560us + 1690us)
#define DATA_1_CNT              2249

typedef enum {
    IRRX_FLAG_RXPND             = 1 << 0,
    IRRX_FLAG_KEYRELS           = 1 << 1,
} IRRX_FLAG_TYPEDEF;

typedef enum {
    IRTX_FLT_SRC_RXIN           = 0 << 1,
    IRTX_FLT_SRC_TMR3_CAP_IRF   = 1 << 1,
} IRTX_FILTER_SRC_TYPEDEF;

typedef enum {
    IRRX_DATA_SELECT_32BIT      = 0 << 0,
    IRRX_DATA_SELECT_16BIT      = 0 << 2,
} IRRX_DATA_SEL_TYPEDEF;


// irtx
/**
 * @brief Flag Enumeration.
 */
typedef enum {
    IRTX_FLAG_TX_PND           = 0x01,
    IRTX_FLAG_ODMA_ALL_PND     = 0x02,  //capture
    IRTX_FLAG_ODMA_HALF_PND    = 0x04,
    IRTX_FLAG_IDMA_ALL_PND     = 0x08,  //learn
    IRTX_FLAG_IDMA_HALF_PND    = 0x10,
} IRTX_FLAG_TYPEDEF;

typedef enum {
    IRTX_ENCODE_FORMAT_NEC      = 0 << 8,    //carrier 38KHz，duty:1/3、1/4
    IRTX_ENCODE_FORMAT_TC9012   = 1 << 8,    //carrier 38KHz，duty:1/3
    IRTX_ENCODE_FORMAT_RC5      = 2 << 8,    //carrier 36KHz，duty:1/3、1/4
    IRTX_ENCODE_FORMAT_RC6      = 3 << 8,    //carrier 36KHz，duty:1/3、1/4

    IRTX_ENCODE_FORMAT_CUSTOM   = 0xff,      //define by user
} IRTX_ENCODE_FORMAT_TYPEDEF;

typedef enum {
    IRTX_REP_DIS    = 0 << 0,
    IRTX_REP_EN     = 1 << 1,
} IRTX_REP_EN_TYPEDEF;

/*if tc9012 indicated as sync code
    0: sync code is "0"
    1: sync code is "1"
  if rc6 indicated as trailer code
    0: trailer code is "0"
    1: trailer code is "1"
*/
typedef enum {
    IRTX_SYNC_OR_TRAILER_CODE_0  = 0 << 2,
    IRTX_SYNC_OR_TRAILER_CODE_1  = 1 << 2,
} IRTX_SYNC_CODE_TYPEDEF;

typedef enum {
    IRTX_REP_SEL_NEC_OR_TC9O12   = 0 << 1,
    IRTX_REP_SEL_RC5_OR_RC6      = 1 << 1,
} IRTX_REP_SEL_TYPEDEF;

typedef enum {
    IRTX_MOD_CLKSEL_3M = 0,
    IRTX_MOD_CLKSEL_4M = 1,
} IRTX_MOD_CLKSEL_TYPEDEF;

typedef enum {
    IRTX_CAP_EDGE_FALLING  = 0 << 10,
    IRTX_CAP_EDGE_RISING   = 1 << 10,
} IRTX_CAP_EDGE_TYPEDEF;

typedef struct {
    FUNCTIONAL_STATE        carrier_en;
    IRTX_MOD_CLKSEL_TYPEDEF mod_clksel;         // Tx Modulation Clock Select.
    uint8_t                 ircw_duty;          // Tx Carrier Duty = (ircw_duty) / (ircw_length + 1))
    uint8_t                 ircw_length;        // Tx Carrier Freq = (3M or 4M) / (ircw_length + 1))
} irtx_carrier_config_typedef;

typedef struct {
    FUNCTIONAL_STATE        capture_en;
    IRTX_CAP_EDGE_TYPEDEF   capture_edge;
    FUNCTIONAL_STATE        cap_dma_en;
    uint16_t                cap_dma_size;
    uint32_t                cap_dma_addr;
} ir_capture_config_typedef;

typedef struct {
    FUNCTIONAL_STATE        filter_en;         // IR Filter Enable(In Capture Mode Or IR_RX Mode)
    IRTX_FILTER_SRC_TYPEDEF filter_src;
    uint8_t                 filter_len;
} ir_filter_config_typedef;

typedef struct {
    IRTX_ENCODE_FORMAT_TYPEDEF      encode_format;
    irtx_carrier_config_typedef     carrier;
} irtx_init_typedef;

//TIMING信息结构体
typedef struct
{
    // irtx select encoding format in modulatin mode. 0-nec;  1-tc9012;  2-rc5;  3-rc6.
    uint16_t encode_format;
    // irtx code "1" mode. 0-mark + space;   1-space + mark.
    uint16_t code_1_mode;
    // irtx code "0" mode. 0-mark + space;   1-space + mark.
    uint16_t code_0_mode;
    // irtx code "1" mark time = (m1 + 1) * (1M or 32K).
    uint16_t code_1_mask;
    // irtx code "1" space time = (s1 + 1) * (1M or 32K).
    uint16_t code_1_space;
    // irtx code "0" mark time = (m0 + 1) * (1M or 32K).
    uint16_t code_0_mask;
    // irtx code "0" space time = (s0 + 1) * (1M or 32K).
    uint16_t code_0_space;
    // irtx start code "1" time = (t1 + 1) * (1M or 32K).
    uint16_t start_1_time;
    // irtx start code "0" time = (t0 + 1) * (1M or 32K).
    uint16_t start_0_time;
    //irtx repeat code "1" time = (r1 + 1) * (1M or 32K).
    uint16_t repeat_1_time;
    // irtx repeat code "0" time = (r0 + 1) * (1M or 32K).
    uint16_t repeat_0_time;
    // irtx the interval between repeat time = (rt + 1) * (1M or 32K).
    uint32_t repeat_interval;
    // irtx end pulse time = (et + 1) * (1M or 32K).
    uint16_t end_pulse_time;
    // irtx sync code in tc9012 or trailer code in rc6.
    uint16_t sync_code;
    // irtx repeat mode.   0-indicates duplicate code(nec or tc9012); 1-indicates no duplicate code(rc5 or rc6)
    uint16_t repeat_mode;
    // irtx length
    uint16_t tx_len;
} irtx_timing_typedef;

typedef struct
{
    IRRX_DATA_SEL_TYPEDEF       data_bits;
    ir_filter_config_typedef    filter;
} irrx_init_typedef;



//1m时钟下TIMING数据
#ifndef IRTX_TIMING_NEC
#define IRTX_TIMING_NEC    {.encode_format = IRTX_ENCODE_FORMAT_NEC,\
                            .code_1_mode = 0,           .code_0_mode = 0,\
                            .code_1_mask = 559,         .code_1_space = 1689,\
                            .code_0_mask = 559,         .code_0_space = 559,\
                            .start_1_time = 8999,       .start_0_time = 4499,\
                            .repeat_1_time = 8999,      .repeat_0_time = 2249,\
                            .repeat_interval = 108000,  .end_pulse_time = 559,\
                            .sync_code = 0,             .repeat_mode = 0,\
                            .tx_len = 32 }
#endif

#ifndef IRTX_TIMING_TC9012
#define IRTX_TIMING_TC9012  {.encode_format = IRTX_ENCODE_FORMAT_TC9012,\
                            .code_1_mode = 0,           .code_0_mode = 0,\
                            .code_1_mask = 559,         .code_1_space = 1689,\
                            .code_0_mask = 559,         .code_0_space = 559,\
                            .start_1_time = 8999,       .start_0_time = 4499,\
                            .repeat_1_time = 4499,      .repeat_0_time = 2249,\
                            .repeat_interval = 108000,  .end_pulse_time = 559,\
                            .sync_code = 1,             .repeat_mode = 0,\
                            .tx_len = 32 }
#endif

#ifndef IRTX_TIMING_RC5
#define IRTX_TIMING_RC5     {.encode_format = IRTX_ENCODE_FORMAT_RC5,\
                            .code_1_mode = 1,           .code_0_mode = 0,\
                            .code_1_mask = 844,         .code_1_space = 844,\
                            .code_0_mask = 844,         .code_0_space = 844,\
                            .start_1_time = 1687,       .start_0_time = 1687,\
                            .repeat_1_time = 0,         .repeat_0_time = 0,\
                            .repeat_interval = 114000,  .end_pulse_time = 17,\
                            .sync_code = 0,             .repeat_mode = 1,\
                            .tx_len = 12 }
#endif

#ifndef IRTX_TIMING_RC6
#define IRTX_TIMING_RC6     {.encode_format = IRTX_ENCODE_FORMAT_RC6,\
                            .code_1_mode = 0,           .code_0_mode = 1,\
                            .code_1_mask = 443,         .code_1_space = 443,\
                            .code_0_mask = 443,         .code_0_space = 443,\
                            .start_1_time = 2665,       .start_0_time = 888,\
                            .repeat_1_time = 0,         .repeat_0_time = 0,\
                            .repeat_interval = 86000,   .end_pulse_time = 17,\
                            .sync_code = 0,             .repeat_mode = 1,\
                            .tx_len = 16 }
#endif

/**
  * @brief  Initializes the IRRX peripheral according to the specified parameters in the irrx_param.
  * @param  irrx_reg: IRRX.
  * @param  irrx_param: IRRX params.
  * @retval None
  */
void irrx_init(irrx_typedef *irrx_reg, irrx_init_typedef *irrx_param);

/**
 * @brief  Enable or disable the specified IRRX interrupt.
 * @param  irrx_reg: IRRX
 * @param  isr: function to be executed for service interruption.
 * @param  pr: Priority of service interruption.
 * @param  interrupt_type: specifies the IRRX interrupt sources to be enable or disable.
 *         this parameter can be one of the following values:
 *             @arg IRRX_FLAG_RXPND: completion a key-value.
 *             @arg IRRX_FLAG_KEYRELS: received Key up.
 * @param  state: the state of the IRRX peripheral.
 *         this parameter can be: ENABLE or DISABLE.
 * @retval None.
 */
void irrx_pic_config(irrx_typedef *irrx_reg, isr_t isr, int pr, IRRX_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
 * @brief  Check the specified IRRX flag is set or not.
 * @param  irrx_reg: IRRX
 * @param  flag_type: specifies the flag to check.
 *         the parameter can be one of the following values:
 *             @arg IRRX_FLAG_RXPND: completion a key-value.
 *             @arg IRRX_FLAG_KEYRELS: received Key up.
 * @retval the state of flag_type (SET or RESET).
 */
FLAG_STATE irrx_get_flag(irrx_typedef *irrx_reg, IRRX_FLAG_TYPEDEF flag_type);

/**
 * @brief  Clear interrupt pending function.
 * @param  irrx_reg: IRRX
 * @param  interrupt_type: specifies the interrupt to clear.
 *         this parameter can be one of the following values:
 *             @arg IRRX_FLAG_RXPND: completion a key-value.
 *             @arg IRRX_FLAG_KEYRELS: received Key up.
 * @retval None.
 */
void irrx_clear_flag(irrx_typedef *irrx_reg, IRRX_FLAG_TYPEDEF flag_type);

/**
 * @brief  Get the specified IRRX data.
 * @param  irrx_reg: IRRX
 * @retval rx data.
 */
uint32_t irrx_get_data(irrx_typedef *irrx_reg);

/**
  * @brief  Enable or disable the specified IRRX peripheral.
 * @param  irrx_reg: IRRX
  * @param  state: state of the IRRX peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void irrx_cmd(irrx_typedef *irrx_reg, FUNCTIONAL_STATE state);

/**
 * @brief  De-initialize the specified IRRX peripheral.
 * @param  irrx_reg: IRRX
 * @retval None
 */
void irrx_deinit(irrx_typedef *irrx_reg);

void irrx_filter_init(const ir_filter_config_typedef *filter_param);

//tx
void irtx_timing_set(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param, const irtx_timing_typedef timing);

/**
  * @brief  Initializes the IRTX peripheral timing according to the specified parameters in the irtx_param.
  * @param  irtx_reg: IRTX
  * @param  irtx_param: IRTX params.
  * @retval None
  */
void irtx_timing_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param);

void irtx_carrier_init(irtx_typedef *irtx_reg, const irtx_carrier_config_typedef *irtx_carrier_param);

/**
  * @brief  Initializes the IRTX peripheral hardware encode according to the specified parameters in the irtx_param.
  * @param  irtx_reg: IRTX
  * @param  irtx_param: IRTX hardwate encode param.
  * @retval None
  */
void irtx_hw_enc_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param);
/**
  * @brief  Initializes the IRTX peripheral learn according to the specified parameters in the irtx_param.
  * @param  irtx_reg: IRTX
  * @param  irtx_param: IRTX learn param.
  * @retval None
  */
void irtx_learn_init(irtx_typedef *irtx_reg, const irtx_init_typedef *irtx_param);
/**
 * @brief  Set the specified IRTX reapeat.
 * @param  irtx_reg: IRTX
 * @param  en: Enable or disable.
 * @retval None
 */
void irtx_set_repeat(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en);

/**
 * @brief  Check the specified IRTX repeat is set or not.
 * @param  irtx_reg: IRTX
 * @retval 0 or 1.
 */
bool irtx_is_repeat(irtx_typedef *irtx_reg);

/**
 * @brief  Set the specified IRTX invert.
 * @param  irtx_reg: IRTX
 * @param  en: Enable or disable.
 * @retval None
 */
void irtx_set_invert(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en);

/**
 * @brief  Check the specified IRTX invert is set or not.
 * @param  irtx_reg: IRTX
 * @retval 0 or 1.
 */
bool irtx_is_invert(irtx_typedef *irtx_reg);

/**
 * @brief  Set the specified IRTX synchronous code.
 * @param  irtx_reg: IRTX
 * @param  syc: Enable or disable.
 * @retval None
 */
void irtx_set_syc(irtx_typedef *irtx_reg, bool syc);

/**
 * @brief  Check the specified IRTX syc is set or not.
 * @param  irtx_reg: IRTX
 * @retval 0 or 1.
 */
bool irtx_is_syc(irtx_typedef *irtx_reg);

/**
 * @brief  Set the specified IRTX data.
 * @param  irtx_reg: IRTX
 * @param  code: IRTX data.
 * @retval None
 */
void irtx_set_data (irtx_typedef *irtx_reg, uint32_t code);

/**
 * @brief  Get the specified IR capture size.
 * @param  irtx_reg: IRTX.
 * @retval capture size.
 */
uint16_t irtx_get_capture_size(irtx_typedef *irtx_reg);

/**
 * @brief  Kick the specified IRTX learn.
 * @param  irtx_reg: IRTX
 * @param  buf: Learn buf
 * @param  len: Learn buf len
 * @param  en: Enable or disable.
 * @retval None.
 */
void irtx_learn_kick(irtx_typedef *irtx_reg, void *buf, uint32_t len, FUNCTIONAL_STATE en);

/**
 * @brief  Kick the specified IRTX hardwire encode.
 * @param  irtx_reg: IRTX
 * @param  en: Enable or disable.
 * @retval None.
 */
void irtx_hw_enc_kick(irtx_typedef *irtx_reg, FUNCTIONAL_STATE en);

/**
 * @brief  Waiting for IRTX to complete the transmission.
 * @param  irtx_reg: IRTX
 * @retval None.
 */
void irtx_kick_wait(irtx_typedef *irtx_reg);

/**
 * @brief  Enable or disable the specified IRTX interrupt.
 * @param  irtx_reg: IRTX
 * @param  isr: function to be executed for service interruption.
 * @param  pr: priority of service interruption.
 * @param  flag_type: specifies the IRTX interrupt sources to be enable or disable.
 *         This parameter can be one of the following values:
 *             @arg IRTX_FLAG_TX_PND: Complete a transmission.
 *             @arg IRTX_FLAG_ODMA_ALL_PND: Complete a capture.
 *             @arg IRTX_FLAG_ODMA_HALF_PND: Complete half a capture.
 *             @arg IRTX_FLAG_IDMA_ALL_PND: Complete one learning sesion.
 *             @arg IRTX_FLAG_IDMA_HALF_PND: Complete half a cycle of learning.
 * @param  state: the new state need to config.
 * @retval None
 */
void irtx_pic_config(irtx_typedef *irtx_reg, isr_t isr, int pr, IRTX_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
 * @brief  Check the specified IRTX flag is set or not.
 * @param  flag_type: specifies the flag to check.
 *         the parameter can be one of the following values:
 *             @arg IRTX_FLAG_TX_PND: Complete a transmission.
 *             @arg IRTX_FLAG_ODMA_ALL_PND: Complete a capture.
 *             @arg IRTX_FLAG_ODMA_HALF_PND: Complete half a capture.
 *             @arg IRTX_FLAG_IDMA_ALL_PND: Complete one learning sesion.
 *             @arg IRTX_FLAG_IDMA_HALF_PND: Complete half a cycle of learning.
 * @retval The state of flag_type (SET or RESET).
 */
FLAG_STATE irtx_get_flag(irtx_typedef *irtx_reg, IRTX_FLAG_TYPEDEF flag_type);

/**
 * @brief  Clear the specified IRTX pending.
 * @param  flag_type: specified flag to clear.
 *             @arg IRTX_FLAG_TX_PND: Complete a transmission.
 *             @arg IRTX_FLAG_ODMA_ALL_PND: Complete a capture.
 *             @arg IRTX_FLAG_ODMA_HALF_PND: Complete half a capture.
 *             @arg IRTX_FLAG_IDMA_ALL_PND: Complete one learning sesion.
 *             @arg IRTX_FLAG_IDMA_HALF_PND: Complete half a cycle of learning.
 * @retval None
 */
void irtx_clear_flag(irtx_typedef *irtx_reg, IRTX_FLAG_TYPEDEF flag_type);

/**
 * @brief  De-initialize the specified IRTX peripheral.
 * @retval None
 */
void irtx_deinit(irtx_typedef *irtx_reg);
void irtx_cmd(irtx_typedef *irtx_reg, FUNCTIONAL_STATE state);
void irtx_learn_cmd(irtx_typedef *irtx_reg, FUNCTIONAL_STATE state);

void ir_capture_init(irtx_typedef *irtx_reg, const ir_capture_config_typedef *irtx_capture_param);

#endif

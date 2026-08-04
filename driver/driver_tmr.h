/*
 * @File name    : driver_tmr.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-20
 * @Description  : This file contains all the functions prototypes for the TMR library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_TMR_H
#define _DRIVER_TMR_H

#include "driver_com.h"

/**
 * @brief TMR Interrupt Flag Definition
 */
typedef enum {
    TMR_IT_UPDATE           = 1 << 0,
    TMR_IT_CAPTURE          = 1 << 1,
} TMR_IT_TYPEDEF;


/**
 * @brief TMR Mode Selection
 */
typedef enum {
    TMR_TMR_MODE            = 0 << 14,
    TMR_CAP_RISING          = 1 << 14,
    TMR_CAP_FALLING         = 2 << 14,
    TMR_CAP_EDGE            = 3 << 14,
} TMR_MODE_TYPEDEF;


/**
 * @brief Clock Divide Selection
 */
typedef enum {
    TMR_CLOCK_DIV_1        = 0 << 4,
    TMR_CLOCK_DIV_2        = 1 << 4,
    TMR_CLOCK_DIV_4        = 2 << 4,
    TMR_CLOCK_DIV_8        = 3 << 4,
    TMR_CLOCK_DIV_16       = 4 << 4,
    TMR_CLOCK_DIV_32       = 5 << 4,
    TMR_CLOCK_DIV_64       = 6 << 4,
} TMR_PDIVSEL_TYPEDEF;

/**
 * @brief Clock Triggle Selection
 */
typedef enum {
    TMR_SYS_CLK             = 1 << 1,
    TMR_XOSC24M             = 2 << 1,
    TMR_TMR_INC0_CR         = 3 << 1,
    TMR_XOSC_DIV24          = 4 << 1,
    TMR_RTC_RC2M            = 5 << 1,
    TMR_TMR_INC1_CR         = 6 << 1,
    TMR_CLKOUT_PINP         = 7 << 1,
} TMR_PCLKSEL_TYPEDEF;


/**
 * @brief Timer Init Structure
 */
typedef struct {
    uint32_t                period;
    TMR_PCLKSEL_TYPEDEF     clock_source;
    TMR_PDIVSEL_TYPEDEF     clock_div;
    TMR_MODE_TYPEDEF        mode;
} tmr_base_init_typedef;

/********************** Function used to configure timer base **********************/
/**
  * @brief  Initializes the timer base peripheral according to the specified
  *         parameters in the tmr_base_init_typedef.
  * @param  tmrx: TMRx_REG，where x can be (0..3) to select the TMR peripheral.
  * @param  tmr_base_init_struct: pointer to a tmr_base_init_typedef structure that
  *         contains the configuration information for the specified TMR peripheral.
  * @retval None
  */
void tmr_base_init(tmr_typedef *tmrx, tmr_base_init_typedef *tmr_base_init_struct);

/**
 * @brief  De-initialize the specified tmr peripheral.
 * @param  tmrx: TMRx_REG,where x can be (0..3) to select the TMR peripheral.
 * @retval None
 */
void tmr_deinit(tmr_typedef *tmrx);

/**
  * @brief  Set the TMR counter register value.
  * @param  tmrx: TMRx_REG,where x can be (0..3) to select the TMR peripheral.
  * @param  cnt: specifies the counter register new value.
  * @retval None
  */
void tmr_set_counter(tmr_typedef *tmrx, uint32_t cnt);

/**
  * @brief  Set the TMR period register value.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @param  period: specifies the period register new value.
  * @retval None
  */
void tmr_set_period(tmr_typedef *tmrx, uint32_t period);

/**
  * @brief  Get the TMR counter register value.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @retval specifies the counter register value.
  */
uint32_t tmr_get_counter(tmr_typedef *tmrx);

/**
  * @brief  Get the TMR period register value.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @retval specifies the period register value.
  */
uint32_t tmr_get_period(tmr_typedef *tmrx);

/**
  * @brief  Enable or disable the specified TMR peripheral.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @param  state: state of the TMRx peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void tmr_cmd(tmr_typedef *tmrx, FUNCTIONAL_STATE state);

/************************** Function used to set capture ***************************/
/**
  * @brief  Get the TMR capture register value.
  * @param  tmrx: TMRx_REG, where x can be (2..3) to select the TMR peripheral.
  * @retval specifies the capture register value.
  */
uint32_t tmr_get_capture(tmr_typedef *tmrx);

/**
  * @brief  Configure the captured edge.
  * @param  tmrx: TMRx_REG, where x can be (2..3) to select the TMR peripheral.
  * @param  edge_select: TMR_CAP_RISING or TMR_CAP_FALLING or TMR_CAP_EDGE.
  * @retval specifies the capture register value.
  */
void tmr_capture_config(tmr_typedef *tmrx, TMR_MODE_TYPEDEF edge_select);

/********************** Function used to configure interrupt ***********************/
/**
  * @brief  Enable or disable the specified TMRx interrupt.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  interrup_type: specifies the TRMx interrupt sources to be enable or disable.
  *         this parameter can be one of the following values:
  *             @arg TMR_IT_UPDATE: TMR(0..3) overflow interrupt enable bit.
  *             @arg TMR_IT_CAPTURE: TMR(2..3) capture interrupt enable bit.
  * @param  state: the state of the TMRx peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void tmr_pic_config(tmr_typedef *tmrx, isr_t isr, int pr, TMR_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state);

/**
  * @brief  Get the TMR interrupt pending.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @param  interrupt_type: specifies the IT to clear.
  *         this parameter can be one of the following values:
  *             @arg TMR_IT_UPDATE: TMR overflow interrupt.
  *             @arg TMR_IT_CAPTURE: TMR capture interrupt.
  * @retval The state of interrupt_type (SET or RESET).
  */
FLAG_STATE tmr_get_flag(tmr_typedef *tmrx, TMR_IT_TYPEDEF interrupt_type);

/**
  * @brief  Clear the TMR interrupt pending.
  * @param  tmrx: TMRx_REG, where x can be (0..3) to select the TMR peripheral.
  * @param  interrupt_type: specifies the interrupt to clear.
  *         this parameter can be one of the following values:
  *             @arg TMR_IT_UPDATE: TMR overflow interrupt.
  *             @arg TMR_IT_CAPTURE: TMR capture interrupt.
  * @retval None
  */
void tmr_clear_flag(tmr_typedef *tmrx, TMR_IT_TYPEDEF interrupt_type);

#endif // _DRIVER_TMR_H

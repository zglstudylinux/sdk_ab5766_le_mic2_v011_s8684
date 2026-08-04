/*
 * @File name    : driver_pwm.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-15
 * @Description  : This file contains all the functions prototypes for the TMR library.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#ifndef _DRIVER_PWM_H
#define _DRIVER_PWM_H

#include "driver_com.h"

/**
 * @brief PWM Channel Selection
 */
typedef enum {
    PWM_CH0                     = 0,
    PWM_CH1,
    PWM_CH2,
    PWM_CH3,
} PWM_CH_SEL_TYPEDEF;

/**
 * @brief PWM TMR Selection
 */
typedef enum {
    PWM_SEL_TMR0                = 0,
    PWM_SEL_TMR1,
    PWM_SEL_TMR2,
    PWM_SEL_TMR3,
} PWM_TMR_SEL_TYPEDEF;


/**
 * @brief PWM TMR Mode Selection
 */
typedef enum {
    TMRP_TMR_PWM_MODE           = 0 << 14,
    TMRP_CAP_RISI_MODE          = 1 << 14,
    TMRP_CAP_FALL_MODE          = 2 << 14,
    TMRP_CAP_RISI_FALL_MODE     = 3 << 14,
} TMRP_MODE_TYPEDEF;


/**
 * @brief Clock Divide Selection
 */
typedef enum {
    TMRP_CLOCK_DIV_1        = 0 << 4,
    TMRP_CLOCK_DIV_2        = 1 << 4,
    TMRP_CLOCK_DIV_4        = 2 << 4,
    TMRP_CLOCK_DIV_8        = 3 << 4,
    TMRP_CLOCK_DIV_16       = 4 << 4,
    TMRP_CLOCK_DIV_32       = 5 << 4,
    TMRP_CLOCK_DIV_64       = 6 << 4,
} TMRP_PDIVSEL_TYPEDEF;

/**
 * @brief Clock Source Selection
 */

typedef enum {
    TMRP_SYS_CLK            = 0 << 1,
    TMRP_SYS_CLKPPP         = 1 << 1,  // TMRP_PDIVSEL_TYPEDEF can't be set to TMRP_CLOCK_DIV_1 !!!
    TMRP_XOSC24M            = 2 << 1,
    TMRP_TMR_INC0_CR        = 3 << 1,
    TMRP_RC24M              = 4 << 1,
    TMRP_RTC_RC2M           = 5 << 1,
    TMRP_TMR_INC1_CR        = 6 << 1,
    TMRP_CLKOUT_PINP        = 7 << 1,
} TMRP_PCLKSEL_TYPEDEF;

/**
 * @brief TMRP Dead Zone Mode Selection
 */
typedef enum {
    PWM_DZMODE_0          = 0 << 1,
    PWM_DZMODE_1          = 1 << 1,
} PWM_DZMODE_TYPEDEF;


/**
 * @brief PWM Timer Init Structure
 */
typedef struct {
    TMRP_PCLKSEL_TYPEDEF    clock_source;
    TMRP_PDIVSEL_TYPEDEF    clock_div;
    TMRP_MODE_TYPEDEF       tmr_mode;
    uint32_t                period;
    uint32_t                duty;
    uint32_t                cnt;
} pwm_tmr_base_init_typedef;

/**
 * @brief PWM Init Structure
 */
typedef struct {
    PWM_CH_SEL_TYPEDEF      ch;
    PWM_TMR_SEL_TYPEDEF     tmr_sel;
    FUNCTIONAL_STATE        en;
    FUNCTIONAL_STATE        output_inv;
} pwm_base_init_typedef;

/**
 * @brief PWM Dead Zone Init Structure
 */
typedef struct {
    PWM_DZMODE_TYPEDEF      mode;
    uint16_t                len;
} pwm_dz_base_init_typedef;

/********************** Function used to configure pwm timer base **********************/

/**
  * @brief  Initializes the pwm timer base peripheral according to the specified
  *         parameters in the pwm_tmr_base_init_typedef.
  * @param  pwm_tmr: PWM_TMRx.
  * @param  tmrp_base_init_struct: pointer to a pwm_tmr_base_init_typedef structure that
  *         contains the configuration information for the specified PWM TMR peripheral.
  * @retval None
  */
void pwm_tmr_base_init(pwm_tmr_typedef *pwm_tmr, pwm_tmr_base_init_typedef *tmrp_base_init_struct);

/**
 * @brief  De-initialize the specified tmr peripheral.
 * @param  pwm_tmr: PWM_TMRx.
 * @retval None
 */
void pwm_tmr_deinit(pwm_tmr_typedef *pwm_tmr);

/**
  * @brief  Set the PWM TMR counter register value.
  * @param  pwm_tmr: PWM_TMRx.
  * @param  cnt: specifies the counter register new value.
  * @retval None
  */
void pwm_tmr_set_cnt(pwm_tmr_typedef *pwm_tmr, uint32_t cnt);

/**
 * @brief  Set pwm_tmr_typedef duty of specified PWM TMR periperal.
 * @param  pwm_tmr: PWM_TMRx.
 * @param  duty: The high level time of the pwm_tmr_typedef to be configured.
 * @retval
 */
void pwm_tmr_set_duty(pwm_tmr_typedef *pwm_tmr, uint16_t duty);

/**
  * @brief  Set the PWM TMR period register value.
  * @param  pwm_tmr: PWM_TMRx.
  * @param  period: specifies the period register new value.
  * @retval None
  */
void pwm_tmr_set_period(pwm_tmr_typedef *pwm_tmr, uint32_t period);

/**
  * @brief  Get the PWM TMR counter register value.
  * @param  pwm_tmr: PWM_TMRx.
  * @retval specifies the counter register value.
  */
uint32_t pwm_tmr_get_cnt(pwm_tmr_typedef *pwm_tmr);

/**
  * @brief  Get the PWM TMR period register value.
  * @param  pwm_tmr: PWM_TMRx.
  * @retval specifies the period register value.
  */
uint32_t pwm_tmr_get_period(pwm_tmr_typedef *pwm_tmr);

/**
  * @brief  Enable or disable the specified PWM TMR peripheral.
  * @param  pwm_tmr: PWM_TMRx.
  * @param  state: state of the pwm_tmr peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void pwm_tmr_cmd(pwm_tmr_typedef *pwm_tmr, FUNCTIONAL_STATE state);


/********************** Function used to configure pwm ***********************/

/**
  * @brief  Initializes the pwm base peripheral according to the specified
  *         parameters in the pwm_base_init_typedef.
  * @param  pwm: PWM_REG.
  * @param  pwm_base_init_struct: pointer to a pwm_base_init_typedef structure that
  *         contains the configuration information for the specified PWM peripheral.
  * @retval None
  */
void pwm_base_init(pwm_typedef *pwm, pwm_base_init_typedef *pwm_base_init_struct);

/**
  * @brief  Set whether the pwm output is invert.
  * @param  pwm: PWM_REG.
  * @param  ch: pwm channel select.
  * @param  state: ENABLE or DISABLE.
  * @retval None
  */
void pwm_set_output_inv(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, FUNCTIONAL_STATE state);

/**
  * @brief  Select the timer to generating pwm.
  * @param  pwm: PWM_REG.
  * @param  ch: pwm channel select.
  * @param  tmr_sel: pwm timer select.
  * @retval None
  */
void pwm_set_tmr_sel(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, PWM_TMR_SEL_TYPEDEF tmr_sel);

/**
  * @brief  Enable or disable the specified PWM peripheral.
  * @param  pwm: PWM_REG.
  * @param  ch: pwm channel select.
  * @param  state: state of the pwm_tmr peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void pwm_cmd(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, FUNCTIONAL_STATE state);

/********************** Function used to configure interrupt ***********************/

/**
  * @brief  Enable or disable the specified PWM TMR interrupt.
  * @param  pwm_tmr: PWM_TMRx.
  * @param  state: the state of the pwm_tmr peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void pwm_tmr_ie_cmd(pwm_tmr_typedef *pwm_tmr, FUNCTIONAL_STATE state);

/**
  * @brief  Enable or disable the PWM TMR interrupt.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  state: the state of the pwm_tmr peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void pwm_tmr_pic_config(isr_t isr, int pr, FUNCTIONAL_STATE state);

/**
  * @brief  Get the PWM TMR interrupt pending.
  * @param  pwm_tmr: PWM_TMRx.
  * @retval The state of interrupt_type (SET or RESET).
  */
FLAG_STATE pwm_tmr_get_flag(pwm_tmr_typedef *pwm_tmr);

/**
  * @brief  Clear the PWM TMR interrupt pending.
  * @param  pwm_tmr: PWM_TMRx.
  * @retval None
  */
void pwm_tmr_clear_flag(pwm_tmr_typedef *pwm_tmr);

/********************** Function used to configure pwm dead zone ***********************/

/**
  * @brief  Initializes the pwm dead zone according to the specified
  *         parameters in the pwm_dz_base_init_typedef.
  * @param  pwm_dz: PWM_DZx_REG.
  * @param  pwm_dz_init_struct: pointer to a pwm_dz_base_init_typedef structure that
  *         contains the configuration information for the specified PWM dead zone.
  * @retval None
  */
void pwm_dead_zone_init(pwm_dz_typedef *pwm_dz, pwm_dz_base_init_typedef *pwm_dz_init_struct);

/**
  * @brief  Enable or disable the specified PWM dead zone.
  * @param  pwm_dz: PWM_DZx_REG.
  * @param  state: ENABLE or DISABLE.
  * @retval None
  */
void pwm_dead_zone_cmd(pwm_dz_typedef *pwm_dz, FUNCTIONAL_STATE state);

#endif // _DRIVER_PWM_H

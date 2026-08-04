/*
 * @File name    : driver_tmr.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-14
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the PWM peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights reserved.
 */
#include "driver_pwm.h"

void pwm_tmr_base_init(pwm_tmr_typedef *pwm_tmr, pwm_tmr_base_init_typedef *tmrp_base_init_struct)
{
    uint32_t tmp_reg = pwm_tmr->con;

    tmp_reg &= ~(uint32_t)PTMRxCON_PCLKSEL;
    tmp_reg |= tmrp_base_init_struct->clock_source;

    tmp_reg &= ~(uint32_t)PTMRxCON_PRESEL;
    tmp_reg |= tmrp_base_init_struct->clock_div;

    tmp_reg &= ~(uint32_t)PTMRxCON_MODE;
    tmp_reg |= tmrp_base_init_struct->tmr_mode;

    pwm_tmr->period = tmrp_base_init_struct->period;

    pwm_tmr->duty = tmrp_base_init_struct->duty;

    pwm_tmr->cnt = tmrp_base_init_struct->cnt;

    pwm_tmr->con = tmp_reg;
}

void pwm_tmr_deinit(pwm_tmr_typedef *pwm_tmr)
{
    pwm_tmr->con = 0;
}

void pwm_tmr_set_cnt(pwm_tmr_typedef *pwm_tmr, uint32_t cnt)
{
    pwm_tmr->cnt = cnt;
}

void pwm_tmr_set_duty(pwm_tmr_typedef *pwm_tmr, uint16_t duty)
{
    pwm_tmr->duty = duty;
}

void pwm_tmr_set_period(pwm_tmr_typedef *pwm_tmr, uint32_t period)
{
    pwm_tmr->period = period;
}


uint32_t pwm_tmr_get_cnt(pwm_tmr_typedef *pwm_tmr)
{
    return (uint32_t)(pwm_tmr->cnt);
}


uint32_t pwm_tmr_get_period(pwm_tmr_typedef *pwm_tmr)
{
    return (uint32_t)(pwm_tmr->period);
}

void pwm_tmr_cmd(pwm_tmr_typedef *pwm_tmr, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        pwm_tmr->con |= PTMRxCON_EN;
    } else {
        pwm_tmr->con &= (uint32_t)(~PTMRxCON_EN);
    }
}

void pwm_tmr_ie_cmd(pwm_tmr_typedef *pwm_tmr, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        pwm_tmr->con |= PTMRxCON_IE;
    } else {
        pwm_tmr->con &= ~PTMRxCON_IE;
    }
}

void pwm_tmr_pic_config(isr_t isr, int pr, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        sys_irq_init(IRQn_PWM, pr, isr);
    } else {
        PICENCLR = BIT(IRQn_PWM);
    }
}

AT(.com_periph.tmr.get)
FLAG_STATE pwm_tmr_get_flag(pwm_tmr_typedef *pwm_tmr)
{
    if ((pwm_tmr->con & PTMRxCON_PND) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.tmr.clear)
void pwm_tmr_clear_flag(pwm_tmr_typedef *pwm_tmr)
{
    pwm_tmr->cpnd |= PTMRxCPND_PCLR;
}

void pwm_set_tmr_sel(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, PWM_TMR_SEL_TYPEDEF tmr_sel)
{
    uint32_t tmp_reg = pwm->con;

    tmp_reg = (tmp_reg & ~(PWMCON_CH_TMR_SEL_MASK << (ch * PWMCON_CH_TMR_SEL_LEN + PWMCON_CH_TMR_SEL_OFFSET))) |
                (tmr_sel << (ch * PWMCON_CH_TMR_SEL_LEN + PWMCON_CH_TMR_SEL_OFFSET));

    tmp_reg = (tmp_reg & ~(PWMCON_TMR_EN_MASK << (tmr_sel * PWMCON_TMR_EN_LEN + PWMCON_TMR_EN_OFFSET))) |
                (tmr_sel << (tmr_sel * PWMCON_TMR_EN_LEN + PWMCON_TMR_EN_OFFSET));

    pwm->con = tmp_reg;
}

void pwm_set_output_inv(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, FUNCTIONAL_STATE state)
{
    pwm->con = (pwm->con & ~(PWMCON_CH_INV_MASK << (ch * PWMCON_CH_INV_LEN + PWMCON_CH_INV_OFFSET))) |
                (state << (ch * PWMCON_CH_INV_LEN + PWMCON_CH_INV_OFFSET));
}

void pwm_cmd(pwm_typedef *pwm, PWM_CH_SEL_TYPEDEF ch, FUNCTIONAL_STATE state)
{
    pwm->con = (pwm->con & ~(PWMCON_CH_EN_MASK << (ch * PWMCON_CH_EN_LEN + PWMCON_CH_EN_OFFSET))) |
                (state << (ch * PWMCON_CH_EN_LEN + PWMCON_CH_EN_OFFSET));
}


void pwm_base_init(pwm_typedef *pwm, pwm_base_init_typedef *pwm_base_init_struct)
{
    pwm_set_tmr_sel(pwm, pwm_base_init_struct->ch, pwm_base_init_struct->tmr_sel);
    pwm_set_output_inv(pwm, pwm_base_init_struct->ch, pwm_base_init_struct->output_inv);
    // pwm_cmd(pwm, pwm_base_init_struct->ch, pwm_base_init_struct->en);
}

void pwm_dead_zone_init(pwm_dz_typedef *pwm_dz, pwm_dz_base_init_typedef *pwm_dz_init_struct)
{
    uint32_t tmp_reg = pwm_dz->con;

    tmp_reg &= ~(uint32_t)PWMDZCONx_MODE;
    tmp_reg |= pwm_dz_init_struct->mode;

    tmp_reg &= ~(uint32_t)PWMDZCONx_LEN_MASK;
    tmp_reg |= pwm_dz_init_struct->len << PWMDZCONx_LEN_LSB;

    pwm_dz->con = tmp_reg;
}

void pwm_dead_zone_cmd(pwm_dz_typedef *pwm_dz, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        pwm_dz->con |= PWMDZCONx_EN;
    } else {
        pwm_dz->con &= (uint32_t)(~PWMDZCONx_EN);
    }
}

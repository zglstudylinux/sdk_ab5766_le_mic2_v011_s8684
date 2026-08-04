/*
* @File name    : driver_sdadc.c
* @Author       : Bluetrum IOT Team
* @Date         : 2025-08-27
* @Description  : This file provides functions to manage the most functionalities
*                 of the SDADC peripheral.
*
* Copyright (c) by Bluetrum, All Rights Reserved.
*/
#include "driver_sdadc.h"
#include "driver_clk.h"
#include "api.h"

#define SDADC_DC_OPT        1
static uint32_t rmdc_sel_normal = RMDC_SEL_6BIT;

void sdadc_init(sdadc_typedef* sdadc, sdadc_init_typedef *sdadc_init_struct)
{
    uint32_t con = sdadc->con;
    uint32_t adc0_con = sdadc->adc0_con;
    uint32_t adc0_gain_con = sdadc->adc0_gain_con;
    uint32_t adc0_rm_dc_con = sdadc->adc0_rm_dc_con;
    uint32_t dc_con0 = sdadc->dc_con0;
#if !SDADC_DC_OPT
    uint32_t dc_con1 = sdadc->dc_con1;
    uint32_t dc_ang_con = sdadc->dc_ang_con;
#endif
    uint32_t dma_cpend = sdadc->dma_cpend;

    if (sdadc_init_struct->adc_mode == SDADC_MODE_MIC) {
        //dc con0
        dc_con0 &= ~SDADCDCCON0_DC_MODE;

        //adccon
        if (sdadc_init_struct->swclk_en != DISABLE) {
            con |= SDADCCON_SWCLK_EN;
        } else {
            con &= ~SDADCCON_SWCLK_EN;
        }

        con &= ~SDADCCON_SWCLK_PERI;
        con |= sdadc_init_struct->swclk_peri & SDADCCON_SWCLK_PERI;

        con &= ~SDADCCON_SWCLK_DUTY;
        con |= sdadc_init_struct->swclk_duty & SDADCCON_SWCLK_DUTY;

        //adc0con
        adc0_con &= ~SDADC0CON_FSOUT_SEL;
        adc0_con |= sdadc_init_struct->sample_rate & SDADC0CON_FSOUT_SEL;

        adc0_con &= ~SDADC0CON_BITS_SEL;
        adc0_con |= sdadc_init_struct->bits_sel & SDADC0CON_BITS_SEL;

        if (sdadc_init_struct->n6db_en != DISABLE) {
            adc0_con |= SDADC0CON_N6DB_EN;
        } else {
            adc0_con &= ~SDADC0CON_N6DB_EN;
        }

        if (sdadc_init_struct->inv_en != DISABLE) {
            adc0_con |= SDADC0CON_INV_EN;
        } else {
            adc0_con &= ~SDADC0CON_INV_EN;
        }

        //adc0gain
        adc0_gain_con &= ~SDADC0GAIN_ADC_GAIN;
        adc0_gain_con |= sdadc_init_struct->dig_gain & SDADC0GAIN_ADC_GAIN;

        //adc0rmdccon
        if (sdadc_init_struct->rmdc_en != DISABLE) {
            adc0_rm_dc_con |= SDADC0RMDCCON_RMDC_EN;
        } else {
            adc0_rm_dc_con &= ~SDADC0RMDCCON_RMDC_EN;
        }

        adc0_rm_dc_con &= ~SDADC0RMDCCON_RMDC_ORDER;
        adc0_rm_dc_con |= sdadc_init_struct->rmdc_order & SDADC0RMDCCON_RMDC_ORDER;

        //先配置FAST RMDC 保存设定值
        adc0_rm_dc_con &= ~SDADC0RMDCCON_RMDC_SEL;
        adc0_rm_dc_con |= RMDC_SEL_6BIT & SDADC0RMDCCON_RMDC_SEL;
        rmdc_sel_normal = sdadc_init_struct->rmdc_sel;

        if (sdadc_init_struct->sw_dc_en != DISABLE) {
            adc0_rm_dc_con |= SDADC0RMDCCON_SW_DC_EN;
        } else {
            adc0_rm_dc_con &= ~SDADC0RMDCCON_SW_DC_EN;
        }

        if (sdadc_init_struct->get_dc_en != DISABLE) {
            adc0_rm_dc_con |= SDADC0RMDCCON_GETDC_EN;
        } else {
            adc0_rm_dc_con &= ~SDADC0RMDCCON_GETDC_EN;
        }

        adc0_rm_dc_con &= ~SDADC0RMDCCON_SW_DC;
        adc0_rm_dc_con |= sdadc_init_struct->sw_dc_value & SDADC0RMDCCON_SW_DC;

        dma_cpend = 0x7;

        sdadc->dma_cpend = dma_cpend;
        sdadc->con = con;
        sdadc->adc0_con = adc0_con;
        sdadc->adc0_gain_con = adc0_gain_con;
        sdadc->adc0_rm_dc_con = adc0_rm_dc_con;
        sdadc->dc_con0 = dc_con0;
#if !SDADC_DC_OPT
    } else if (sdadc_init_struct->adc_mode == SDADC_MODE_DC) {
        //dc analog
        dc_ang_con = 0x6aa8000;

        //dc con0
        dc_con0 |= SDADCDCCON0_DC_MODE;

        if (sdadc_init_struct->dc_auto_ana_en != DISABLE) {
            dc_con0 |= SDADCDCCON0_ANA_CFG;
        } else {
            dc_con0 &= ~SDADCDCCON0_ANA_CFG;
        }

        dc_con0 &= ~SDADCDCCON0_SIGN_SEL;
        dc_con0 |= sdadc_init_struct->dc_sign_sel & SDADCDCCON0_SIGN_SEL;

        dc_con0 &= ~SDADCDCCON0_DONE_CNT;
        dc_con0 |= (sdadc_init_struct->dc_done_cnt << 10) & SDADCDCCON0_DONE_CNT;

        dc_con0 &= ~SDADCDCCON0_CH_EN;
        dc_con0 |= sdadc_init_struct->dc_ch & SDADCDCCON0_CH_EN;

        dc_con0 &= ~SDADCDCCON0_DC_FSOUT_SEL;
        dc_con0 |= sdadc_init_struct->dc_spr & SDADCDCCON0_DC_FSOUT_SEL;

        //dc con1
        dc_con1 &= ~SDADCDCCON1_THRESHOLD;
        dc_con1 |= (sdadc_init_struct->dc_cmp_thresh << 16) & SDADCDCCON1_THRESHOLD;

        if (sdadc_init_struct->dc_hvio_hold_en != DISABLE) {
            dc_con1 |= SDADCDCCON1_HVIO_HOLD;
        } else {
            dc_con1 &= ~SDADCDCCON1_HVIO_HOLD;
        }

        dc_con1 &= ~SDADCDCCON1_CMP_DIR;
        dc_con1 |= sdadc_init_struct->dc_cmp_dir & SDADCDCCON1_CMP_DIR;

        dc_con1 &= ~SDADCDCCON1_CH_CMP_EN;
        dc_con1 |= sdadc_init_struct->dc_cmp_ch & SDADCDCCON1_CH_CMP_EN;
        if (sdadc_init_struct->dc_cmp_op_pin & DC_CMP_OP_PIN_PA0) {
            GPIOADE  |= BIT(0);
            GPIOADIR &= ~BIT(0);
            GPIOADRV |= BIT(18);
            GPIOASET  = BIT(0);
            if (sdadc_init_struct->dc_cmp_op_volt == DC_CMP_OP_VOLT_VDDIO) {
                GPIOADRV &= ~BIT(16);
            } else {
                GPIOADRV |= BIT(16);
            }
        }
        if (sdadc_init_struct->dc_cmp_op_pin & DC_CMP_OP_PIN_PA1) {
            GPIOADE  |= BIT(1);
            GPIOADIR &= ~BIT(1);
            GPIOADRV |= BIT(19);
            GPIOASET  = BIT(1);
            if (sdadc_init_struct->dc_cmp_op_volt == DC_CMP_OP_VOLT_VDDIO) {
                GPIOADRV &= ~BIT(17);
            } else {
                GPIOADRV |= BIT(17);
            }
        }

        dma_cpend = 0x7;

        sdadc->dma_cpend = dma_cpend;
        sdadc->dc_con0 = dc_con0;
        sdadc->dc_con1 = dc_con1;
        sdadc->dc_ang_con = dc_ang_con;
#endif
    }
}

void sdadc_deinit(sdadc_typedef* sdadc)
{
    sdadc->adc0_dma_con &= ~SDADC0DMACON_DMA_EN;
    sdadc->con &= ~(SDADCCON_TOTAL_EN | \
                    SDADCCON_ADC0_EN | \
                    SDADCCON_SWCLK_EN);
    sdadc_ana_deinit();
    PICEN &= ~BIT(IRQn_SDADC);
    clk_gate0_cmd(CLK_GATE0_SDADC, CLK_DIS);
}

AT(.com_text.mic_emit.proc.sdadc)
void sdadc0_cmd(sdadc_typedef* sdadc, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        sdadc->con |= SDADCCON_ADC0_EN;
    } else {
        sdadc->con &= ~SDADCCON_ADC0_EN;
    }
}

AT(.com_text.mic_emit.proc.sdadc)
void sdadc_rmdc_restore(void)
{
    SDADC0RMDCCON &= ~SDADC0RMDCCON_RMDC_SEL;
    SDADC0RMDCCON |= rmdc_sel_normal & SDADC0RMDCCON_RMDC_SEL;
}

AT(.com_text.mic_emit.proc.sdadc)
void sdadc_total_cmd(sdadc_typedef* sdadc, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        sdadc->con |= SDADCCON_TOTAL_EN;
    } else {
        sdadc->con &= ~SDADCCON_TOTAL_EN;
    }
}

void sdadc_dma_cmd(sdadc_typedef* sdadc, u32 addr, u16 len, DMA_DATA_MODE_TYPEDEF data_mode, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        /* Config DMA address and size */
        sdadc->adc0_dma_addr = addr;
        sdadc->adc0_dma_size = len;
        sdadc->adc0_dma_con &= ~SDADC0DMACON_DMA_DATA_MODE;
        sdadc->adc0_dma_con |= data_mode & SDADC0DMACON_DMA_DATA_MODE;
        sdadc->adc0_dma_con |= SDADC0DMACON_DMA_EN;
    } else {
        sdadc->adc0_dma_con &= ~SDADC0DMACON_DMA_EN;
    }
}

void sdadc_pic_config(sdadc_typedef* sdadc, isr_t isr, int pr, SDADC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_pending_bit = 0;
    u32 all_flag_mask = (SDADC0DMACON_DMA_SAMPLE_IE | SDADC0DMACON_DMA_HALF_IE | SDADC0DMACON_DMA_ALL_IE);

    if (flag_type == 0) {
        return;
    }

    if (flag_type & SDADC_FLAG_SINGLE_SAMPLE) {
        flag_pending_bit |= SDADC0DMACON_DMA_SAMPLE_IE;
    }

    if (flag_type & SDADC_FLAG_DMA_HALF_DONE) {
        flag_pending_bit |= SDADC0DMACON_DMA_HALF_IE;
    }

    if (flag_type & SDADC_FLAG_DMA_ALL_DONE) {
        flag_pending_bit |= SDADC0DMACON_DMA_ALL_IE;
    }

    if (state != DISABLE) {
        sdadc->adc0_dma_con |= flag_pending_bit;
        sys_irq_init(IRQn_SDADC, pr, isr);
    } else {
        sdadc->adc0_dma_con &= ~flag_pending_bit;
        if (flag_pending_bit & all_flag_mask) {
            PICEN &= ~BIT(IRQn_SDADC);
        }
    }
}

AT(.com_periph.sdadc.flag)
FLAG_STATE sdadc_get_flag(sdadc_typedef* sdadc, SDADC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & SDADC_FLAG_DMA_ALL_DONE) {
        flag_pending_bit |= SDADCDMAFLAG_DMA_ALL_DONE;
    }

    if (flag_type & SDADC_FLAG_DMA_HALF_DONE) {
        flag_pending_bit |= SDADCDMAFLAG_DMA_HALF_DONE;
    }

    if (flag_type & SDADC_FLAG_SINGLE_SAMPLE) {
        flag_pending_bit |= SDADCDMAFLAG_DMA_SAMPLE_DONE;
    }

    if ((sdadc->dma_pend & flag_pending_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.sdadc.flag)
void sdadc_clear_flag(sdadc_typedef* sdadc, SDADC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending_bit = 0;

    if (flag_type & SDADC_FLAG_DMA_ALL_DONE) {
        flag_pending_bit |= SDADCDMACLR_DMA_ALL_DONE;
    }

    if (flag_type & SDADC_FLAG_DMA_HALF_DONE) {
        flag_pending_bit |= SDADCDMACLR_DMA_HALF_DONE;
    }

    if (flag_type & SDADC_FLAG_SINGLE_SAMPLE) {
        flag_pending_bit |= SDADCDMACLR_DMA_SAMPLE_DONE;
    }

    sdadc->dma_cpend |= flag_pending_bit;
}

void sdadc_digital_gain_set(sdadc_typedef* sdadc, uint16_t dig_gain)
{
    uint32_t gain_con = sdadc->adc0_gain_con;
    gain_con &= ~SDADC0GAIN_ADC_GAIN;
    gain_con |= dig_gain & SDADC0GAIN_ADC_GAIN;
    sdadc->adc0_gain_con = gain_con;
}

#if !SDADC_DC_OPT
void sdadc_dc_pic_config(sdadc_typedef* sdadc, isr_t isr, int pr, SDADC_DC_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state)
{
    u32 flag_pending_bit = 0;
    u32 all_flag_mask = (SDADCDCCON1_CMP_IE | SDADCDCCON1_SMP_IE);

    if (flag_type == 0) {
        return;
    }

    if (flag_type & SDADC_DC_FLAG_SMP_DONE) {
        flag_pending_bit |= SDADCDCCON1_SMP_IE;
    }

    if (flag_type & SDADC_DC_FLAG_CMP_DONE) {
        flag_pending_bit |= SDADCDCCON1_CMP_IE;
    }

    if (state != DISABLE) {
        sdadc->dc_con1 |= flag_pending_bit;
        sys_irq_init(IRQn_SDADC, pr, isr);
    } else {
        sdadc->dc_con1 &= ~flag_pending_bit;
        if (flag_pending_bit & all_flag_mask) {
            PICEN &= ~BIT(IRQn_SDADC);
        }
    }
}

AT(.com_periph.sdadc.flag)
FLAG_STATE sdadc_dc_get_flag(sdadc_typedef* sdadc, SDADC_DC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending0_bit = 0;
    u32 flag_pending1_bit = 0;

    if (flag_type & SDADC_DC_FLAG_OUPUT_DONE) {
        flag_pending0_bit |= SDADCDCCON0_OUTPUT_FLAG;
    }

    if (flag_type & SDADC_DC_FLAG_SMP_DONE) {
        flag_pending1_bit |= SDADCDCCON1_SMP_PND;
    }

    if (flag_type & SDADC_DC_FLAG_CMP_DONE) {
        flag_pending1_bit |= SDADCDCCON1_CMP_PND;
    }

    if (flag_pending0_bit && (sdadc->dc_con0 & flag_pending0_bit) != RESET) {
        return SET;
    } else if (flag_pending1_bit && (sdadc->dc_con1 & flag_pending1_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.sdadc.flag)
void sdadc_dc_clear_flag(sdadc_typedef* sdadc, SDADC_DC_FLAG_TYPEDEF flag_type)
{
    u32 flag_pending0_bit = 0;
    u32 flag_pending1_bit = 0;

    if (flag_type & SDADC_DC_FLAG_OUPUT_DONE) {
        flag_pending0_bit |= SDADCDCCON0_OUTPUT_FLAG;
    }

    if (flag_type & SDADC_DC_FLAG_SMP_DONE) {
        flag_pending1_bit |= SDADCDCCON1_SMP_PND_CLR;
    }

    if (flag_type & SDADC_DC_FLAG_CMP_DONE) {
        flag_pending1_bit |= SDADCDCCON1_CMP_PND_CLR;
    }

    if (flag_pending0_bit) {
        sdadc->dc_con0 |= flag_pending0_bit;
    }

    if (flag_pending1_bit) {
        sdadc->dc_con1 |= flag_pending1_bit;
    }
}

AT(.com_periph.sdadc.dc_kick)
void sdadc_dc_kick(sdadc_typedef* sdadc, FUNCTIONAL_STATE auto_en, FUNCTIONAL_STATE kick_en)
{
    if (auto_en != DISABLE) {
        sdadc->dc_con0 |= SDADCDCCON0_AUTO_KICK;
    } else {
        sdadc->dc_con0 &= ~SDADCDCCON0_AUTO_KICK;
    }

    if (kick_en != DISABLE) {
        sdadc->dc_con1 |= SDADCDCCON1_DC_MANUAL_KICK;
    } else {
        sdadc->dc_con1 &= ~SDADCDCCON1_DC_MANUAL_KICK;
    }
}

AT(.com_periph.sdadc.hold_set)
void sdadc_dc_hvio_hold_set(sdadc_typedef* sdadc, FUNCTIONAL_STATE hold_en)
{
    if (hold_en != DISABLE) {
        sdadc->dc_con1 |= SDADCDCCON1_HVIO_HOLD;
    } else {
        sdadc->dc_con1 &= ~SDADCDCCON1_HVIO_HOLD;
    }
}

AT(.com_periph.sdadc.hold_get)
FLAG_STATE sdadc_dc_hvio_hold_flag_get(sdadc_typedef* sdadc)
{
    if (sdadc->dc_con1 & SDADCDCCON1_HVIO_HOLD) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.sdadc.dc_data)
uint16_t sdadc_dc_ch_data_get(sdadc_typedef* sdadc, DC_CH_SEL_TYPEDEF channel)
{
    uint16_t data = 0;
    switch (channel)
    {
        case DC_SAMP_CH0_VBAT_EN:
            data = sdadc->dc_data0;
            break;
        case DC_SAMP_CH1_PA2_EN:
            data = sdadc->dc_data1;
            break;
        case DC_SAMP_CH2_PA3_EN:
            data = sdadc->dc_data2;
            break;

        default:
            break;
    }
    return data;
}

AT(.com_periph.sdadc.dc_data)
uint16_t sdadc_dc_ch_volt_mv_get(sdadc_typedef* sdadc, DC_CH_SEL_TYPEDEF channel)
{
    uint16_t ad_val = sdadc_dc_ch_data_get(sdadc, channel);

    uint16_t volt_mv = (ad_val*1000/32768-1000)*248*292/24000;

    if (channel == DC_SAMP_CH0_VBAT_EN) {
        volt_mv *= 2;
    }

    return volt_mv;
}
#endif

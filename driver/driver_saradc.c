/*
 * @File name    : driver_saradc.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-02
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the SARADC peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_saradc.h"


void saradc_init(saradc_base_init_typedef* saradc_base_init_struct)
{
    saradc_typedef * saradc = SARADC_REG;
    u16 baud_reg;
    u32 saradc_clk;

    saradc_clk = clk_saradc_clk_get(CLK_VALUE_MODE_FREQ);

    /* Configure Baud Register */
    baud_reg = (saradc_clk / (2 * saradc_base_init_struct->baud) - 1) & 0x3ff;
    saradc->baud = baud_reg;

    /* Configure SARADC Mode */
    saradc->con &= ~SADCCON_ADCMODE;
    saradc->con |=  saradc_base_init_struct->mode & SADCCON_ADCMODE;

    /* Configure SARADC auto enable analog enable bit */
    saradc->con &= ~SADCCON_ADCAEN;
    saradc->con |= saradc_base_init_struct->auto_analog & SADCCON_ADCAEN;

    /* Configure SARADC auto enable analog IO enable bit */
    saradc->con &= ~SADCCON_ADCANGIOEN;
    saradc->con |= saradc_base_init_struct->auto_analog_io & SADCCON_ADCANGIOEN;
}

void saradc_channel_init(saradc_channel_init_typedef *saradc_channel_init_struct)
{
    saradc_typedef * saradc = SARADC_REG;

    u8 i = 0;

    /* Battery channel config */
    if (saradc_channel_init_struct->channel & ADC_CHANNEL_VBAT) {
        PWRCON0  |= (uint32_t)(1 << 10);        // VBATDIV2 ADC enable
    }

    /* Bandgap channel config */
    if (saradc_channel_init_struct->channel & ADC_CHANNEL_BG) {
        PWRCON1  |= (uint32_t)(1 << 21);       //VBG 1.5V
        delay_us(100);
        PWRCON0 |= (uint32_t)(1 << 22);
        delay_us(100);
    }

    /* PWRKEY channel config */
    if (saradc_channel_init_struct->channel & ADC_CHANNEL_WK0) {
        RTCCON1 |= (uint32_t)(1 << 5);          // WK0 output ADC enable
        if (RTCCON1 & BIT(1)) {                 // Sure this bit is enable,
            RTCCON11 |= BIT(4);                 // and recommends it is enabled normally.
        }
    }

    /* VDDRTC channel config */
    if (saradc_channel_init_struct->channel & ADC_CHANNEL_VRTC) {
        RTCCON1 |= (uint32_t)(1 << 7);          // VRTC output ADC enable
    }

    /* VUSB channel config */
    if (saradc_channel_init_struct->channel & ADC_CHANNEL_VUSB) {
        RTCCON8 |= (uint32_t)(1 << 15);         // VUSB output ADC enable
        PWRCON2 &= ~(uint32_t)(0x03 << 5);      // TSEN disable
        saradc->con &= ~(0x7 << 21);
    /* TSEN channel config */
    } else if (saradc_channel_init_struct->channel & ADC_CHANNEL_TSEN) {
        RTCCON8 &= ~(uint32_t)(1 << 15);
        PWRCON2 |= (uint32_t)(0x03 << 5);
        saradc_channel_init_struct->channel &= ~ADC_CHANNEL_TSEN;
        saradc_channel_init_struct->channel |= ADC_CHANNEL_VUSB;
        saradc->con &= ~(0x7 << 21);
        saradc->con |= (0x2 << 21);
    } else if (saradc_channel_init_struct->channel & ADC_CHANNEL_VC2SAR) {
        saradc_channel_init_struct->channel |= ADC_CHANNEL_VUSB;
        saradc->con &= ~(0x7 << 21);
        saradc->con |= (0x3 << 21);
    }

    /* Configure each channel */
    for (i = 0; i < SARADC_MAX_CH; i++) {
        if (saradc_channel_init_struct->channel & (0x01 << i)) {
            saradc->cst &= ~(SADCST_CH_ST_MASK << (i * SADCST_CH_ST_LEN + SADCST_CH_ST_OFFSET));
            saradc->cst |= (saradc_channel_init_struct->setup_time) << (i * SADCST_CH_ST_LEN + SADCST_CH_ST_OFFSET);

            saradc->con &= ~(0x01 << i);
            saradc->con |= (saradc_channel_init_struct->pullup_en & 0x01) << i;
        }
    }
}

uint8_t saradc_deinit(void)
{
    saradc_typedef * saradc = SARADC_REG;

    uint8_t saradc_analog_bits = 0;

    /* Battery channel config */
    if (PWRCON0 & (uint32_t)(1 << 10)) {
        PWRCON0 &= ~((uint32_t)(1 << 10));
        saradc_analog_bits |= SARADC_ANALOG_BIT_VBAT;
    }

    /* Bandgap channel config */
    if (PWRCON0 & (uint32_t)(1 << 22)) {
        PWRCON0 &= ~((uint32_t)(1 << 22));
        saradc_analog_bits |= SARADC_ANALOG_BIT_BG;
    }

    /* PWRKEY channel config */
    if (RTCCON1 & (uint32_t)(1 << 5)) {
        RTCCON1 &= ~((uint32_t)(1 << 5));
        saradc_analog_bits |= SARADC_ANALOG_BIT_WK0;
    }

    /* VDDRTC channel config */
    if (RTCCON1 & (uint32_t)(1 << 7)) {
        RTCCON1 &= ~((uint32_t)(1 << 7));
        saradc_analog_bits |= SARADC_ANALOG_BIT_VRTC;
    }

    /* VUSB channel config */
    if (RTCCON8 & (uint32_t)(1 << 15)) {
        RTCCON8 &= ~((uint32_t)(1 << 15));
        saradc_analog_bits |= SARADC_ANALOG_BIT_VUSB;
    }

    /* TSEN channel config */
    if (PWRCON2 & (0x03 << 5)) {
        PWRCON2 &= ~(0x03 << 5);
        saradc_analog_bits |= ADC_CHANNEL_TSEN;
    }

    /* Clear saradc register config */
    saradc->con = 0;
    saradc->ch = 0;

    /* Disable SARADC clock */
    clk_gate0_cmd(CLK_GATE0_SARADC, CLK_DIS);

    return saradc_analog_bits;
}

void saradc_cmd(FUNCTIONAL_STATE state)
{
    saradc_typedef * saradc = SARADC_REG;

    if(state) {
        saradc->con |= SADCCON_ADCEN;
    } else {
        saradc->con &= (uint32_t)~SADCCON_ADCEN;
    }
}


AT(.com_periph.saradc.kick)
void saradc_kick_start(uint32_t adc_ch)
{
    saradc_typedef * saradc = SARADC_REG;

    if (!adc_ch) {
        return;
    }

    if ((adc_ch & ADC_CHANNEL_TSEN) || (adc_ch & ADC_CHANNEL_VC2SAR)) {
//        adc_ch &= ~(ADC_CHANNEL_TSEN | ADC_CHANNEL_VC2SAR);
        adc_ch |= ADC_CHANNEL_VUSB;
    }

    do {
        GLOBAL_INT_DISABLE();
        if((saradc->ch & 0xffff) == 0) {
            saradc->ch = (adc_ch & 0xffff);
            GLOBAL_INT_RESTORE();
            break;
        } else {
            GLOBAL_INT_RESTORE();
        }
    } while(1);
}


void saradc_set_baud(uint32_t baud)
{
    saradc_typedef * saradc = SARADC_REG;

    u16 baud_reg;
    u32 saradc_clk;

    saradc_clk = clk_saradc_clk_get(CLK_VALUE_MODE_FREQ);

    /* Configure Baud Register */
    baud_reg = (saradc_clk / (2 * baud) - 1) & 0x3ff;
    saradc->baud = baud_reg;
}


void saradc_pic_config(isr_t isr, int pr, SARADC_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state)
{
    saradc_typedef * saradc = SARADC_REG;

    uint32_t interrupt_bit = 0;

    /* Get interrupt ctrl bit */
    if (interrupt_type & SARADC_IT_FINISHED) {
        interrupt_bit |= SADCCON_ADCIE;
    }

    if(state != DISABLE) {
        saradc->con |= interrupt_bit;
        sys_irq_init(IRQn_SARADC, pr, isr);
    } else {
        saradc->con &= ~interrupt_bit;
        PICEN &= ~BIT(IRQn_SARADC);
    }
}


AT(.com_periph.saradc.get)
FLAG_STATE saradc_get_flag(SARADC_IT_TYPEDEF interrupt_type)
{
    saradc_typedef * saradc = SARADC_REG;

    uint32_t flag_bit = 0;

    if (interrupt_type & SARADC_IT_FINISHED) {
        flag_bit |= SADCCH_ADCPND;
    }

    if ((saradc->ch & flag_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}


AT(.com_periph.saradc.get_channel)
FLAG_STATE saradc_get_channel_flag(uint32_t adc_ch)
{
    saradc_typedef * saradc = SARADC_REG;

    if (adc_ch & ADC_CHANNEL_TSEN) {
        adc_ch &= ~ADC_CHANNEL_TSEN;
        adc_ch |= ADC_CHANNEL_VUSB;
    }

    if (((saradc->ch) & adc_ch) != RESET) {
        return RESET;
    } else {
        return SET;
    }
}


AT(.com_periph.saradc.clear)
void saradc_clear_flag(SARADC_IT_TYPEDEF interrupt_type)
{
    saradc_typedef * saradc = SARADC_REG;

    uint32_t flag_bit = 0;

    if (interrupt_type & SARADC_IT_FINISHED) {
        flag_bit |= SADCCH_ADCPND;
    }

    saradc->ch |= flag_bit;
}


AT(.com_periph.saradc.get_data)
uint32_t saradc_get_data(uint32_t adc_chx)
{
    saradc_typedef * saradc = SARADC_REG;

    u8 i = 0;

    while (i < 18) {
        if (adc_chx & (0x01 << i)) {
            break;
        }
        i++;
    }

    if ((0x01 << i) == ADC_CHANNEL_VBAT) {
        return saradc->dat[i] * pmu_get_vbatdet() / 1000;
    } else if ((0x01 << i) == ADC_CHANNEL_VUSB) {
        return saradc->dat[i] * 3;
    } else if ((0x01 << i) == ADC_CHANNEL_TSEN) {
        return saradc->dat[15];
    } else if ((0x01 << i) == ADC_CHANNEL_VC2SAR) {
        return saradc->dat[15];
    } else if (i < SARADC_MAX_CH) {
        return saradc->dat[i];
    } else {
        return 0;
    }
}

void saradc_get_mic_dc_init(void)
{
    saradc_channel_init_typedef saradc_channel_init_struct;

    saradc_channel_init_struct.channel = ADC_CHANNEL_VC2SAR | ADC_CHANNEL_BG;
    saradc_channel_init_struct.pullup_en = SARADC_PULLUP_DIS;
    saradc_channel_init_struct.setup_time = SARADC_ST_8_CLK;
    saradc_channel_init(&saradc_channel_init_struct);

    saradc_kick_start(ADC_CHANNEL_VC2SAR | ADC_CHANNEL_BG);
}

void saradc_get_mic_dc_deinit(void)
{
    saradc_typedef * saradc = SARADC_REG;
    saradc->con &= ~(0x3 << 21);
}

AT(.com_text.saradc.get_mic_dc)
uint32_t saradc_get_mic_dc_volt(void)
{
    uint32_t dc_volt = 0;
    uint32_t vc2_sar, bg_sar;
    saradc_kick_start(ADC_CHANNEL_VC2SAR | ADC_CHANNEL_BG);
    while (SARADC_REG->ch & 0xffff);
    vc2_sar = saradc_get_data(ADC_CHANNEL_VC2SAR);
    bg_sar = saradc_get_data(ADC_CHANNEL_BG);
//    if (bg_sar == 0) { //避免除0现象
//        WDT_RST();
//    }
    dc_volt = vc2_sar * ADC_VOLTAGE_VBG / bg_sar;

    return dc_volt;
}

/*
* @File name    : driver_sdadc.c
* @Author       : Bluetrum IOT Team
* @Date         : 2025-09-11
* @Description  : This file provides functions to manage the most functionalities
*                 of the SDDAC peripheral.
*
* Copyright (c) by Bluetrum, All Rights Reserved.
*/
#include "include.h"
#include "driver_sddac.h"
#include "driver_clk.h"
#include "api_sddac.h"

///优化dac1代码
#define SDDAC1_OPT_EN         1

void sddac_clock_init(void)
{
    clk_gate0_cmd(CLK_GATE0_DAC, CLK_EN);
#if !SDDAC1_OPT_EN
    clk_gate1_cmd(CLK_GATE1_DAC_SRC, CLK_EN);
#endif
    clk_sdadda_clk_set(CLK_SDADDA_CLK48_A);
}

void sddac_aubuf_init(sddac_cb_typedef *sddac_cb)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb->aubuf;

    if (aubuf->au0_en == 1) {
        AUBUF0CON |= BIT(24);                                                   //aubuf0 mono
        AUBUF0CON |= BIT(18);                                                   //DAC high sample rate count enable
        AUBUF0CON = (aubuf->au1_start_sel  << 9) | (AUBUF0CON & ~(0x3 << 9));   //au0 start sel
        AUBUF0CON = (aubuf->au1_start_size << 6) | (AUBUF0CON & ~(0x3 << 6));   //au0 start size
        AUBUF0CON |= BIT(0);                                                    //Reset audio Buffer
        AUBUF0SIZE = (aubuf->au0_size - 1);                                     //Configure audio buffer size
        AUBUF0SIZE = (aubuf->au0_thr << 16) | (AUBUF0SIZE & ~(0xffff << 16));   //Audio Buffer Threshold
        AUBUF0STARTADDR = DMA_ADR(aubuf->au0_ptr);                              //Configure audio buffer start address
    }
#if !SDDAC1_OPT_EN
    if (aubuf->au1_en == 1) {
        AUBUF1CON |= BIT(24);                                                     //aubuf1 mono
        AUBUF1CON |= BIT(18);                                                     //DAC high sample rate count enable
        AUBUF1CON = (aubuf->au1_start_sel  << 9) | (AUBUF1CON & ~(0x3 << 9));     //au1 start sel
        AUBUF1CON = (aubuf->au1_start_size << 6) | (AUBUF1CON & ~(0x3 << 6));     //au1 start size
        AUBUF1CON |= BIT(0);                                                      //Reset audio Buffer
        AUBUF1SIZE = (aubuf->au1_size - 1);                                       //Configure audio buffer size
        AUBUF1SIZE = (aubuf->au1_thr << 16) | (AUBUF1SIZE & ~(0xffff << 16));     //Audio Buffer Threshold
        AUBUF1STARTADDR = DMA_ADR(aubuf->au1_ptr);                                //Configure audio buffer start address
    }
#endif
}

void sddac_dig_power_on(sddac_cb_typedef *sddac_cb)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb->aubuf;
    sddac_dig_typedef *dig = &sddac_cb->dig;

    if (aubuf->au0_en == 1) {
        if (aubuf->au0_pcm_bits == SDDAC_PCM_24BITS) {
            DACDIGCON1 &= ~BIT(29);          //aubuf0 24bits
        } else {
            DACDIGCON1 |= BIT(29);           //aubuf0 16bits
        }
        DACDIGCON0 = (dig->src0_bypass_en << 3) | (DACDIGCON0 & ~(0x1 << 3));
        if (dig->src0_mute == 1) {
            SRC0VOLCON = 0;
        } else {
            SRC0VOLCON = dig->src0_vol;
        }
        SRC0VOLCON |= BIT(30);
        AU0LMIXCOEF = dig->au0mixcoef;
        DACSRC0PHAI = (dig->src0_spr << 15) / 1000 | 1<<31;
    }
#if !SDDAC1_OPT_EN
    if (aubuf->au1_en == 1) {
        if (aubuf->au1_pcm_bits == SDDAC_PCM_24BITS) {
            DACDIGCON1 &= ~BIT(30);          //aubuf0 24bits
        } else {
            DACDIGCON1 |= BIT(30);           //aubuf0 16bits
        }
        DACDIGCON0 = (dig->src1_bypass_en << 4) | (DACDIGCON0 & ~(0x1 << 4));
        if (dig->src1_mute == 1) {
            SRC1VOLCON = 0;
        } else {
            SRC1VOLCON = dig->src1_vol;
        }
        SRC1VOLCON |= BIT(30);
        AU1LMIXCOEF = dig->au1mixcoef;
        DACSRC1PHAI = (dig->src1_spr << 15) / 1000 | 1<<31;
        DACDIGCON0 |= BIT(1);  //src1 en
    } else {
        DACDIGCON0 &= ~BIT(1);  //src1 dis
    }
#endif
    DACDIGCON0 = (dig->mix_en << 25) | (DACDIGCON0 & ~(1 << 25));
    DACDIGCON0 = (dig->mix_div2_en << 24) | (DACDIGCON0 & ~(1 << 24));

    DACVOLCON = dig->total_vol;
    DACVOLCON |= BIT(30);

    DACDIGCON0 = (dig->dac_en << 0) | (DACDIGCON0 & ~(0x1 << 0));  //dac en
}

void sddac_deinit(void)
{
    sddac_cb.init_flag = 0;
    AUBUF0DMACON  &= ~(BIT(5) | BIT(0));  //au0 dma ie、dis en
    AUBUF1DMACON  &= ~(BIT(5) | BIT(0));  //au1 dma ie、dis en
    DACDIGCON0    &=  ~(0x1 << 0);          //dac dis en
    clk_gate0_cmd(CLK_GATE0_DAC, CLK_DIS);
    clk_gate1_cmd(CLK_GATE1_DAC_SRC, CLK_DIS);
}

void sddac_aubuf_size_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t size, uint16_t thr)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        aubuf->au0_size = size;
        aubuf->au0_thr  = thr;
        AUBUF0CON |= BIT(24);                                 //aubuf0 mono
        AUBUF0CON |= BIT(18);                                 //DAC high sample rate count enable
        AUBUF0CON |= BIT(0);                                  //Reset audio Buffer
        AUBUF0SIZE = (size - 1);                              //Configure audio buffer size
        AUBUF0SIZE |= thr << 16;                              //Audio Buffer Threshold
        AUBUF0STARTADDR = DMA_ADR(aubuf->au0_ptr);            //Configure audio buffer start address
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        aubuf->au1_size = size;
        aubuf->au1_thr  = thr;
        AUBUF1CON |= BIT(24);                                 //aubuf1 mono
        AUBUF1CON |= BIT(18);                                 //DAC high sample rate count enable
        AUBUF1CON |= BIT(0);                                  //Reset audio Buffer
        AUBUF1SIZE = (size - 1);                              //Configure audio buffer size
        AUBUF1SIZE |= thr << 16;                              //Audio Buffer Threshold
        AUBUF1STARTADDR = DMA_ADR(aubuf->au1_ptr);            //Configure audio buffer start address
#endif
    }
}

void sddac_aubuf_put_samples(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf, uint32_t samples)
{
    int16_t *ptr = (int16_t *)buf;
    while (samples--) {
        if (idx == SDDAC_AUBUF0) {
            if (sddac_aubuf_is_full(idx)) {
                AUBUF0CON |= BIT(1) | BIT(4);     //Audio Buffer Pend Clear, interrupt enable
                os_sem_au0_full_take(UINT_MAX);
            }

            if (sddac_pcm_is_24bits(0)) {
                AUBUF0DATA = *buf++;
            } else {
                AUBUF0DATA = *ptr++;
            }
#if !SDDAC1_OPT_EN
        } else if (idx == SDDAC_AUBUF1) {
            if (sddac_aubuf_is_full(idx)) {
                AUBUF1CON |= BIT(1) | BIT(4);     //Audio Buffer Pend Clear, interrupt enable
                os_sem_au1_full_take(UINT_MAX);
            }

            if (sddac_pcm_is_24bits(1)) {
                AUBUF1DATA = *buf++;
            } else {
                AUBUF1DATA = *ptr++;
            }
#endif
        }
    }
}

AT(.com_text.sddac)
void sddac_aubuf_dma_kick(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf_addr, uint32_t samples)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        if (sddac_aubuf_is_full(SDDAC_AUBUF0)) {
            os_sem_au0_full_take(UINT_MAX);
        }

        if (aubuf->au0_pcm_bits == SDDAC_PCM_24BITS) {
            AUBUF0DMACON = (0x0 << 3) | (AUBUF0DMACON & ~(0x3<<3));  //24bits
            AUBUF0DMACON = (((1*samples-1)*4) << 16) | (AUBUF0DMACON & ~(0xffff<<16));
        } else {
            AUBUF0DMACON = (0x1 << 3) | (AUBUF0DMACON & ~(0x3<<3));  //16bits
            AUBUF0DMACON = (((1*samples-1)*2) << 16) | (AUBUF0DMACON & ~(0xffff<<16));
        }
        AUBUF0DMAADR = (uint32_t)buf_addr;
        AUBUF0DMACON  |= BIT(5);  //dma ie
        AUBUF0DMACON  |= BIT(0);  //dma en
        AUBUF0DMAKICK |= BIT(0);  //dma kick en

        if (!(AUBUF0CON & BIT(4))) {
            AUBUF0CON |= BIT(1);         //Clear AUBUF0 THR Flag
            AUBUF0CON |= BIT(4);         //Audio buffer interrupt enable
        }
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        if (sddac_aubuf_is_full(SDDAC_AUBUF1)) {
            os_sem_au1_full_take(UINT_MAX);
        }

        if (aubuf->au1_pcm_bits == SDDAC_PCM_24BITS) {
            AUBUF1DMACON = (0x0 << 3) | (AUBUF1DMACON & ~(0x3<<3));  //24bits
            AUBUF1DMACON = (((1*samples-1)*4) << 16) | (AUBUF1DMACON & ~(0xffff<<16));
        } else {
            AUBUF1DMACON = (0x1 << 3) | (AUBUF1DMACON & ~(0x3<<3));  //16bits
            AUBUF1DMACON = (((1*samples-1)*2) << 16) | (AUBUF1DMACON & ~(0xffff<<16));
        }
        AUBUF1DMAADR = (uint32_t)buf_addr;
        AUBUF1DMACON  |= BIT(5);  //dma ie
        AUBUF1DMACON  |= BIT(0);  //dma en
        AUBUF1DMAKICK |= BIT(0);  //dma kick en

        if (!(AUBUF1CON & BIT(4))) {
            AUBUF1CON |= BIT(1);         //Clear AUBUF1 THR Flag
            AUBUF1CON |= BIT(4);         //Audio buffer interrupt enable
        }
#endif
    }
}

AT(.com_text.sddac)
void sddac_aubuf_dma_kick_w4_done(SDDAC_AUBUF_TYPEDEF idx)
{
    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        if (AUBUF0DMACON & BIT(5)) {
            os_sem_au0_dma_take(UINT_MAX);
        } else {
            while (!(AUBUF0DMACON & BIT(8)));       //wait finish
            AUBUF0DMAKICK = BIT(8);                 //Clear GPDMA Done Flag
        }
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        if (AUBUF1DMACON & BIT(5)) {
            os_sem_au1_dma_take(UINT_MAX);
        } else {
            while (!(AUBUF1DMACON & BIT(8)));       //wait finish
            AUBUF1DMAKICK = BIT(8);                 //Clear GPDMA Done Flag
        }
#endif
    }
}

void sddac_aubuf_pic_config(isr_t isr, int pr)
{
    sys_irq_init(IRQn_KEYSCAN_AUBUFx, pr, isr);
}

void sddac_aubuf_dma_pic_config(isr_t isr, int pr)
{
    sys_irq_init(IRQn_DAC_DMA, pr, isr);
}

AT(.com_text.sddac)
bool sddac_aubuf_is_full(SDDAC_AUBUF_TYPEDEF idx)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (!sddac_aubuf_en_check(idx)) {
        return 0;
    }

    switch (idx) {
        case SDDAC_AUBUF0:
            if (AUBUF0CON & BIT(8) || aubuf->au0_full_flag) {
                aubuf->au0_full_flag = 1;
                return 1;
            }
            break;
#if !SDDAC1_OPT_EN
        case SDDAC_AUBUF1:
            if (AUBUF1CON & BIT(8) || aubuf->au1_full_flag) {
                aubuf->au1_full_flag = 1;
                return 1;
            }
            break;
#endif
        default:
            break;
    }
    return 0;
}

AT(.com_text.sddac)
bool sddac_aubuf_get_flag(SDDAC_AUBUF_TYPEDEF idx)
{
    if (!sddac_aubuf_en_check(idx)) {
        return 0;
    }

    switch (idx) {
        case SDDAC_AUBUF0:
            if (AUBUF0CON & BIT(5)) {
                return 1;
            }
            break;
#if !SDDAC1_OPT_EN
        case SDDAC_AUBUF1:
            if (AUBUF1CON & BIT(5)) {
                return 1;
            }
            break;
#endif
        default:
            break;
    }
    return 0;
}

AT(.com_text.sddac)
void sddac_aubuf_clear_flag(SDDAC_AUBUF_TYPEDEF idx)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    switch (idx) {
        case SDDAC_AUBUF0:
            AUBUF0CON &= ~BIT(4);  //close aubuf ie
            AUBUF0CON |= BIT(1);   //Clear AUBUF0 THR Flag
             if (sddac_aubuf_is_full(SDDAC_AUBUF0)) {
                aubuf->au0_full_flag = 0;
                os_sem_au0_full_release();
            }
            break;
#if !SDDAC1_OPT_EN
        case SDDAC_AUBUF1:
            AUBUF1CON &= ~BIT(4);  //close aubuf ie
            AUBUF1CON |= BIT(1);   //Clear AUBUF1 THR Flag
            if (sddac_aubuf_is_full(SDDAC_AUBUF1)) {
                aubuf->au1_full_flag = 0;
                os_sem_au1_full_release();
            }
            break;
#endif
        default:
            break;
    }
}

AT(.com_text.sddac)
bool sddac_aubuf_dma_get_flag(SDDAC_AUBUF_TYPEDEF idx)
{
    if (!sddac_aubuf_en_check(idx)) {
        return 0;
    }

    switch (idx) {
        case SDDAC_AUBUF0:
            if (AUBUF0DMACON & BIT(8)) {
                return 1;
            }
            break;
#if !SDDAC1_OPT_EN
        case SDDAC_AUBUF1:
            if (AUBUF1DMACON & BIT(8)) {
                return 1;
            }
            break;
#endif
        default:
            break;
    }
    return 0;
}

AT(.com_text.sddac)
void sddac_aubuf_dma_clear_flag(SDDAC_AUBUF_TYPEDEF idx)
{
    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    switch (idx) {
        case SDDAC_AUBUF0:
            AUBUF0DMAKICK |= BIT(8);                     //Clear AUBUF0DMA Done Flag
            os_sem_au0_dma_release();
            break;
#if !SDDAC1_OPT_EN
        case SDDAC_AUBUF1:
            AUBUF1DMAKICK |= BIT(8);                     //Clear AUBUF1DMA Done Flag
            os_sem_au1_dma_release();
            break;
#endif
        default:
            break;
    }
}

//推静音数据
void sddac_aubuf_put_zero(SDDAC_AUBUF_TYPEDEF idx, uint16_t samples)
{
    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    // sddac_aubuf_dma_kick_w4_done(idx);

    while (samples--) {
        if (idx == SDDAC_AUBUF0) {
            if (sddac_aubuf_is_full(SDDAC_AUBUF0)) {
                AUBUF0CON |= BIT(1) | BIT(4);     //Audio Buffer Pend Clear, interrupt enable
                os_sem_au0_full_take(UINT_MAX);
            }
            AUBUF0DATA = 0;
            AUBUF0DATA24R = 0;
#if !SDDAC1_OPT_EN
        } else if (idx == SDDAC_AUBUF1) {
            if (sddac_aubuf_is_full(SDDAC_AUBUF1)) {
                AUBUF1CON |= BIT(1) | BIT(4);     //Audio Buffer Pend Clear, interrupt enable
                os_sem_au1_full_take(UINT_MAX);
            }
            AUBUF1DATA = 0;
            AUBUF1DATA24R = 0;
#endif
        }
    }
}

uint32_t sddac_samp_rate_get(SDDAC_AUBUF_TYPEDEF idx)
{
    uint32_t cur_spr = 0;

    if (!sddac_aubuf_en_check(idx)) {
        return cur_spr;
    }

    if (idx == SDDAC_AUBUF0) {
        cur_spr = ((DACSRC0PHAI & 0xffffff) * 1000) >> 15;
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        cur_spr = ((DACSRC1PHAI & 0xffffff) * 1000) >> 15;
#endif
    }

    return cur_spr;
}


void sddac_samp_rate_set(SDDAC_AUBUF_TYPEDEF idx, uint32_t spr)
{
    uint32_t cur_spr = 0;

    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    cur_spr = sddac_samp_rate_get(idx);

    if (spr == cur_spr) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        DACSRC0PHAI = (spr << 15) / 1000 | 1 << 31;//src0_in_sample
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        DACSRC1PHAI = (spr << 15) / 1000 | 1 << 31;//src1_in_sample
#endif
    }
}

uint16_t sddac_dig_vol_get(SDDAC_AUBUF_TYPEDEF idx)
{
    if (!sddac_aubuf_en_check(idx)) {
        return 0;
    }

    if (idx == SDDAC_AUBUF0) {
        return SRC0VOLCON & 0xffff;
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        return SRC1VOLCON & 0xffff;
#endif
    } else {
        return 0;
    }
}

void sddac_dig_vol_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t vol, uint8_t fade_step)
{
    uint16_t cur_src0vol = SRC0VOLCON & 0xffff;
#if !SDDAC1_OPT_EN
    uint16_t cur_src1vol = SRC1VOLCON & 0xffff;
#endif
    sddac_dig_typedef *dig = &sddac_cb.dig;

    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    if ((idx == SDDAC_AUBUF0) && (cur_src0vol != vol)) {
        if (sddac_is_mute(idx) == 0) {
            dig->src0_vol = vol;
        }
        SRC0VOLCON = vol;
        SRC0VOLCON |= fade_step << 24;
#if !SDDAC1_OPT_EN
    } else if ((idx == SDDAC_AUBUF1) && (cur_src1vol != vol)) {
        if (sddac_is_mute(idx) == 0) {
            dig->src1_vol = vol;
        }
        SRC1VOLCON = vol;
        SRC1VOLCON |= fade_step << 24;
#endif
    }
}

void sddac_fade_inout_set(SDDAC_FADE_DIR_TYPEDEF fade_dir, uint8_t fade_step)
{
    if (fade_dir == SDDAC_FADE_IN) {
        DACVOLCON = 0x7fff;
        DACVOLCON |= fade_step << 24;
    } else {
        DACVOLCON = 0;
        DACVOLCON |= fade_step << 24;
    }
}

bool sddac_fade_inout_done_flag(void)
{
    return (DACVOLCON & BIT(28));
}

void sddac_fade_inout_w4_done(void)
{
    while(!(DACVOLCON & BIT(28)));
}

bool sddac_is_mute(SDDAC_AUBUF_TYPEDEF idx)
{
    sddac_dig_typedef *dig = &sddac_cb.dig;
    if (!sddac_aubuf_en_check(idx)) {
        return 0;
    }

    if (idx == SDDAC_AUBUF0) {
        return dig->src0_mute;
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        return dig->src1_mute;
#endif
    } else {
        return 0;
    }
}

void sddac_mute_en(SDDAC_AUBUF_TYPEDEF idx, bool en)
{
    sddac_dig_typedef *dig = &sddac_cb.dig;
    uint16_t vol[2] = {dig->src0_vol, dig->src1_vol};

    if (sddac_is_mute(idx) == en) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        dig->src0_mute = en;
    } else {
        dig->src1_mute = en;
    }

    if (en == 1) {
        sddac_dig_vol_set(idx, 0, 2);
    } else {
        sddac_dig_vol_set(idx, vol[idx], 2);
    }
}

void sddac_aubuf_mix_en(bool en, bool div2_en)
{
    DACDIGCON0 = (en << 24) | (DACDIGCON0 & ~(1 << 24));
    DACDIGCON0 = (div2_en << 25) | (DACDIGCON0 & ~(1 << 25));
}

void sddac_aubuf_mix_coef_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t au_coef)
{
    if (!sddac_aubuf_en_check(idx)) {
        return;
    }

    if (idx == SDDAC_AUBUF0) {
        AU0LMIXCOEF = au_coef | (AU0LMIXCOEF & ~(0xffff << 0));
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1) {
        AU1LMIXCOEF = au_coef | (AU1LMIXCOEF & ~(0xffff << 0));
#endif
    }
}

AT(.com_text.sddac)
bool sddac_aubuf_en_check(SDDAC_AUBUF_TYPEDEF idx)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (idx == SDDAC_AUBUF0 && aubuf->au0_en == 1) {
        return 1;
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1 && aubuf->au1_en == 1) {
        return 1;
#endif
    }

    return 0;
}

AT(.com_text.sddac)
bool sddac_pcm_is_24bits(SDDAC_AUBUF_TYPEDEF idx)
{
    sddac_aubuf_typedef *aubuf = &sddac_cb.aubuf;

    if (idx == SDDAC_AUBUF0 && aubuf->au0_pcm_bits == SDDAC_PCM_24BITS) {
        return 1;
#if !SDDAC1_OPT_EN
    } else if (idx == SDDAC_AUBUF1 && aubuf->au1_pcm_bits == SDDAC_PCM_24BITS) {
        return 1;
#endif
    }

    return 0;
}

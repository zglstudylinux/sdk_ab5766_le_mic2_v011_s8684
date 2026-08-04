#include "include.h"
#include "bsp_sddac.h"
#include "driver_gpio.h"
#include "driver_sddac.h"
#include "api_sddac.h"

#define SDDAC_AUBUF0_EN          MUSIC_PLAY_EN   //音乐
#define SDDAC_AU0_SAMP_RATE      SDDAC_SPR_48k   //初始采样率
#define SDDAC_AU0_PCM_BITS       0   //0:16bis, 1:24bits
#define SDDAC_AU0_DMA_EN         1
#define SDDAC_AUBUF0_SIZE        (576*2)


#define SDDAC_AUBUF1_EN          TONE_PLAY_EN   //提示音
#define SDDAC_AU1_SAMP_RATE      SDDAC_SPR_16k  //初始采样率
#define SDDAC_AU1_PCM_BITS       0   //0:16bis, 1:24bits
#define SDDAC_AU1_DMA_EN         1
#define SDDAC_AUBUF1_SIZE        (576*2)


#define SDDA_MIX_EN              0
#define SDDA_MIX_DIV2_EN         0

#if SDDAC_AUBUF0_EN
#if SDDAC_AU0_PCM_BITS
uint32_t sddac_aubuf0[SDDAC_AUBUF0_SIZE] AT(.sddac_buf.au0);
#else
uint16_t sddac_aubuf0[SDDAC_AUBUF0_SIZE] AT(.sddac_buf.au0);
#endif  //SDDAC_AU1_PCM_BITS
#endif  //SDDAC_AUBUF1_EN

#if SDDAC_AUBUF1_EN
#if SDDAC_AU1_PCM_BITS
uint32_t sddac_aubuf1[SDDAC_AUBUF1_SIZE] AT(.sddac_buf.au1);
#else
uint16_t sddac_aubuf1[SDDAC_AUBUF1_SIZE] AT(.sddac_buf.au1);
#endif  //SDDAC_AU1_PCM_BITS
#endif  //SDDAC_AUBUF1_EN

AT(.sddac_buf.sddac_cb)
sddac_cb_typedef sddac_cb;

void bsp_sddac_aubuf_kick(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf_addr, uint32_t samples)
{
    sddac_aubuf_dma_kick(idx, buf_addr, samples);
    sddac_aubuf_dma_kick_w4_done(idx);
    if (idx == SDDAC_AUBUF0) {
        sddac_done_proc_kick(SDDAC_AU0_ALL_DONE);
    } else {
        sddac_done_proc_kick(SDDAC_AU1_ALL_DONE);
    }
}

AT(.com_text.sddac.isr)
void bsp_sddac_aubuf_isr(void)
{
    if (sddac_aubuf_get_flag(SDDAC_AUBUF0)) {
        sddac_aubuf_clear_flag(SDDAC_AUBUF0);  //clear函数中有线程和kick操作
    }

    if (sddac_aubuf_get_flag(SDDAC_AUBUF1)) {
        sddac_aubuf_clear_flag(SDDAC_AUBUF1);
    }
}

AT(.com_text.sddac.isr)
void bsp_sddac_aubuf_dma_isr(void)
{
    if (sddac_aubuf_dma_get_flag(SDDAC_AUBUF0)) {
        sddac_aubuf_dma_clear_flag(SDDAC_AUBUF0);  //clear函数中有线程和kick操作
    }

    if (sddac_aubuf_dma_get_flag(SDDAC_AUBUF1)) {
        sddac_aubuf_dma_clear_flag(SDDAC_AUBUF1);
    }
}

void bsp_sddac_process(SDDAC_AU_TYPEDEF au_type)
{
    if (au_type == SDDAC_AU0_ALL_DONE) {
#if SDDAC_AUBUF0_EN
        //音乐
        bsp_music_play_process(SDDAC_AU0_ALL_DONE);
#endif  //SDDAC_AUBUF0_EN
    }

    if (au_type == SDDAC_AU1_ALL_DONE) {
#if SDDAC_AUBUF1_EN
        //提示音
        bsp_tone_play_process(SDDAC_AU1_ALL_DONE);
#endif  //SDDAC_AUBUF1_EN
    }
}

void bsp_sddac_init(void)
{
    sddac_cb.init_flag = 1;
#if SDDAC_AUBUF0_EN
    memset(sddac_aubuf0,0, sizeof(sddac_aubuf0));
    sddac_cb.aubuf.au0_en = 1;
    sddac_cb.aubuf.au0_full_flag = 0;
    sddac_cb.aubuf.au0_pcm_bits = SDDAC_AU0_PCM_BITS;
    sddac_cb.aubuf.au0_size = SDDAC_AUBUF0_SIZE;
    sddac_cb.aubuf.au0_thr = SDDAC_AUBUF0_SIZE/4;
    sddac_cb.aubuf.au0_ptr = (uint32_t *)sddac_aubuf0;
    sddac_cb.aubuf.au0_start_sel = SDDAC_START_SIZE;
    sddac_cb.aubuf.au0_start_size = SDDAC_START_SIZE_128;

    sddac_cb.dig.src0_mute = 0;
    sddac_cb.dig.src0_bypass_en = 0;
    sddac_cb.dig.src0_vol = DIG_N0DB;
    sddac_cb.dig.au0mixcoef = DIG_N8DB;
    sddac_cb.dig.src0_spr = SDDAC_AU0_SAMP_RATE;
#else
    sddac_cb.aubuf.au0_en = 0;
#endif  //SDDAC_AUBUF0_EN

#if SDDAC_AUBUF1_EN
    memset(sddac_aubuf1,0, sizeof(sddac_aubuf1));
    sddac_cb.aubuf.au1_en = 1;
    sddac_cb.aubuf.au1_full_flag = 0;
    sddac_cb.aubuf.au1_pcm_bits = SDDAC_AU1_PCM_BITS;
    sddac_cb.aubuf.au1_size = SDDAC_AUBUF1_SIZE;
    sddac_cb.aubuf.au1_thr = SDDAC_AUBUF1_SIZE/4;
    sddac_cb.aubuf.au1_ptr = (uint32_t *)sddac_aubuf1;
    sddac_cb.aubuf.au1_start_sel = SDDAC_START_SIZE;
    sddac_cb.aubuf.au1_start_size = SDDAC_START_SIZE_128;

    sddac_cb.dig.src1_mute = 0;
    sddac_cb.dig.src1_bypass_en = 0;
    sddac_cb.dig.src1_vol = DIG_N0DB;
    sddac_cb.dig.au1mixcoef = DIG_N8DB;
    sddac_cb.dig.src1_spr = SDDAC_AU1_SAMP_RATE;
#else
    sddac_cb.aubuf.au1_en = 0;
#endif  //SDDAC_AUBUF1_EN

    sddac_cb.dig.mix_en = SDDA_MIX_EN;
    sddac_cb.dig.mix_div2_en = SDDA_MIX_DIV2_EN;
    sddac_cb.dig.total_vol = DIG_N0DB;
    sddac_cb.dig.dac_en = 1;

    //analog
    sddac_cb.ana.ch = SDDAC_CH_DIFF;

    sddac_sem_init();
    sddac_done_callback_set(bsp_sddac_process);

    sddac_clock_init();
    sddac_aubuf_init(&sddac_cb);
    sddac_dig_power_on(&sddac_cb);
    sddac_ana_init(&sddac_cb.ana);
    sddac_aubuf_pic_config(bsp_sddac_aubuf_isr, 0);
#if (SDDAC_AU0_DMA_EN || SDDAC_AU1_DMA_EN)
    sddac_aubuf_dma_pic_config(bsp_sddac_aubuf_dma_isr, 0);
#endif
    sddac_fade_inout_set(SDDAC_FADE_OUT, 100);
}

void bsp_sddac_deinit(void)
{
    sddac_deinit();
    sddac_ana_deinit();
}

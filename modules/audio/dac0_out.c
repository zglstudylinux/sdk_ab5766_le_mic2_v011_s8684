#include "include.h"
#include "dac0_out.h"
#include "driver_gpio.h"
#include "driver_sddac.h"
#include "api_sddac.h"

#if (WIRELESS_MIC_DAC_OUT_EN || ADAPTER_DAC_OUTPUT_EN)
uint16_t sddac_aubuf0[SDDAC_AUBUF0_SIZE];

static dac0_out_cfg_t dac0_out_cfg;
u32 aufifo0_cnt = 0;

sddac_cb_typedef sddac_cb;

AT(.com_text.sddac.isr)
static void dac0_out_aubuf_isr(void)
{
    if (sddac_aubuf_get_flag(SDDAC_AUBUF0)) {
        sddac_aubuf_clear_flag(SDDAC_AUBUF0);  //clear函数中有线程和kick操作
    }
}

AT(.com_text.sddac.isr)
static void dac0_out_aubuf_dma_isr(void)
{
    if (sddac_aubuf_dma_get_flag(SDDAC_AUBUF0)) {
        sddac_aubuf_dma_clear_flag(SDDAC_AUBUF0);  //clear函数中有线程和kick操作
    }
}

AT(.com_text.dac0_proc)
void dac0_out_audio_input(u8 *ptr, u32 samples, u32 params)
{
//    u32 pcm_mode = params & 0xffff;
//    bool is_24bit = pcm_mode & PCM_24BIT;
//    uint nch = pcm_mode & PCM_CHMASK;

    if (!dac0_out_cfg.mute) {
        sddac_aubuf_dma_kick(SDDAC_AUBUF0, (u32 *)ptr, samples);
        sddac_aubuf_dma_kick_w4_done(SDDAC_AUBUF0);
    }

//    if (dac0_out_cfg.callback) {
//        dac0_out_cfg.callback(ptr, samples, params);
//    }
}

AT(.com_text.dac0_proc)
void dac0_get_fifocnt(u32 tick_cnt)
{
    ///获取的样点时双声道，除一下2
    aufifo0_cnt = (AUBUF0FIFOCNT >> 18);
}

AT(.com_text.dac0_proc)
void dac0_play_sync_fifocnt(u16 high_thr, u16 low_thr)
{
    if (dac0_out_cfg.mute) {
        return;
    }
    u16 fifo_cnt = aufifo0_cnt;
    u32 dacsrcphi_nomal = (SDDAC_SPR_48k << 15)/1000;
    ///10万分之一
    u32 dacsrcphi_tiny = dacsrcphi_nomal/100000;

    if (fifo_cnt <= low_thr) {
        dacsrcphi_nomal = dacsrcphi_nomal - dacsrcphi_tiny*(low_thr - fifo_cnt);
    } else if (fifo_cnt >= high_thr) {
        dacsrcphi_nomal = dacsrcphi_nomal + dacsrcphi_tiny*(fifo_cnt - high_thr);
    } else {
        ///不需要修改
    }

    if (dacsrcphi_nomal != (DACSRC0PHAI&0xffffff)) {
        DACSRC0PHAI = dacsrcphi_nomal | 1 << 31;
    }
}

void dac0_out_audio_output_callback_set(audio_callback_t callback)
{
    dac0_out_cfg.callback = callback;
}

void dac0_out_audio_mute_set(uint8_t mute)
{
    dac0_out_cfg.mute = mute;
}

void dac0_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    printf("%s\n", __func__);
    memset(&dac0_out_cfg, 0, sizeof(dac0_out_cfg));
//    dac0_out_param_cfg_t *dac0_out_param_temp_ptr = dac0_out_param_get_by_sam_rate(sample_rate);
//    dac0_out_cfg.low_thr = dac0_out_param_temp_ptr->low_thr;
//    dac0_out_cfg.high_thr = dac0_out_param_temp_ptr->high_thr;
    dac0_out_cfg.sample_rate = sample_rate;

    sddac_cb.init_flag = 1;
    memset(sddac_aubuf0,0, sizeof(sddac_aubuf0));
    sddac_cb.aubuf.au0_en = 1;
    sddac_cb.aubuf.au0_full_flag = 0;
    sddac_cb.aubuf.au0_pcm_bits = 0;
    sddac_cb.aubuf.au0_size = samples*2;
    sddac_cb.aubuf.au0_thr = (samples*2)/4;
    sddac_cb.aubuf.au0_ptr = (uint32_t *)sddac_aubuf0;
    sddac_cb.aubuf.au0_start_sel = SDDAC_START_SIZE;
    sddac_cb.aubuf.au0_start_size = SDDAC_START_SIZE_128;

    sddac_cb.dig.src0_mute = 0;
    sddac_cb.dig.src0_bypass_en = 0;
    sddac_cb.dig.src0_vol = DIG_N0DB;
    sddac_cb.dig.au0mixcoef = DIG_N8DB;
    sddac_cb.dig.src0_spr = SDDAC_SPR_48k;

    sddac_cb.aubuf.au1_en = 0;

    sddac_cb.dig.mix_en = 0;
    sddac_cb.dig.mix_div2_en = 0;
    sddac_cb.dig.total_vol = DIG_N0DB;
    sddac_cb.dig.dac_en = 1;

    //analog
    if(xcfg_cb.dac_sel == SDDAC_CH_SINGLE) {
        sddac_cb.ana.ch = SDDAC_CH_SINGLE;   //单端
    }else {
        sddac_cb.ana.ch = SDDAC_CH_DIFF;     //差分
    }

    sddac_sem_init();

    sddac_clock_init();
    sddac_aubuf_init(&sddac_cb);
    sddac_dig_power_on(&sddac_cb);
    sddac_ana_init(&sddac_cb.ana);
    sddac_aubuf_pic_config(dac0_out_aubuf_isr, 0);
    sddac_aubuf_dma_pic_config(dac0_out_aubuf_dma_isr, 0);
    sddac_samp_rate_set(SDDAC_AUBUF0, SDDAC_SPR_48k);
    sddac_fade_inout_set(SDDAC_FADE_IN, 4);
}

void dac0_out_exit(void)
{
    printf("%s\n", __func__);
    dac0_out_audio_mute_set(1);
    while ((AUBUF0FIFOCNT >> 18)) {
//        printf("#");
    }

    dac_power_off();        //关闭dac 模拟相关
    sddac_deinit();
}

#endif

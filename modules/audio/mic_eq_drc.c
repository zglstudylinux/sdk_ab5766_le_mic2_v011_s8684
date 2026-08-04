/*
 * 文件名称: mic_eq_drc.c
 * 功能描述: EQ_DRC主要用来处理音效,主频160M，120个点,EQ和DRC算法时间分别为250us和180us左右;
 *           MIX_DRC主要用来一拖二音频混合,主频160M，120个点,算法时间为180us左右;
 ****************************************************************************************
 */
#include "include.h"
#include "mic_eq_drc.h"


#if WIRELESS_MIC_EQ_DRC_EN

static mic_eq_drc_cfg_t wireless_mic_eq_drc_cfg;

//static u8 pacc_obuf[WIRELESS_MIC_SAMPLES_SELECT*4] AT(.buf.mic_effect);
//------------------------------------------------------------------------------------------
//EQ_DRC处理
AT(.com_text.mic_eq_drc.input)
void mic_eq_drc_audio_input(u8 *ptr, u32 samples, u32 params)
{
    //硬件EQ_DRC
    if(wireless_mic_eq_drc_cfg.eq_drc_en && samples != 0) {
        loc_mic_pacc_process(ptr, ptr, samples);
    }
}

AT(.text.mic_eq_drc.init)
void mic_eq_drc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&wireless_mic_eq_drc_cfg, 0, sizeof(wireless_mic_eq_drc_cfg));

    //先初始化PACC链路
    loc_mic_pacc_init();

    //然后设置参数
    loc_mic_pacc_set_param();

    //最后使能PACC
    if(loc_mic_pacc_enable()) {
            my_printf("mic_eq_drc_init 4\n");
        wireless_mic_eq_drc_cfg.eq_drc_en = true;
    }

#if WIRELESS_MIC_SOFT_GAIN_EN
    soft_gain_init();
#endif

}

AT(.text.mic_eq_drc.exit)
void mic_eq_drc_exit(u8 sample_rate, u16 samples)
{
    loc_mic_pacc_exit();
}
#endif

#if ADAPTER_MIX_DRC_EN
//------------------------------------------------------------------------------------------
//MIX_DRC处理
//若output = NULL, MIX_DRC处理输出到pcm0
AT(.com_text.mix_drc.input)
void mix_drc_audio_input(mic_pcm_t *pcm0, mic_pcm_t *pcm1, mic_pcm_t *output, u16 samples)
{
    mix_pacc_process(output, pcm0, pcm1, samples);
}

AT(.text.mix_drc.init)
void mix_drc_init(void)
{
    //先初始化PACC链路
    mix_pacc_init();

    //然后设置参数
    mix_pacc_set_param();

    //最后使能PACC
    mix_pacc_enable();
}

AT(.text.mix_drc.exit)
void mix_drc_exit(void)
{
    mix_pacc_exit();
}
#endif

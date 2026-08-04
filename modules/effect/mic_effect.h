#ifndef __MIC_EFFECT_H
#define __MIC_EFFECT_H



enum {
    LOC_MIC_PACC_EQ_CS          = 0,
#if WIRELESS_FREQ_SHIFT2_EN
    LOC_MIC_PACC_FSH_CS,
#endif
    LOC_MIC_PACC_DRC_CS,

    LOC_MIC_PACC_MAX_CS,
};

enum {
    MIX_MIC_PACC_DRC_CS             = 0,
#if ADAPTER_FREQ_SHIFT_EN
    MIX_MIC_PACC_FSH_CS,
#endif
    MIX_MIC_PACC_EQ_CS,

    MIX_MIC_PACC_MAX_CS,
};

void loc_mic_pacc_init(void);
void loc_mic_pacc_set_param(void);
void loc_mic_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void loc_mic_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
bool loc_mic_pacc_enable(void);
void loc_mic_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void loc_mic_pacc_exit(void);

void mix_pacc_init(void);
void mix_pacc_set_param(void);
void mix_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
void mix_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void mix_pacc_enable(void);
void mix_pacc_process(mic_pcm_t *obuf, mic_pcm_t *ibuf0, mic_pcm_t *ibuf1, u32 samples);
void mix_pacc_exit(void);

#endif

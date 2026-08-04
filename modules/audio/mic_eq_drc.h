#ifndef __MIC_EQ_DRC_H
#define __MIC_EQ_DRC_H

typedef s16			mic_pcm_t;

typedef struct {
    u8 eq_drc_en;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} mic_eq_drc_cfg_t;


//EQ_DRC模块处理接口
void mic_eq_drc_init(u8 sample_rate, u16 samples, u8 channel);
void mic_eq_drc_audio_input(u8 *ptr, u32 samples, u32 params);
void mic_eq_drc_audio_output_callback_set(audio_callback_t callback);

//MIX_DRC模块处理接口
void mix_drc_audio_input(mic_pcm_t *pcm0, mic_pcm_t *pcm1, mic_pcm_t *output, u16 samples);
void mix_drc_init(void);

#endif

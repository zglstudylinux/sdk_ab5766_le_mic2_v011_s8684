#ifndef __MAGIC_API_H
#define __MAGIC_API_H

#define MAGIC_PROC_SAMPLES          120

///公共

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    u16 magic_effect_level;
} magic_cfg_t;
///magic
void magic_audio_mute_set(uint8_t mute);// 0：打开算法  1：关闭算法
///magic_param: -32767  ~ +32767
///默认值magic_mic_param_set(0)
void magic_audio_param_set(s32 magic_param);
void magic_audio_init(u8 sample_rate, u16 samples);
void magic_audio_process(s16 *ptr, u32 samples);
void magic_effect_level_change(void);
void magic_effect_level_set(u8 effect_level);
u8 magic_effect_level_get(void);

#endif

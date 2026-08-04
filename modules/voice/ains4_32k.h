#ifndef __AINS4_32K_H
#define __AINS4_32K_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} ains4_32k_mic_cfg_t;

///库接口外的模块接口声明
void ains4_32k_mic_init(u8 sample_rate, u16 samples, u8 channel);
void ains4_32k_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void ains4_32k_mic_output_callback_set(audio_callback_t callback);
void ains4_32k_mic_param_set(s16 ains4_32k_nt);
uint8_t ains4_32k_mic_mute_get(void);
void ains4_32k_mic_mute_set(uint8_t mute);
void ains4_32k_mic_exit(void);
void ains4_32k_mic_proc_cb(void);
int s_clip16(int x);
#endif

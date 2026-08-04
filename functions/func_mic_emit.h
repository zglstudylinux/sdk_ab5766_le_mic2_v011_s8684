#ifndef _FUNC_WIRELESS_MIC_EMIT_H
#define _FUNC_WIRELESS_MIC_EMIT_H

void wireless_mic_emit_conn_process(void);
void func_mic_emit(void);
void func_mic_emit_init(void);

///extern api
void func_mic_emit_message(u16 msg);

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);

void mic_init(u8 sample_rate, u16 samples, u8 channel);
void mic_audio_output_callback_set(audio_callback_t callback);

void dac0_out_init(u8 sample_rate, u16 samples, u8 channel);
void dac0_out_audio_output_callback_set(audio_callback_t callback);
void dac0_out_audio_input(u8 *ptr, u32 samples, u32 params);

u8 *dac_192k_out_get_obuf(void);
void dac_192k_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void dac_192k_out_audio_output_callback_set(audio_callback_t callback);
void dac_192k_out_init(u8 sample_rate, u16 samples, u8 channel);  //sample_rateŒﬁ”√
void dac_192k_out_stop(void);

void encoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel);
void encoder_prio_trans_audio_output_callback_set(audio_callback_t callback);
void encoder_prio_trans_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);

void mic_enc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void mic_enc_audio_output_callback_set(audio_callback_t callback);
void mic_enc_init(u8 sample_rate, u16 samples, u8 channel);
void mic_dec_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void mic_dec_audio_output_callback_set(audio_callback_t callback);
void mic_dec_init(u8 sample_rate, u16 samples, u8 channel);

#endif

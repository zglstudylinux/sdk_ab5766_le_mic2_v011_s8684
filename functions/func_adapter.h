#ifndef _FUNC_WIRELESS_MIC_REC_H
#define _FUNC_WIRELESS_MIC_REC_H


void func_adapter(void);
void func_adapter_init(void);

extern volatile uint8_t adapter_usb_init_flag;

///extern api
void func_adapter_message(u16 msg);

//void decoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel);
//void decoder_prio_trans_audio_output_callback_set(audio_callback_t callback);
//void decoder_prio_trans_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
//
//void src_init(u8 sample_rate, u16 samples, u8 channel);
//void src_audio_input(u8 *buf, u32 samples, int nch, void *params);
//void src_audio_output_callback_set(audio_callback_t callback);
//
void usb_mic_in_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
//void usb_mic_in_audio_output_callback_set(audio_callback_t callback);
//void usb_mic_in_init(u8 sample_rate, u16 samples, u8 channel);
//
//void usb_audio_out_init(u8 sample_rate, u16 samples, u8 channel);
//void usb_audio_out_audio_output_callback_set(audio_callback_t callback);

#endif

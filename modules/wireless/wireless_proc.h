#ifndef _WIRELESS_PRCO_H
#define _WIRELESS_PRCO_H

#define WIRELESS_MIC_STA_MASK           ((1<<WIRELESS_CON_LINK_NB)-1)

struct wireless_mic_tag {
    uint8_t change_flag;
    uint8_t change_sta[2];

    uint8_t connected_sta;
    uint8_t audio_sta;
    uint8_t fifo_sta;
    bool dma_flag;
#if WIRELESS_CON_PAIR_MODE
    uint8_t con_role_switch_flag;
    u32 con_id_switch_ticks;
    uint8_t con_temp;
#endif
};

extern struct wireless_mic_tag wireless_mic;

void wireless_mic_var_init(void);
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus);
void wireless_mic_dump(void);
void wireless_mic_proc(void);

void mic_enc_proc_cb(s16 *pcm, uint samples);
void mic_dec_prco_cb(u8 idx);

void mic_emit_init(void);
void mic_emit_reset(void);
void wireless_cmd_reset(u8 index);
void adapter_init(void);
void adapter_reset(u8 idx, u8 con_sta);
void ain3_set_level(u8 level);
void echo_set_params(u8 alg_echo_atten_set, u8 alg_echo_delay, u8 alg_echo_curroff_set, u8 alg_echo_wet_pct);
void mic_set_soft_gain(u16 gain);   //gain取值范围0~127,0~63正增益，64~127负增益

u8 wireless_emit_get_connected_sta(void);
#endif

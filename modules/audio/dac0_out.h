#ifndef _DAC0_OUT_H
#define _DAC0_OUT_H

#include "include.h"
#include "dec.h"

#define SDDAC_AUBUF0_SIZE        (120*2)

typedef struct {
    u8  mute;
    u8  sample_rate;
    u16 samples;
    u8  low_thr;
    u8  high_thr;
    u8 first_kick_flag;
    audio_callback_t callback;
} dac0_out_cfg_t;

void dac0_out_init(u8 sample_rate, u16 samples, u8 channel);
void dac0_out_audio_input(u8 *ptr, u32 samples, u32 params);
void dac0_get_fifocnt(u32 tick_cnt);
void dac0_play_sync_fifocnt(u16 high_thr, u16 low_thr);
void dac0_out_exit(void);
#endif

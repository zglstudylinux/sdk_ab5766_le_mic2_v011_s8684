#ifndef _BSP_TONE_H_
#define _BSP_TONE_H_
#include "include.h"
#include "res.h"
#include "dec.h"

typedef enum {
    TONE_TYPE_NONE,
    TONE_TYPE_SIN,
    TONE_TYPE_MP3,
    TONE_TYPE_WAV,
} TONE_TYPE_TYPEDEF;

typedef struct {
    bool sta;
    uint8_t sys_clk_rec;
    uint8_t  type;
    uint32_t addr;
    uint32_t len;
} tone_paly_cb_t;

void bsp_tone_play_output_mute(bool mute);
void bsp_tone_play_volume_set(uint16_t vol);
uint16_t bsp_tone_play_volume_get(void);
void bsp_tone_play_volume_proc(int16_t *dest, int16_t *src, uint16_t len);
void bsp_tone_play_init(void);
void bsp_tone_play(uint8_t tone_type, uint32_t addr, uint32_t len);
void bsp_tone_play_process(SDDAC_AU_TYPEDEF done_type);

#endif  //_BSP_TONE_H_

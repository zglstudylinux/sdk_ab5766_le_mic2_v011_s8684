#ifndef _DEC_H_
#define _DEC_H_
#include "include.h"
#include "driver_sddac.h"
#include "api_sddac.h"

/////////////////SIN
enum {
    INDEX_SIN_RES_16K_1K = 0,  //16K采样率的1K正弦波
    INDEX_SIN_RES_48K_1K,
    INDEX_SIN_RES_MAX,
};

typedef struct {
    uint8_t index;
    void *addr;
    uint16_t len;   //样点数
    uint32_t spr;
    uint16_t time;  //音频持续时间，ms
} sin_res_tbl_t;

extern sin_res_tbl_t sin_res_tbl[INDEX_SIN_RES_MAX];

void sin_res_init(void);
void sin_res_stop(void);
bool sin_res_play(uint32_t addr, uint32_t len);
uint8_t sin_res_play_proc(SDDAC_AU_TYPEDEF done_type);

/////////////////MP3
void mp3_res_init(void);
void mp3_res_stop(void);
bool mp3_res_play(uint32_t addr, uint32_t len);
uint8_t mp3_res_play_proc(SDDAC_AU_TYPEDEF done_type);

/////////////////WAV
void wav_res_init(void);
void wav_res_stop(void);
bool wav_res_play(uint32_t addr, uint32_t len);
uint8_t wav_res_play_proc(SDDAC_AU_TYPEDEF done_type);

/////////////////MIC TO DAC
void mic_t_dac_init(void);
void mic_t_dac_stop(void);
bool mic_t_dac_play(uint32_t addr, uint32_t len);
uint8_t mic_t_dac_play_proc(SDDAC_AU_TYPEDEF done_type);
void mic_t_dac_pp(void);
#endif  //_DEC_H_

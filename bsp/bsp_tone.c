#include "include.h"
#include "bsp_tone.h"
#include "driver_gpio.h"
#include "driver_sddac.h"
#include "api_sddac.h"

#if (TONE_PLAY_EN)
AT(.codec.tone_play)
tone_paly_cb_t tone_play_cb;

uint8_t bsp_tone_play_sta_get(void)
{
    return tone_play_cb.sta;
}

void bsp_tone_play_volume_set(uint16_t vol)
{
    sddac_dig_vol_set(SDDAC_AUBUF1, vol, 4);
}

uint16_t bsp_tone_play_volume_get(void)
{
    return sddac_dig_vol_get(SDDAC_AUBUF1);
}

void bsp_tone_play_init(void)
{
    memset(&tone_play_cb, 0, sizeof(tone_paly_cb_t));

#if (TONE_PLAY_EXTERN_FLASH_EN)
    //uint extern_spiflash_init(void);  //用户自己添加
    extern_spiflash_init(void);
#endif  //(TONE_PLAY_EXTERN_FLASH_EN)

#if (TONE_PLAY_SIN_DEC_EN)
    sin_res_init();
#endif  //(TONE_PLAY_SIN_DEC_EN)

#if (TONE_PLAY_MP3_DEC_EN)
    mp3_res_init();
#endif  //(TONE_PLAY_MP3_DEC_EN)

#if (TONE_PLAY_WAV_DEC_EN)
    wav_res_init();
#endif  //(TONE_PLAY_WAV_DEC_EN)

    bsp_tone_play_volume_set(DIG_N10DB);
}

void bsp_tone_play_stop(void)
{
    tone_play_cb.sta = 0;

#if (TONE_PLAY_SIN_DEC_EN)
    sin_res_stop();
#endif  //(TONE_PLAY_SIN_DEC_EN)   

#if (TONE_PLAY_MP3_DEC_EN)
    mp3_res_stop();
#endif  //(TONE_PLAY_MP3_DEC_EN)   

#if (TONE_PLAY_WAV_DEC_EN)
    wav_res_stop();
#endif  //(TONE_PLAY_WAV_DEC_EN)   

    if (tone_play_cb.sys_clk_rec != 0) {
        sys_clk_set(tone_play_cb.sys_clk_rec);  //恢复时钟
    }
	
    sddac_fade_inout_set(SDDAC_FADE_OUT, 4);
    sddac_aubuf_put_zero(SDDAC_AUBUF1, (sddac_cb.aubuf.au1_start_size+1)*64);  //静音数据
}

void bsp_tone_play(uint8_t tone_type, uint32_t addr, uint32_t len)
{
    bool res = 0;

    tone_play_cb.sta = 1;
    tone_play_cb.type = tone_type;
    tone_play_cb.addr = addr;
    tone_play_cb.len  = len;

    WDT_CLR();

#if (SYS_SLEEP_TIME)
    lowpwr_sleep_auto_dis();
#endif //SYS_SLEEP_TIME

    if (sys_clk_get() < SYS_48M) {
        tone_play_cb.sys_clk_rec = sys_clk_get();
        sys_clk_set(SYS_48M);
    }

    printf("tone play volume:%d\n", bsp_tone_play_volume_get());

    sddac_fade_inout_set(SDDAC_FADE_IN, 4);

    switch (tone_type) {
        case TONE_TYPE_SIN: {
                #if (TONE_PLAY_SIN_DEC_EN)
                    res = sin_res_play(addr, len);
                #endif  //(TONE_PLAY_SIN_DEC_EN)
            } break;
        case TONE_TYPE_MP3: {
                #if (TONE_PLAY_MP3_DEC_EN)
                    res = mp3_res_play(addr, len);
                #endif  //(TONE_PLAY_MP3_DEC_EN)
            } break;
        case TONE_TYPE_WAV: {
                #if (TONE_PLAY_WAV_DEC_EN)
                    res = wav_res_play(addr, len);
                #endif  //(TONE_PLAY_WAV_DEC_EN)
            } break;

        default:
            break;
    }

    if (res == 0) {
        printf("!!tone play err!!\n");
    }
}

void bsp_tone_play_process(SDDAC_AU_TYPEDEF done_type)
{
    uint16_t end = 0;
    switch (tone_play_cb.type) {
        case TONE_TYPE_SIN: {
                #if (TONE_PLAY_SIN_DEC_EN)
                    end = sin_res_play_proc(done_type);
                #endif  //(TONE_PLAY_SIN_DEC_EN)
            } break;
        case TONE_TYPE_MP3: {
                #if (TONE_PLAY_MP3_DEC_EN)
                    end = mp3_res_play_proc(done_type);
                #endif  //(TONE_PLAY_MP3_DEC_EN)
            } break;
        case TONE_TYPE_WAV: {
                #if (TONE_PLAY_WAV_DEC_EN)
                    end = wav_res_play_proc(done_type);
                #endif  //(TONE_PLAY_WAV_DEC_EN)
            } break;

        default:
            break;
    }

    if (end == 1) {
        printf("tone play end!\n");
        bsp_tone_play_stop();
        
        #if (SYS_SLEEP_TIME)
            lowpwr_sleep_auto_en();
        #endif //SYS_SLEEP_TIME
    }
}

#endif  //(TONE_PLAY_EN)

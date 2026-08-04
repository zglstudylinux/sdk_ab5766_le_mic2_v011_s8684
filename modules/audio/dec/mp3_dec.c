#include "include.h"
#include "driver_sddac.h"
#include "res.h"
#include "api_codec.h"
#include "driver_gpio.h"

#if (TONE_PLAY_MP3_DEC_EN)
//系统时钟：SYS CLK 48M 以上
//支持采样率：0:48K, 1:24K, 2:16K, 3:12K, 4:8K
uint8_t music_mp3_long_buffer = 1;                 // 大于192
uint8_t spi_audio_test_en = 0;                     // 测试模式使能
uint16_t mp3_fade_in_time = 36;                    // 淡入时间，36的倍数
uint16_t mp3_fade_out_time = 36;                   // 淡出时间，36的倍数

enum {
    MUSIC_STOP = 0,
    MUSIC_PAUSE,
    MUSIC_PLAYING,
};

AT(.text.spr.table)
const uint16_t mp3_sample_rate_tbl[] = {
    SDDAC_SPR_48k,
    SDDAC_SPR_24k,
    SDDAC_SPR_16k,
    SDDAC_SPR_12k,
    SDDAC_SPR_8k
};

static int16_t *mp3_dec_obuf_prt = NULL;
static uint32_t mp3_dec_obuf_len = 576;
static uint8_t mp3_dec_spr = 2;  //0:48K, 1:24K, 2:16K, 3:12K, 4:8K， 默认16K

void spi_read_function_register(void (* read_func)(uint8_t *buf, uint32_t addr, uint32_t len));

AT(.com_text.dac)
void mp3_dec_end_call_back(int16_t *ptr, uint32_t samples)
{
    //samples  576
    mp3_dec_obuf_prt = ptr;
    mp3_dec_obuf_len = samples;
}

void mp3_dec_spr_set_call_back(uint8_t spr)
{
    mp3_dec_spr = spr;
}

void mp3_res_init(void)
{
#if TONE_PLAY_EXTERN_FLASH_EN
    //uint extern_spiflash_read(void *buf, u32 addr, uint len);  //用户自己添加
    spi_read_function_register((void *)extern_spiflash_read);    //mp3注册外挂flash读接口
#else
    spi_read_function_register(NULL);
#endif
}

void mp3_res_stop(void)
{
    mp3_res_play_exit();
}

bool mp3_res_play(uint32_t addr, uint32_t len)
{
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);

    if (len == 0) {
        return false;
    }

    if (mp3_music_dec_sta_get() != MUSIC_STOP) {
        mp3_music_dec_sta_set(MUSIC_STOP);
    }

    //mp3 dec init
    mp3_res_play_kick(addr, len);

    //mp3 dec first frame, get sampling rate
    mp3_dec_frame();
    mp3_music_pcm_kick();

    sddac_samp_rate_set(SDDAC_AUBUF1, mp3_sample_rate_tbl[mp3_dec_spr]);
    printf("mp3 spr:%d\n", sddac_samp_rate_get(SDDAC_AUBUF1));

    bsp_sddac_aubuf_kick(SDDAC_AUBUF1, (uint32_t*)mp3_dec_obuf_prt, mp3_dec_obuf_len);
    return true;
}

uint8_t mp3_res_play_proc(SDDAC_AU_TYPEDEF done_type)
{
    uint8_t ret = 0;

    if (mp3_music_dec_sta_get() != MUSIC_STOP) {
        if (mp3_dec_frame()) {
            mp3_music_pcm_kick();

            bsp_sddac_aubuf_kick(SDDAC_AUBUF1, (uint32_t*)mp3_dec_obuf_prt, mp3_dec_obuf_len);
        } else {
            #if (SYS_SLEEP_TIME)
                lowpwr_sleep_auto_en();
            #endif //SYS_SLEEP_TIME
            mp3_res_play_exit();
            ret = 1;
        }
    }

    return ret;
}

#endif  //(TONE_PLAY_MP3_DEC_EN)

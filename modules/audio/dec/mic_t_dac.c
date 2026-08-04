#include "include.h"
#include "driver_sddac.h"
#include "dec.h"
#include "api_codec.h"
#include "driver_gpio.h"
#include "api_sddac.h"
#include "bsp.h"

#if (MUSIC_PLAY_MIC_T_DAC_EN)

typedef struct {
    uint8_t sta;
} mic_t_dac_cb_t;

AT(.codec.sin_cb)
mic_t_dac_cb_t mic_t_dac_cb;

void mic_t_dac_init(void)
{
    memset(&mic_t_dac_cb, 0, sizeof(mic_t_dac_cb_t));
}

void mic_t_dac_stop(void)
{
    mic_t_dac_cb.sta = 0;
}

bool mic_t_dac_play(uint32_t addr, uint32_t len)
{
    printf("%s: addr: 0x%x, len: 0x%x\n", __func__, addr, len);

    if (len == 0) {
        return false;
    }

    if (mic_t_dac_cb.sta == 1) {
        mic_t_dac_cb.sta = 0;
    }
    
    mic_t_dac_cb.sta = 1;

    sddac_samp_rate_set(SDDAC_AUBUF0, SDDAC_SPR_48k);
    printf("mic to dac spr:%d\n", sddac_samp_rate_get(SDDAC_AUBUF0));

    bsp_sddac_aubuf_kick(SDDAC_AUBUF0, (uint32_t *)addr, len);
    return true;
}

uint8_t mic_t_dac_play_proc(SDDAC_AU_TYPEDEF done_type)
{
    uint8_t ret = 0;
    uint8_t *dac_buf_ptr = NULL;
    if (mic_t_dac_cb.sta == 1) {
        if (done_type == SDDAC_AU0_ALL_DONE) {
            dac_buf_ptr = bsp_sdadc_voice_data_get(128);
            if (dac_buf_ptr) {
                bsp_sddac_aubuf_kick(SDDAC_AUBUF0, (uint32_t *)dac_buf_ptr, 128);
            } else {
                os_sem_au0_dma_take(4);
                sddac_done_proc_kick(SDDAC_AU0_ALL_DONE);
            }
       }
    } else {
        ret = 1;
    }

    return ret;
}

void mic_t_dac_pp(void)
{
    if (!mic_t_dac_cb.sta) {
        bsp_sdadc_voice_start();
        delay_ms(14);  //缓存1包mic数据  48K 576个样点
        bsp_music_play(MUSIC_TYPE_MIC_T_DAC, (uint32_t)bsp_sdadc_voice_data_get(128), 128);

        printf("AUANGCON0:0x%x\n", AUANGCON0);
        printf("AUANGCON1:0x%x\n", AUANGCON1);
        printf("AUANGCON2:0x%x\n", AUANGCON2);
        printf("AUANGCON3:0x%x\n", AUANGCON3);
        printf("AUANGCON4:0x%x\n", AUANGCON4);
    } else {
        mic_t_dac_stop();
        bsp_sdadc_voice_stop();
    }
}

#endif  //(MUSIC_PLAY_MIC_T_DAC_EN)

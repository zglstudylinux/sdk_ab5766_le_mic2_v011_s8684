#include "include.h"
#include "driver_sddac.h"
#include "dec.h"
#include "api_codec.h"
#include "driver_gpio.h"
#include "api_sddac.h"

#if (TONE_PLAY_SIN_DEC_EN)

typedef struct {
    uint8_t sta;
    uint8_t index;
    uint32_t tick;
} sin_res_cb_t;

AT(.codec.sin_cb)
sin_res_cb_t sin_res_cb;

//16K采样率的1K正弦波
AT(.com_rodata.sindata)
const uint16_t SinData_16k_16bit[16] = {
    0x0000, 0x30FB, 0x5A82, 0x7641, 0x7FFF, 0x7641, 0x5A82, 0x30FB,
    0x0000, 0xCF05, 0xA57E, 0x89BF, 0x8001, 0x89BF, 0xA57E, 0xCF05
};

//48K采样率的1K正弦波
AT(.com_rodata.sindata)
const uint16_t SinData_48k_16bit[48] = {
    0x0000, 0x10B5, 0x2121, 0x30FB, 0x3FFF, 0x4DEB, 0x5A82, 0x658C,
    0x6ED9, 0x7641, 0x7BA2, 0x7EE7, 0x7FFF, 0x7EE7, 0x7BA2, 0x7641,
    0x6ED9, 0x658C, 0x5A82, 0x4DEB, 0x3FFF, 0x30FB, 0x2121, 0x10B5,
    0x0000, 0xEF4B, 0xDEDF, 0xCF05, 0xC001, 0xB215, 0xA57E, 0x9A74,
    0x9127, 0x89BF, 0x845E, 0x8119, 0x8001, 0x8119, 0x845E, 0x89BF,
    0x9127, 0x9A74, 0xA57E, 0xB215, 0xC000, 0xCF05, 0xDEDF, 0xEF4B
};

sin_res_tbl_t sin_res_tbl[INDEX_SIN_RES_MAX] = {
    {INDEX_SIN_RES_16K_1K, (uint16_t *)SinData_16k_16bit, sizeof(SinData_16k_16bit)/2, SDDAC_SPR_16k, 70},
    {INDEX_SIN_RES_48K_1K, (uint16_t *)SinData_48k_16bit, sizeof(SinData_48k_16bit)/2, SDDAC_SPR_48k, 70},
};

uint8_t sin_res_get_index_for_addr(uint32_t res_addr)
{
    for (uint8_t i = 0; i < INDEX_SIN_RES_MAX; i ++) {
        if (res_addr == (uint32_t)sin_res_tbl[i].addr) {
            return sin_res_tbl[i].index;
        }
    }

    return INDEX_SIN_RES_MAX;
}

void sin_res_init(void)
{
    memset(&sin_res_cb, 0, sizeof(sin_res_cb_t));
}

void sin_res_stop(void)
{
    sin_res_cb.sta = 0;
}

bool sin_res_play(uint32_t addr, uint32_t len)
{
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);

    if (len == 0) {
        return false;
    }

    if (sin_res_cb.sta == 1) {
        sin_res_cb.sta = 0;
    }

    sin_res_cb.index = sin_res_get_index_for_addr(addr);
    sin_res_cb.sta = 1;
    sin_res_cb.tick = tick_get();

    sddac_samp_rate_set(SDDAC_AUBUF1, sin_res_tbl[sin_res_cb.index].spr);
    printf("sin spr:%d\n", sddac_samp_rate_get(SDDAC_AUBUF1));

    bsp_sddac_aubuf_kick(SDDAC_AUBUF1, (uint32_t*)addr, len);
    return true;
}

uint8_t sin_res_play_proc(SDDAC_AU_TYPEDEF done_type)
{
    uint8_t ret = 0;
    if (sin_res_cb.sta == 1) {
        if (done_type == SDDAC_AU1_ALL_DONE) {
            // if (tick_check_expire(sin_res_cb.tick, sin_res_tbl[sin_res_cb.index].time)) {
            //     sin_res_cb.sta = 0;
            //     ret = 1;
            // } else {
                bsp_sddac_aubuf_kick(SDDAC_AUBUF1, (uint32_t *)sin_res_tbl[sin_res_cb.index].addr, sin_res_tbl[sin_res_cb.index].len);
    //         }
       }
    }

    return ret;
}

#endif  //(TONE_PLAY_SIN_DEC_EN)

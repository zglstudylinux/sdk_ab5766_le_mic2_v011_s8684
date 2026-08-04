#include "include.h"
#include "driver_sddac.h"
#include "res.h"
#include "api_codec.h"
#include "driver_gpio.h"
#include "api_sddac.h"

#if (TONE_PLAY_WAV_DEC_EN)
/*****************************************************/
#define WAV_PACKED_FORMAT   0   // 0: PCM
                                // 1: ADPCM
/*****************************************************/

#define PCM16_2_ADPCM_BLOCK(x)              ((x-2)/4+4)
#define ADPCM_BLOCK_2_PCM16_BYTES(x)        ((x-4)*4+2)
#define ADPCM_SAMPLE_2_BLOCK(x)             ((x-1)/2+4)

typedef struct {
    u8 *addr;
    uint32_t pos;
    uint32_t len;
} spi_res_cb_t;

AT(.codec.wav_cb)
spi_res_cb_t spires_wav_cb;

typedef struct {
    u8 sta;
    u8 nch;
    uint32_t spr;
#if WAV_PACKED_FORMAT
    u16 block_sample_size; // Number of samples in one block for IMA-ADPCM
#endif // WAV_PACKED_FORMAT
    u8 *buf;
} wav_res_cb_t;

AT(.codec.wav_cb)
wav_res_cb_t wavres_cb;


#if WAV_PACKED_FORMAT
// One ADPCM Block min 32B sample, max 512B.
// That means decode buf_size(PCM 16bits) min 114B, max 2034B.
bool adpcm_dma_flag = 0;  //0:half done, 1:all done
u16 adpcm_block_bytes;
u8 wav_res_buf[512] AT(.codec.wav_buf);
u8 adpcm_decode_obuf[ADPCM_BLOCK_2_PCM16_BYTES(512)] AT(.codec.wav.buf);
#else
u8 wav_res_buf[512] AT(.codec.wav_buf);
#endif // WAV_PACKED_FORMAT

void wav_dec_buf_init(void)
{
    memset(wav_res_buf, 0, sizeof(wav_res_buf));
#if WAV_PACKED_FORMAT
    memset(wav_res_buf, 0, sizeof(adpcm_decode_obuf));
#endif
}

AT(.text.spires)
int wav_res_stream_read(void *buf, unsigned int size)
{
    int rlen = 0;
    spi_res_cb_t *s = &spires_wav_cb;

    if ((s->pos + size) >= s->len) {
        rlen = s->len - s->pos;
    } else {
        rlen = size;
    }
    if (rlen) {
#if (TONE_PLAY_EXTERN_FLASH_EN)
        //uint extern_spiflash_read(void *buf, u32 addr, uint len);  //用户自己添加
        extern_spiflash_read(buf,(u32)(s->addr + s->pos),rlen);
#else
		memcpy(buf, s->addr + s->pos, rlen);
#endif  //(TONE_PLAY_EXTERN_FLASH_EN)
    }
    s->pos += rlen;

    return rlen;
}

AT(.text.spires)
bool wav_res_is_play(void)
{
    if (wavres_cb.sta) {
        return true;
    }
    return false;
}

uint16_t wav_res_play_kick(uint32_t addr, uint32_t len)
{
    int ret;
    spi_res_cb_t *s = &spires_wav_cb;
    wav_res_cb_t *w = &wavres_cb;

    s->addr = (u8 *)addr;
    s->len = len;
    s->pos = 0;

    memset(&wavres_cb, 0, sizeof(wav_res_cb_t));
    w->buf = wav_res_buf;
    ret = wav_res_stream_read(w->buf, 512);

    if (ret != 512) {
        return 0;
    }

#if WAV_PACKED_FORMAT
    return wav_res_analize(w->buf, &w->nch, &w->spr, &w->block_sample_size);
#else
    return wav_res_analize(w->buf, &w->nch, &w->spr, NULL);
#endif // WAV_PACKED_FORMAT
}

void wav_res_init(void)
{
    wavres_cb.sta = 0;
#if WAV_PACKED_FORMAT
    adpcm_dma_flag = 0;
#endif
}

void wav_res_stop(void)
{
    wavres_cb.sta = 0;
}

bool wav_res_play(uint32_t addr, uint32_t len)
{
    u16 block_len;
    uint16_t offset;
    uint32_t *obuf_ptr;
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);

    wav_dec_buf_init();
    offset = wav_res_play_kick(addr, len);

    if(offset && (wavres_cb.nch == 1)){         //only support mono
        printf("offset:%d\n",offset);
        printf("nch:%d\n",wavres_cb.nch);
        printf("wav spr:%d\n",wavres_cb.spr);

        wavres_cb.sta = 1;
        spires_wav_cb.pos = offset;

#if WAV_PACKED_FORMAT
        if (wavres_cb.block_sample_size) {
            printf("ADPCM Block: %d(Samples)\n", wavres_cb.block_sample_size);
        }

        /* Check block size enough to decode. */
        adpcm_block_bytes = ADPCM_SAMPLE_2_BLOCK(wavres_cb.block_sample_size);

        printf("ADPCM Block Size: %d\n", adpcm_block_bytes);

        wav_res_stream_read(wavres_cb.buf, adpcm_block_bytes);

        adpcm_decode_block((uint8_t*)adpcm_decode_obuf, wavres_cb.buf, wavres_cb.block_sample_size);
        block_len = wavres_cb.block_sample_size;
        obuf_ptr = (uint32_t*)adpcm_decode_obuf;
#else
        block_len = wav_res_stream_read(wavres_cb.buf, 128) / 2;  //样点数
        obuf_ptr = (uint32_t*)wavres_cb.buf;
#endif // WAV_PACKED_FORMAT

        sddac_samp_rate_set(SDDAC_AUBUF1, wavres_cb.spr);

        bsp_sddac_aubuf_kick(SDDAC_AUBUF1, obuf_ptr, block_len);
        return true;
    }

    return false;
}

AT(.text.spires)
uint8_t wav_res_dec_process(SDDAC_AU_TYPEDEF done_type)
{
    int res_len;
    uint16_t buf_len;
    uint32_t *buf_ptr = NULL;
    wav_res_cb_t *w = &wavres_cb;
#if WAV_PACKED_FORMAT
    uint32_t half_done_point = wavres_cb.block_sample_size / 2;
    uint32_t all_done_point = half_done_point;

    if(wavres_cb.block_sample_size % 2){
        half_done_point = half_done_point + 1;
        all_done_point = wavres_cb.block_sample_size - half_done_point;
    }

    if (done_type == SDDAC_AU1_ALL_DONE) {
        if (adpcm_dma_flag == 0) {
            adpcm_dma_flag = 1;
            res_len = wav_res_stream_read(w->buf, adpcm_block_bytes);
            if (res_len < adpcm_block_bytes) {
                w->sta = 0;
                return 1;
            }
            adpcm_decode_block((uint8_t*)adpcm_decode_obuf, w->buf, wavres_cb.block_sample_size);
            buf_len = half_done_point;
            buf_ptr = (uint32_t *)&adpcm_decode_obuf[0];
        } else {
            adpcm_dma_flag = 0;
            buf_len = all_done_point;
            buf_ptr = (uint32_t *)&adpcm_decode_obuf[half_done_point*2];
        }
    }
#else
    if (done_type == SDDAC_AU1_ALL_DONE) {
        res_len = wav_res_stream_read(w->buf, 128);
        if (res_len > 0) {
            buf_len = res_len/2;
            buf_ptr = (uint32_t *)wavres_cb.buf;
        } else {
            w->sta = 0;
            return 1;
        }
    }
#endif // WAV_PACKED_FORMAT

    if (buf_ptr != NULL) {
        bsp_sddac_aubuf_kick(SDDAC_AUBUF1, buf_ptr, buf_len);
    }

    return 0;
}

uint8_t wav_res_play_proc(SDDAC_AU_TYPEDEF done_type)
{
    uint8_t ret = 0;
    if (wavres_cb.sta == 1) {
        if (wav_res_is_play()) {
            ret = wav_res_dec_process(SDDAC_AU1_ALL_DONE);
        }
    }

    return ret;
}

#endif  //(TONE_PLAY_WAV_DEC_EN)

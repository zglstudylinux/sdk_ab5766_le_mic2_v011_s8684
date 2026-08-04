#include "include.h"
#include "wireless.h"
#include "res.h"
#include "driver_com.h"
#include "driver_gpio.h"
#include "api_alg.h"
#define MIC_ENC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE
#define MIC_DEC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE

#define MIC_DEC_OBUF_SIZE           (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_CHANNEL_SELECT*2)
#define MIC_CACHE_BUF_NUM           2

typedef struct {
    u8 buffer[MIC_ENC_BUFFER_SIZE];
#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
    u8 enc_cache_buf[MIC_DEC_OBUF_SIZE*MIC_CACHE_BUF_NUM];
    u8 enc_cache_cnt;
    u8 key_press_mute_en;
#endif
//    audio_callback_t callback;
    bool kick_flag;
    uint8_t mute_en;                //控制mic端是否静音mute
    uint8_t first_pkt;
#if WIRELESS_MIC_DAC_OUT_EN
    uint8_t dac_cnt;
#endif
} mic_enc_t;

typedef struct {
    audio_callback_t callback;
    u8 obuf[MIC_DEC_OBUF_SIZE];
    struct {
        u8 buf[MIC_DEC_OBUF_SIZE];
        u16 buf_cnt;
    } pcm[WIRELESS_CON_LINK_NB];

    u8 frame[MIC_DEC_BUFFER_SIZE];
    u8 last_bfi[WIRELESS_CON_LINK_NB];
    u8 frag_samples[2];
    bool frag0_done_flag;
    uint8_t obuf_off;
    u8 sync_idx;
    u8 fifo_sta;
} mic_dec_t;

AT(.buf.mic_emit.proc)
mic_enc_t mic_enc;

AT(.buf.adapter.proc)
mic_dec_t mic_dec;

void wl_get_tick1_time(uint tick_cnt);
void wl_play_sync_tick1(uint8_t interval, uint8_t spr_idx);
void plc_soft_init(u8 idx, uint pkt_len, u8 sample_rate);
void plc_soft_process(s16 *int_data, u32 samples, u8 bfi, u8 idx);
void plc_soft_exit(uint8_t idx);
///------------------------------------------------------------------------------------------
void mic_emit_alg_init(void)
{
}

//DAC播放偏差处理
AT(.text.adapter.mic_dec)
static void mic_dec_dac_sync_proc(void)
{
#if ADAPTER_DAC_OUTPUT_EN
    if(mic_dec.fifo_sta <= 10) {
        mic_dec.fifo_sta++;
        return;
    } else {
        uint high_thr, low_thr;
        uint samples = cfg_wireless_d2a_dec_us*48/1000;
        high_thr = samples + 12;
        low_thr = samples + 6;

        mic_dec.fifo_sta = 0;
        dac0_play_sync_fifocnt(high_thr, low_thr);
    }
#endif
}

void adapter_alg_init(void)
{
    plc_soft_init(0, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_SAMPLE_RATE_SELECT);
    plc_soft_init(1, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_SAMPLE_RATE_SELECT);
}

#if !WIRELESS_MIC_CLOSE_EN
AT(.text.mic_emit.proc)
void mic_enc_adc_dma_kick(uint tick_cnt)
{
    wl_get_tick1_time(tick_cnt);
    if(sys_cb.mic_alg_en) {
        bsp_sdadc_kick();
    }
}

AT(.text.mic_mute_set)
void mic_enc_mute_en_set(uint mute_en)
{
    mic_enc.mute_en = mute_en;
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_mic_mute_level_write((u8 *)&mic_enc.mute_en);
#endif
}

AT(.text.mic_mute_get)
u8 mic_enc_mute_en_get(void)
{
    return mic_enc.mute_en;
}

#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
static volatile u8 enc_cache_rptr = 1;

AT(.text.bsp_key_scan)
void mic_enc_key_press_mute_en_set(uint mute_en)
{
    mic_enc.key_press_mute_en = mute_en;
}

AT(.text.bsp_key_scan)
u8 mic_enc_key_press_mute_en_get(void)
{
    return mic_enc.key_press_mute_en;
}


AT(.text.mic_emit.proc)
void mic_enc_cache_proc(s16 *pcm, uint samples)
{
    u8 *cache_in_buf = mic_enc.enc_cache_buf + mic_enc.enc_cache_cnt*MIC_DEC_OBUF_SIZE;
    u8 *cache_out_buf = mic_enc.enc_cache_buf + enc_cache_rptr*MIC_DEC_OBUF_SIZE;
    if (mic_enc.key_press_mute_en) {
        memset(mic_enc.enc_cache_buf, 0x00, MIC_DEC_OBUF_SIZE*MIC_CACHE_BUF_NUM);
    }
    memcpy(cache_in_buf, (u8 *)pcm, samples*2);
    memcpy((u8 *)pcm, cache_out_buf, samples*2);

    mic_enc.enc_cache_cnt++;
    if (mic_enc.enc_cache_cnt >= MIC_CACHE_BUF_NUM) {
        mic_enc.enc_cache_cnt = 0;
    }
    enc_cache_rptr++;
    if (enc_cache_rptr >= MIC_CACHE_BUF_NUM) {
        enc_cache_rptr = 0;
    }
}
#endif

#if WARNING_MIC_MIX_WAV_PLAY_EN
//-------------------------------------------------------------------------
//PCM(WAV)资源文件混到audio数据流中一起播放出来
//提示音是16K的单声道WAV(PCM)数据, audio是48K的,一个PCM数据需要混到3个audio数据上
//需要播放提示音的地方,直接调用 mic_mix_play_wav就行
//mic_mix_play_wav(RES_BUF_MAX_VOL_WAV,RES_LEN_MAX_VOL_WAV);
enum  {
    KMIX_STOP = 0,
    KMIX_RUN,
};
#define KMIX_BUF_SIZE   240
typedef struct _KMIX_T {
    s16 *buf;
    u16 samples_total;
    u16 samples_idx;
    u8 div_cnt;
    volatile u8 sta;
    // 乒乓缓冲区
    s16 pcm_buf[2][KMIX_BUF_SIZE/2];  // 120字节 = 60个16位样本
    u8 curr_buf;
    u16 buf_pos;
    // 同步标志
    volatile u8 buf_ready;  // 缓冲区是否准备好
    volatile u8 buf_used;   // 缓冲区是否被使用
} KMIX_T;

KMIX_T kmix;

//WAV资源文件混到KARAOK_MIC数据流中一起播放出来
void mic_mix_play_wav(u32 addr, u32 len)
{
    //printf("pcm_len = %d\n",pcm_len);
    if (len < 44) {
        return;
    }
    len = len - 44;
    kmix.sta = KMIX_STOP;
    kmix.buf = (s16*)(addr + 44);                  //wav 有44byte 头信息
//    kmix.samples_total = ((u16)len-256)/2;         //资源文件中的数据以256BYTE对齐,减去最后256BYTE,防止取到无效的对齐数据
    kmix.samples_total = len/2 - 256;
    kmix.samples_idx = 0;
    kmix.div_cnt = 0;
    kmix.curr_buf = 0;
    kmix.buf_pos = 0;
    kmix.buf_ready = 0;  // 重置缓冲区就绪标志
    kmix.buf_used = 0;   // 重置缓冲区使用标志
    kmix.sta  = KMIX_RUN;
}
// 处理提示音混合的函数，在主循环中调用
AT(.text.func.mic_emit)
void mic_mix_process(void)
{
    if (KMIX_RUN == kmix.sta) {
        if (!kmix.buf_ready || kmix.buf_used) {
            // printf(test_print3, kmix.samples_idx, kmix.samples_total, kmix.curr_buf, kmix.buf_pos);
            while (kmix.buf_pos < KMIX_BUF_SIZE/2) {
                if (kmix.samples_idx < kmix.samples_total) {
                    kmix.pcm_buf[kmix.curr_buf][kmix.buf_pos] = kmix.buf[kmix.samples_idx];
                    kmix.buf_pos++;
                    kmix.samples_idx++;
                } else {
                    while (kmix.buf_pos < KMIX_BUF_SIZE/2) {
                        kmix.pcm_buf[kmix.curr_buf][kmix.buf_pos] = 0;
                        kmix.buf_pos++;
                    }
                    kmix.sta = KMIX_STOP;
                    kmix.buf_ready = 1;
                    kmix.buf_used = 0;
                    break;
                }
            }
        }
        if (kmix.buf_pos >= KMIX_BUF_SIZE/2) {
            kmix.curr_buf = 1 - kmix.curr_buf;
            kmix.buf_pos = 0;
            kmix.buf_ready = 1;
            kmix.buf_used = 0;
        }
    }
}

AT(.com_text.play.proc)
const s16* mic_mix_get_current_buf(void)
{
    if (KMIX_RUN == kmix.sta && kmix.buf_ready) {
        return kmix.pcm_buf[1 - kmix.curr_buf];
    }
    return NULL;
}

AT(.com_text.play.proc)
s16 mic_mix_play_weak(s16 ptr)
{
    static u16 mix_idx = 0;
    s16 mic_data = ptr;

    if (KMIX_STOP == kmix.sta) {
        mix_idx = 0;
        return mic_data;
    }
    const s16* mix_buf = mic_mix_get_current_buf();
    if (mix_buf) {
        s32 mix_data = mic_data + mix_buf[mix_idx];

        if (mix_data > 32767) {
            mix_data = 32767;
        } else if (mix_data < -32768) {
            mix_data = -32768;
        }

        kmix.div_cnt++;
        if (kmix.div_cnt >= 3) {  // PCM提示音是16K的, audio是48K的,一个PCM数据需要混到3个audio数据上
            kmix.div_cnt = 0;
            mix_idx++;
            if (mix_idx >= KMIX_BUF_SIZE/2) {
                mix_idx = 0;
                kmix.buf_used = 1;
            }
        }
        return (s16)mix_data;
    }

    return mic_data;
}
#endif

AT(.text.mic_emit.proc)
void mic_enc_proc_cb(s16 *pcm, uint samples)
{
    bool mute_flag = mic_enc.mute_en;
    if (bsp_sdadc_discard_proc()) {
        mute_flag = true;
    }
#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
    mic_enc_cache_proc(pcm, samples);
    if (mic_enc.key_press_mute_en) {
        memset(pcm, 0x00, samples*2);
    }
#endif

    if(mute_flag) {
        memset(pcm, 0x00, samples*2);
    }
//    GPIOASET = BIT(8);
    if(sys_cb.mic_alg_en) {

#if WIRELESS_MIC_AINS4_32K_EN
        ains4_32k_mic_audio_input((u8 *)pcm, samples, 1, NULL);
#endif

#if WIRELESS_MIC_ECHO_EN && WIRELESS_MIC_32K_EN
        echo_audio_process(pcm, samples);
#endif

#if WIRELESS_MIC_32K_EN
        samples = src_frame_resample(0,(short *)pcm, (short *)pcm_48k, samples);
        pcm = pcm_48k;
#endif

#if WIRELESS_MAX_POWER_DETECT_EN
        dnr_buf_maxpow(pcm,samples);
#endif

#if WIRELESS_MIC_EQ_DRC_EN
        mic_eq_drc_audio_input((u8 *)pcm, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        dnr_fre_mic_audio_input((u8 *)pcm, WIRELESS_MIC_SAMPLES_SELECT, 1, NULL);
#endif

#if WIRELESS_MIC_ECHO_EN && !WIRELESS_MIC_32K_EN
        echo_audio_process(pcm, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_MAGIC_EN
        magic_audio_process(pcm, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_ROBOTIZATION_EN
        robotization_mic_audio_input(pcm, WIRELESS_MIC_SAMPLES_SELECT, 1, NULL);
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
        room_reverb_audio_process(pcm, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_AGC_EN
        agc_audio_input(pcm, WIRELESS_MIC_SAMPLES_SELECT, 0, NULL);
#endif

#if DUMP_HUART
        bsp_huart_dump_audio(pcm,WIRELESS_MIC_SAMPLES_SELECT);
#endif
    }
#if WARNING_MIC_MIX_WAV_PLAY_EN
    s16 *rptr = (s16 *)pcm;
    for(int i = 0; i < samples; i++) {
       rptr[i] = mic_mix_play_weak(rptr[i]);              //WAV提示音混合进音频中
    }
#endif // WARNING_MIC_MIX_WAV_PLAY_EN
#if WIRELESS_CON_CODEC_SEL == CODEC_SBC
    sbc_enc(pcm, mic_enc.buffer, WIRELESS_MIC_SAMPLES_SELECT);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ESBC
    esbc_enc(pcm, mic_enc.buffer, WIRELESS_MIC_SAMPLES_SELECT);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ADPCM
    adpcm_enc(pcm, mic_enc.buffer, WIRELESS_MIC_SAMPLES_SELECT);
#elif  (WIRELESS_CON_CODEC_SEL == CODEC_LC3S)
    lc3s_enc((s16 *)pcm, mic_enc.buffer, WIRELESS_MIC_SAMPLES_SELECT);
//    memset(mic_enc.buffer, 0x00, 25);
#endif

    wireless_d2a_put_tx_frame(mic_enc.buffer, WIRELESS_MIC_FRAME_SIZE);
//    GPIOACLR = BIT(8);

    wl_play_sync_tick1(WIRELESS_MIC_TX_INTERVAL*2/WIRELESS_MIC_COMB_NB, SAMPLE_RATE_48K);

#if WIRELESS_MIC_DAC_OUT_EN
    dac0_get_fifocnt(0);
    mic_enc.dac_cnt++;
    if (mic_enc.dac_cnt > 50) {
        mic_enc.dac_cnt = 0;
        dac0_play_sync_fifocnt(50, 30);
    }
    dac0_out_audio_input((u8 *)pcm, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif

}
#endif
#if !ADAPTER_CLOSE_EN
AT(.text.adapter.proc)
void mic_dec_dac_fifo_get(u8 idx)
{
    mic_dec.sync_idx = idx;
    dac0_get_fifocnt(0);
}
AT(.text.adapter.proc)
void mic_dec_dac_dma_kick(uint tick_cnt)
{
}

#if WIRELESS_CON_COMB_BUF_EN
AT(.text.adapter.proc)
void mic_dec_pcm_out(u8 idx)
{
    s16 *obuf = (s16 *)mic_dec.obuf;
    u8 samples = WIRELESS_MIC_SAMPLES_SELECT/2;

    mic_dec.pcm[idx].buf_cnt = 0;                   //复位pcm[idx].buf_cnt，确保先copy前半段pcm[idx].buf

    s16 *pcm0 = (s16 *)(mic_dec.pcm[0].buf + mic_dec.pcm[0].buf_cnt);
    s16 *pcm1 = (s16 *)(mic_dec.pcm[1].buf + mic_dec.pcm[1].buf_cnt);

#if ADAPTER_MIX_DRC_EN
    mix_drc_audio_input(pcm0, pcm1, obuf, samples);
#else
    for(uint i=0; i<samples; i++) {
        obuf[i] = pcm0[i]/2 + pcm1[i]/2;
    }
#endif

    mic_dec.pcm[0].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[0].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[0].buf_cnt = 0;
    }
    mic_dec.pcm[1].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[1].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[1].buf_cnt = 0;
    }

    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)obuf, samples, 1, NULL);
    }
}
#else // WIRELESS_CON_COMB_BUF_EN

AT(.text.adapter.proc)
static uint16_t mic_dec_us_trans_samples(uint16_t us, uint8_t spr_idx)
{
    uint16_t samples_duration = 0;

    switch(spr_idx) {
        ///对应48k采样率的情况，一个样点对应20.8us
        case SAMPLE_RATE_48K:
            samples_duration = 21;
            break;

        ///对应16k采样率的情况，一个样点对应62.5us
        case SAMPLE_RATE_16K:
            samples_duration = 62;
            break;

        default:
            samples_duration = 21;
            break;
    }

    return (us/samples_duration);
}

AT(.text.adapter.proc)
static void mic_dec_pcm_out_samples(s16 *pcm0, s16 *pcm1, uint8_t samples, uint8_t adj_dac_flag)
{
    s16 *obuf = (s16 *)mic_dec.obuf;

#if ADAPTER_MIX_DRC_EN
    mix_drc_audio_input(pcm0, pcm1, (obuf+mic_dec.obuf_off), samples);
#else
    for(uint i=0; i<(samples); i++) {
        obuf[i+mic_dec.obuf_off] = pcm0[i]/2 + pcm1[i]/2;
    }
#endif

#if ADAPTER_DAC_OUTPUT_EN
    dac0_out_audio_input((u8 *)(obuf+mic_dec.obuf_off), samples, 0);
#endif
    mic_dec.obuf_off += samples;

    if (mic_dec.obuf_off == WIRELESS_MIC_SAMPLES_SELECT) {
        mic_dec.obuf_off = 0;
        ///在这里推dac/usb输出
#if ADAPTER_USB_MIC_RX_EN
        usb_mic_in_audio_input((u8 *)obuf, WIRELESS_MIC_SAMPLES_SELECT,0,0);
#endif
    }
}

AT(.text.adapter.proc)
static void mic_dec_pcm_out_frag0(u8 idx, u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];

    s16 *pcm0 = (s16 *)(&mic_dec.pcm[0].buf[0]);
    s16 *pcm1 = (s16 *)(&mic_dec.pcm[1].buf[0]) + (WIRELESS_MIC_SAMPLES_SELECT-frag0_samples);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag0_samples, dec_flag);
}

//format pcm / upsample / mix
AT(.text.adapter.proc)
static void mic_dec_pcm_out_frag1(u8 idx, u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];
    uint8_t frag1_samples = mic_dec.frag_samples[1];

    s16 *pcm0 = (s16 *)(&mic_dec.pcm[0].buf[0]) + frag0_samples;
    s16 *pcm1 = (s16 *)(&mic_dec.pcm[1].buf[0]);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag1_samples, dec_flag);
}

AT(.text.adapter.proc)
static void mic_dec_pcm_out(u8 idx)
{
#if (WIRELESS_CON_LINK_NB == 1)
    //输出到下一级
#if ADAPTER_USB_MIC_RX_EN
    usb_mic_in_audio_input((u8 *)(&mic_dec.pcm[0].buf[0]), WIRELESS_MIC_SAMPLES_SELECT,0,0);
#endif
#if ADAPTER_DAC_OUTPUT_EN
    dac0_out_audio_input((u8 *)(&mic_dec.pcm[0].buf[0]), WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif
#else
    u8 con_status;
    u8 dec_flag = 0;

    if(idx == 0) {
        if (mic_dec.frag0_done_flag == 0) {
            //推通道1前半帧+通道2后半帧
            mic_dec_pcm_out_frag0(idx, dec_flag);
            mic_dec.frag0_done_flag = 1;
        }

        //若通道2未连接，把frag1也转换了
        con_status = ble_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag1(idx, 0);
            mic_dec.frag0_done_flag = 0;
        }
    } else {
        //通道2解码完，再转换frag1，输出到OBUF后半段
        if(mic_dec.frag0_done_flag) {
            mic_dec_pcm_out_frag1(idx, dec_flag);
            mic_dec.frag0_done_flag = 0;
        }

        //若通道1未连接，把frag0也转换了
        con_status = ble_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag0(idx, 0);
            mic_dec.frag0_done_flag = 1;
        }
    }
#endif
}
#endif // WIRELESS_CON_COMB_BUF_EN

AT(.text.adapter.proc)
void mic_dec_prco_cb(u8 idx)
{
#if WARNING_TONE_EN
    //mp3 提示音功能使用过程中pass解码流程
    if(res_music_playing_state_get()){
        return;
    }
#endif

    s16 *pcm = (s16 *)&mic_dec.pcm[idx].buf;
    u32 samples = WIRELESS_MIC_SAMPLES_SELECT;

    if(sys_cb.mic_alg_en) {
        u8 con_status = ble_con_get_status();
        if(con_status & BIT(idx)) {
            bool bfi = wireless_d2a_get_rx_frame(idx, mic_dec.frame, WIRELESS_MIC_FRAME_SIZE);

            if(!bfi) {
#if WIRELESS_CON_CODEC_SEL == CODEC_SBC
                sbc_dec(mic_dec.frame, pcm, samples, idx);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ESBC
                esbc_dec(mic_dec.frame, pcm, samples, idx);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ADPCM
                adpcm_dec(mic_dec.frame, pcm, samples, idx);
#elif WIRELESS_CON_CODEC_SEL == CODEC_LC3S
                lc3s_dec(mic_dec.frame, pcm, samples, idx);
#endif
            }

            plc_soft_process(pcm, samples, mic_dec.last_bfi[idx]||bfi, idx);
            mic_dec.last_bfi[idx] = bfi;
        }
    } else {
        memset(pcm, 0x00, samples*2);
    }

    mic_dec_pcm_out(idx);

    if(idx == mic_dec.sync_idx) {
        //DAC调速，避免长时间后播放速度和发射端不匹配
        mic_dec_dac_sync_proc();
    }

//    GPIOASET = BIT(8);
}
#endif

void mic_emit_init(void)
{
#if !WIRELESS_MIC_CLOSE_EN
    mic_emit_alg_init();

    memset(&mic_enc, 0x00, sizeof(mic_enc));
    mic_enc.first_pkt = true;
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_mic_mute_level_read((u8 *)&mic_enc.mute_en);
#endif
#if WIRELESS_MAX_POWER_DETECT_EN
    mic_max_power_detect_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif
#if ADAPTER_SYNC_PARAM_EN
    ///adapter控制参数时，先mute住发射端的音频，等待adapter同步参数后用新的参数决定出不出声音
    mic_enc.mute_en = 1;
#endif
#if WIRELESS_CON_CODEC_SEL == CODEC_SBC
    sbc_enc_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ESBC
    esbc_enc_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ADPCM
    adpcm_enc_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#elif (WIRELESS_CON_CODEC_SEL == CODEC_LC3S)
    lc3s_enc_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
    allpass_filter_change_set(0,100);
#endif

#if WIRELESS_MIC_32K_EN
    src_init(0,32000,48000);
#endif

#if WIRELESS_MIC_AINS4_32K_EN
    ains4_32k_mic_init(0, WIRELESS_MIC_SAMPLES_SELECT, 1);
#endif

#if WIRELESS_MIC_ECHO_EN
    echo_audio_init(0, WIRELESS_MIC_SAMPLES_SELECT, 1);
#endif

#if WIRELESS_MIC_MAGIC_EN
    magic_audio_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_ROBOTIZATION_EN
    robotization_mic_init(0, WIRELESS_MIC_SAMPLES_SELECT, 1);
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
    room_reverb_audio_init_cfg(0, WIRELESS_MIC_SAMPLES_SELECT);
    room_reverb_audio_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_EQ_DRC_EN
    mic_eq_drc_init(0, WIRELESS_MIC_SAMPLES_SELECT, 1);
#endif

#if WIRELESS_MIC_AGC_EN
    agc_audio_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_DNR_FRE_EN
    dnr_fre_mic_init(0, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif

#if WIRELESS_MIC_DAC_OUT_EN
    dac0_out_init(0, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif

    bsp_sdadc_init();

#if WIRELESS_MIC_LOWPASS_EN
    bsp_sdadc_lowpass_filter_en();
#endif
#endif
}

void mic_emit_reset(void)
{
#if !WIRELESS_MIC_CLOSE_EN
    bsp_sdadc_exit();
//    bsp_sdadc_init();
    wireless_cmd_reset(0);
    memset(&mic_enc, 0x00, sizeof(mic_enc));
#if WIRELESS_MIC_PARAM_MEMORY_EN
    param_mic_mute_level_read((u8 *)&mic_enc.mute_en);
#endif

#if WIRELESS_MIC_DAC_OUT_EN
    dac0_out_exit();
#endif
#endif
}

void mic_dec_reset(u8 idx)
{
    mic_dec.last_bfi[idx] = 0;
    memset(mic_dec.pcm[idx].buf, 0, MIC_DEC_OBUF_SIZE);
}

void adapter_init(void)
{
#if !ADAPTER_CLOSE_EN
    adapter_alg_init();

    memset(&mic_dec, 0x00, sizeof(mic_dec));
#if WIRELESS_CON_LINK_NB > 1 && !WIRELESS_CON_COMB_BUF_EN
    int temp_frag = mic_dec_us_trans_samples(ble_single_link_duration_get(), WIRELESS_MIC_SAMPLE_RATE_SELECT)%WIRELESS_MIC_SAMPLES_SELECT;
    //寻址WORD对齐
    mic_dec.frag_samples[0] = ((temp_frag%2) ? temp_frag+1 : temp_frag);//mic_dec_us_trans_samples(ble_single_link_duration_get(), WIRELESS_MIC_SAMPLE_RATE_SELECT)%WIRELESS_MIC_SAMPLES_SELECT;
    mic_dec.frag_samples[1] = WIRELESS_MIC_SAMPLES_SELECT - mic_dec.frag_samples[0];
#endif
#if WIRELESS_CON_CODEC_SEL == CODEC_SBC
    sbc_dec_init(0, 0);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ESBC
    esbc_dec_init(0, 0);
#elif WIRELESS_CON_CODEC_SEL == CODEC_ADPCM
    adpcm_dec_init(0, 0);
#elif WIRELESS_CON_CODEC_SEL == CODEC_LC3S
    lc3s_dec_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif
#if ADAPTER_MIX_DRC_EN
    mix_drc_init();
#endif
#if ADAPTER_HUART_AUDIO_OUTPUT_EN
    huart_audio_output_init();
#endif

#if ADAPTER_DAC_OUTPUT_EN
    dac0_out_init(0, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif

    wireless_host_ch_class_set();
#endif
}

void adapter_reset(u8 idx, u8 con_sta)
{
#if !ADAPTER_CLOSE_EN
    mic_dec_reset(idx);
    plc_soft_exit(idx);
    wireless_cmd_reset(idx);
    m_lc3s_dec_exit(idx);
    if(con_sta == 0) {
        mic_dec.obuf_off = 0;
    #if ADAPTER_DAC_OUTPUT_EN
        dac0_out_exit();
    #endif
    }
#endif
}

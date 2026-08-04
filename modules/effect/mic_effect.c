/*
 * 文件名称: mic_effect.c
 * 功能描述: 复用PACC0模块，处理EQ/DRC等音效
 *
 * 注意事项：PACC0能分时处理不同cs链路，但要避免多线程同时调用pacc_ctl_proc_cs
 *
 *                  复用场景
 *      music/spk   /   a2dp        /   sco
 *      loc_mic     /   loc_mic     /
 *      loc_mic     /   mix_mic     /
 *                                  /   sco_mic
 *
 */

#include "include.h"
#include "effect_idx.h"
#include "mic_effect.h"

enum {
    PACC0_LOC_MIC   = BIT(0),
    PACC0_MIX_MIC   = BIT(1),
    PACC0_SCO_MIC   = BIT(2),
};

static struct {
    void *ctl;
    u8 enable;
} pacc0_hw = {
    .ctl        = NULL,
    .enable     = 0,
};

void *pacc0_ctl_alloc(uint en_bit)
{
    if(pacc0_hw.ctl == NULL) {
        pacc0_hw.ctl = pacc_effect_init(0);
    }
    pacc0_hw.enable |= en_bit;

    return pacc0_hw.ctl;
}

void pacc0_ctl_free(uint en_bit)
{
    pacc0_hw.enable &= ~en_bit;
    if(pacc0_hw.enable == 0) {
        pacc0_hw.ctl = NULL;
        pacc_effect_exit(0);
    }
}

#if WIRELESS_MIC_EQ_DRC_EN
//------------------------------------------------------------------------------------------
//本地麦EQ/DRC链路
#define LOC_PACC_FRAME_LEN		                120                 //算法处理帧长

static struct {
    u8 cache[LOC_PACC_FRAME_LEN*4];                                 //pacc中间缓存
    void *pacc_ctl;                                                 //pacc硬件模块的控制句柄
    pacc_cs_t *start_cs;                                            //pacc链表处理的表头
    pacc_cs_t pacc_cs[LOC_MIC_PACC_MAX_CS];                         //pacc_cs列表
    u8 cs_en[LOC_MIC_PACC_MAX_CS];                                  //cs列表使能标志
} loc_mic_pacc AT(.buf.mic_effect.pacc);

static pacc_eq_t loc_mic_eq AT(.buf.mic_effect);
static pacc_eq_bd_t loc_mic_eq_bd AT(.buf.mic_effect);
static pacc_drc_t loc_mic_drc AT(.buf.mic_effect);
static pacc_drc_bd_t loc_mic_drc_bd AT(.buf.mic_effect);

#if WIRELESS_FREQ_SHIFT2_EN
static freq_shift_bd_t mic_fsh AT(.buf.mic_effect);
#endif

static const pacc_effect_cb loc_mic_pacc_cb_tbl[LOC_MIC_PACC_MAX_CS] = {
    {LOC_MIC_PACC_EQ_CS,   PACC_EQ,    false, false,  loc_mic_pacc.cache,        S16_Q15_SAT, loc_mic_pacc.cache,  S32_Q23, &loc_mic_pacc.pacc_cs[LOC_MIC_PACC_EQ_CS],        &loc_mic_eq},
#if WIRELESS_FREQ_SHIFT2_EN
    {LOC_MIC_PACC_FSH_CS,  PACC_FSH,   false, false,  loc_mic_pacc.cache,        S32_Q23, loc_mic_pacc.cache,  S32_Q23,     &loc_mic_pacc.pacc_cs[LOC_MIC_PACC_FSH_CS],       &mic_fsh},
#endif
    {LOC_MIC_PACC_DRC_CS,  PACC_DRC,   false, false,  loc_mic_pacc.cache,        S32_Q23, loc_mic_pacc.cache,  S16_Q15_SAT, &loc_mic_pacc.pacc_cs[LOC_MIC_PACC_DRC_CS],       &loc_mic_drc},
};

void loc_mic_pacc_init(void)
{
    printf("%s\n", __func__);

    //temp ram init
    memset(loc_mic_pacc.cache, 0, sizeof(loc_mic_pacc.cache));

    //初始链路模块状态全部关闭
    memset(loc_mic_pacc.cs_en, 0x0, LOC_MIC_PACC_MAX_CS);

    //ram init
    pacc_effect_eq_init(&loc_mic_eq, &loc_mic_pacc.pacc_cs[LOC_MIC_PACC_EQ_CS], 0, EQ_BAND_MAX_NB, 0, &loc_mic_eq_bd.coef, EQ_COEF_BUF_SIZE, &loc_mic_eq_bd.zx, EQ_ZX_BUF_SIZE);
    pacc_effect_drc_init(&loc_mic_drc, &loc_mic_pacc.pacc_cs[LOC_MIC_PACC_DRC_CS], 0, 0, NULL, DRC_COEF_BUF_SIZE, &loc_mic_drc_bd.drc_coef, DRC_ZX_BUF_SIZE);
#if WIRELESS_FREQ_SHIFT2_EN
    memset(&mic_fsh, 0, sizeof(freq_shift_bd_t));
#endif

    loc_mic_pacc.pacc_ctl = pacc0_ctl_alloc(PACC0_LOC_MIC);  //麦使用PACC0

    loc_mic_pacc.start_cs = pacc_effect_link((void*)loc_mic_pacc_cb_tbl, LOC_MIC_PACC_MAX_CS, LOC_PACC_FRAME_LEN);

}

void loc_mic_pacc_set_param(void)
{
    u32 drc_addr = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_DRC);
    u32 drc_len = effect_res_len_get(EFFECT_IDX_LOC_MIC_DRC);
    u32 eq_addr = (u32)effect_res_addr_get(EFFECT_IDX_LOC_MIC_EQ);
    u32 eq_len  = effect_res_len_get(EFFECT_IDX_LOC_MIC_EQ);

    if(pacc_drc_set_by_res(&loc_mic_pacc.pacc_cs[LOC_MIC_PACC_DRC_CS], drc_addr, drc_len) == ERROR_NO) {
        loc_mic_pacc.cs_en[LOC_MIC_PACC_DRC_CS] = 1;
    }

    if (pacc_eq_set_by_res(&loc_mic_pacc.pacc_cs[LOC_MIC_PACC_EQ_CS], eq_addr, eq_len) == ERROR_NO) {
        loc_mic_pacc.cs_en[LOC_MIC_PACC_EQ_CS] = 1;
    }

#if WIRELESS_FREQ_SHIFT2_EN
    loc_mic_pacc.cs_en[LOC_MIC_PACC_FSH_CS] = 1;
#endif
}

void loc_mic_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param)
{
    pacc_eq_set_by_param(&loc_mic_pacc.pacc_cs[LOC_MIC_PACC_EQ_CS], band_cnt, eq_param);
}

void loc_mic_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param)
{
    if ( pacc_drc_set_by_param(&loc_mic_pacc.pacc_cs[LOC_MIC_PACC_DRC_CS], band_cnt, drc_param) == ERROR_NO) {

    } else {

    }
}

bool loc_mic_pacc_enable(void)
{
    printf("%s\n", __func__);

    //根据使能的链路，重新链接cs链路
    loc_mic_pacc.start_cs = pacc_effect_relink(loc_mic_pacc.pacc_cs, loc_mic_pacc.cs_en, LOC_MIC_PACC_MAX_CS);

    //重新设置链头和链尾的数据格式
#if WIRELESS_MIC_24B_PCM_EN
    pacc_cs_list_set_io_fmt(loc_mic_pacc.start_cs, false, S32_Q23_SAT, S32_Q23_SAT);
#else
    #if WIRELESS_MIC_SOFT_GAIN_EN
    pacc_cs_list_set_io_fmt(loc_mic_pacc.start_cs, false, S32_Q15, S16_Q15_SAT);
    #else
    pacc_cs_list_set_io_fmt(loc_mic_pacc.start_cs, false, S16_Q15_SAT, S16_Q15_SAT);
    #endif
#endif



//    pacc_dump_cs_list(loc_mic_pacc.start_cs);

    return (loc_mic_pacc.start_cs != NULL);
}

AT(.com_text.loc_mic)
void loc_mic_pacc_process(u8 *obuf, u8 *ibuf, u32 samples)
{

    while(samples > 0) {
        u32 size = (samples > LOC_PACC_FRAME_LEN)? LOC_PACC_FRAME_LEN : samples;
#if WIRELESS_MIC_SOFT_GAIN_EN
        s32* wptr =  (s32* )loc_mic_pacc.cache;
        s16* rptr =  (s16* )ibuf;
        for (int i=0; i<size; i++) {
            wptr[i] = (s32)soft_gain_proc_16bit(rptr[i]);
        }
        //输入调整回cache
        pacc_cs_list_set_io_addr_fsize(loc_mic_pacc.start_cs, obuf, loc_mic_pacc.cache, size);
#else
        pacc_cs_list_set_io_addr_fsize(loc_mic_pacc.start_cs, obuf, ibuf, size);
#endif
        pacc_ctl_proc(loc_mic_pacc.pacc_ctl, loc_mic_pacc.start_cs, size);

        samples -= size;
        ibuf += sizeof(mic_pcm_t)*size;
        obuf += sizeof(mic_pcm_t)*size;
    }
}

void loc_mic_pacc_exit(void)
{
    loc_mic_pacc.pacc_ctl = NULL;
    pacc0_ctl_free(PACC0_LOC_MIC);               //麦使用PACC0
}
#endif

#if ADAPTER_MIX_DRC_EN
//------------------------------------------------------------------------------------------
//无线麦混音后EQ/DRC等链路
#define MIX_PACC_FRAME_LEN		                120                 //算法处理帧长
s32 mix_temp_buf[MIX_PACC_FRAME_LEN] AT(.buf.mic_mix);

static struct {
    void *pacc_ctl;                                                 //pacc硬件模块的控制句柄
    pacc_cs_t *start_cs;                                            //pacc链表处理的表头
    pacc_cs_t pacc_cs[MIX_MIC_PACC_MAX_CS];                        //pacc_cs列表
    u8 cs_en[MIX_MIC_PACC_MAX_CS];                                 //cs列表使能标志
//    u8 cache[MIX_PACC_FRAME_LEN*4];                                 //pacc中间缓存
} mix_pacc AT(.buf.mic_mix.pacc);

static pacc_eq_t mix_eq AT(.buf.mic_mix);
static pacc_eq_bd_t mix_eq_bd AT(.buf.mic_mix);
static pacc_drc_t mix_drc AT(.buf.mic_mix);
static pacc_drc_bd_t mix_drc_bd AT(.buf.mic_mix);

#if ADAPTER_FREQ_SHIFT_EN
static freq_shift_bd_t mix_fsh AT(.buf.mic_mix);
#endif

static const pacc_effect_cb mix_pacc_cb_tbl[MIX_MIC_PACC_MAX_CS] = {
    {MIX_MIC_PACC_EQ_CS,    PACC_EQ,    false, false,  mix_temp_buf,        S32_Q23, mix_temp_buf,  S32_Q23,     &mix_pacc.pacc_cs[MIX_MIC_PACC_EQ_CS],        &mix_eq},
#if ADAPTER_FREQ_SHIFT_EN
    {MIX_MIC_PACC_FSH_CS,   PACC_FSH,   false, false,  mix_temp_buf,        S32_Q23, mix_temp_buf,  S32_Q23,     &mix_pacc.pacc_cs[MIX_MIC_PACC_FSH_CS],       &mix_fsh},
#endif
    {MIX_MIC_PACC_DRC_CS,   PACC_DRC,   false, false,  mix_temp_buf,        S32_Q23, mix_temp_buf,  S32_Q23,     &mix_pacc.pacc_cs[MIX_MIC_PACC_DRC_CS],       &mix_drc},

};

void mix_pacc_init(void)
{
    printf("%s\n", __func__);

    //temp ram init
//    memset(mix_pacc.cache, 0, sizeof(mix_pacc.cache));

    //初始链路模块状态全部关闭
    memset(mix_pacc.cs_en, 0x0, MIX_MIC_PACC_MAX_CS);

    //ram init
    pacc_effect_eq_init(&mix_eq, &mix_pacc.pacc_cs[MIX_MIC_PACC_EQ_CS], 0, EQ_BAND_MAX_NB, 0, &mix_eq_bd.coef, EQ_COEF_BUF_SIZE, &mix_eq_bd.zx, EQ_ZX_BUF_SIZE);
    pacc_effect_drc_init(&mix_drc, &mix_pacc.pacc_cs[MIX_MIC_PACC_DRC_CS], 0, 0, NULL, DRC_COEF_BUF_SIZE, &mix_drc_bd.drc_coef, DRC_ZX_BUF_SIZE);
#if ADAPTER_FREQ_SHIFT_EN
    memset(&mix_fsh, 0, sizeof(freq_shift_bd_t));
#endif

    mix_pacc.pacc_ctl = pacc0_ctl_alloc(PACC0_MIX_MIC);  //麦使用PACC0
    mix_pacc.start_cs = pacc_effect_link((void*)mix_pacc_cb_tbl, MIX_MIC_PACC_MAX_CS, MIX_PACC_FRAME_LEN);
}

void mix_pacc_set_param(void)
{
    u32 drc_addr = (u32)effect_res_addr_get(EFFECT_IDX_MIX_DRC);
    u32 drc_len = effect_res_len_get(EFFECT_IDX_MIX_DRC);
    u32 eq_addr = (u32)effect_res_addr_get(EFFECT_IDX_MIX_EQ);
    u32 eq_len  = effect_res_len_get(EFFECT_IDX_MIX_EQ);

    if (pacc_eq_set_by_res(&mix_pacc.pacc_cs[MIX_MIC_PACC_EQ_CS], eq_addr, eq_len) == ERROR_NO) {
        mix_pacc.cs_en[MIX_MIC_PACC_EQ_CS] = 1;
    }

    if(pacc_drc_set_by_res(&mix_pacc.pacc_cs[MIX_MIC_PACC_DRC_CS], drc_addr, drc_len) == ERROR_NO) {
        mix_pacc.cs_en[MIX_MIC_PACC_DRC_CS] = 1;
    }

#if ADAPTER_FREQ_SHIFT_EN
    mix_pacc.cs_en[MIX_MIC_PACC_FSH_CS] = 1;
#endif
}

void mix_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param)
{
    pacc_eq_set_by_param(&mix_pacc.pacc_cs[MIX_MIC_PACC_EQ_CS], band_cnt, eq_param);
}

void mix_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param)
{
    pacc_drc_set_by_param(&mix_pacc.pacc_cs[MIX_MIC_PACC_DRC_CS], band_cnt, drc_param);
}

void mix_pacc_enable(void)
{
    printf("%s\n", __func__);

    //根据使能的链路，重新链接cs链路
    mix_pacc.start_cs = pacc_effect_relink(mix_pacc.pacc_cs, mix_pacc.cs_en, MIX_MIC_PACC_MAX_CS);

    //重新设置链头和链尾的数据格式
#if WIRELESS_MIC_24B_PCM_EN
    pacc_cs_list_set_io_fmt(mix_pacc.start_cs, false, S32_Q23, S32_Q23_SAT);
#else
    pacc_cs_list_set_io_fmt(mix_pacc.start_cs, false, S32_Q15, S16_Q15_SAT);
#endif
}

AT(.text.mic_mix_proc)
void mix_pacc_process(mic_pcm_t *obuf, mic_pcm_t *ibuf0, mic_pcm_t *ibuf1, u32 samples)
{
    mic_pcm_t *out = (obuf == NULL)? ibuf0 : obuf;

    while(samples > 0) {
        u32 size = (samples > MIX_PACC_FRAME_LEN)? MIX_PACC_FRAME_LEN : samples;

        for(uint i=0; i<size; i++) {
            mix_temp_buf[i] = ibuf0[i] + ibuf1[i];
        }
        pacc_cs_list_set_io_addr_fsize(mix_pacc.start_cs, out, mix_temp_buf, size);
        pacc_ctl_proc(mix_pacc.pacc_ctl, mix_pacc.start_cs, size);

        samples -= size;
        ibuf0 += size;
        ibuf1 += size;
        out += size;
    }
}

void mix_pacc_exit(void)
{
    mix_pacc.pacc_ctl = NULL;
    pacc0_ctl_free(PACC0_MIX_MIC);           //麦使用PACC0
}
#endif // ADAPTER_MIX_DRC_EN

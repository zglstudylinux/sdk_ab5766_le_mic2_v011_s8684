#include "include.h"
#include "effect.h"

const effect_update_callback_t effect_update_callback_tbl[CFG_MAX] = {
	[CFG_MIC_EQ]   = {local_mic_eq_effect_update_callback},
	[CFG_MIC_DRC]  = {local_mic_drc_effect_update_callback},
	[CFG_ECHO]     = {mic_echo_effect_update_callback},
	[CFG_MAGIC]    = {mic_magic_effect_update_callback},
	[CFG_MIX_EQ]   = {mix_mic_eq_effect_update_callback},
	[CFG_MIX_DRC]  = {mix_mic_drc_effect_update_callback},
};

const u8 effect_offset_tbl[EFFECT_IDX_MAX_NB] = {
	[EFFECT_IDX_LOC_MIC_EQ]     = CFG_MIC_EQ,
	[EFFECT_IDX_LOC_MIC_DRC]    = CFG_MIC_DRC,
	[EFFECT_IDX_LOC_MIC_ECHO]   = CFG_ECHO,
	[EFFECT_IDX_LOC_MIC_MAGIC]  = CFG_MAGIC,
	[EFFECT_IDX_MIX_EQ]         = CFG_MIX_EQ,
	[EFFECT_IDX_MIX_DRC]        = CFG_MIX_DRC,
};

u8 effect_info_offset_get(uint effect_idx)
{
    if(effect_idx >= EFFECT_IDX_MAX_NB) {
        return CFG_MAX;
    }

    return effect_offset_tbl[effect_idx];
}

AT(.text.effect_res)
u8 *effect_res_addr_get(uint effect_idx)
{
    u8 res_offset;

    if(effect_idx > EFFECT_IDX_MAX_NB) {
        return NULL;
    }

    //EFFECT_IDX_MAX_NB 返回的是完整bin文件地址/长度
    if(effect_idx == EFFECT_IDX_MAX_NB) {
        return (u8*)RES_BUF_EFFECT_EFFECT_BIN;
    }

    res_offset = effect_offset_tbl[effect_idx];
    if(res_offset >= CFG_MAX) {
        return NULL;
    }

    u32 effect_res_offset = effect_info[res_offset].effect_res_offset;
    u32 *res_inbin_addr = (u32*)((u8*) (RES_BUF_EFFECT_EFFECT_BIN) + effect_res_offset);
    u8 *res_real_addr  = (u8*)RES_BUF_EFFECT_EFFECT_BIN + *res_inbin_addr;

    return res_real_addr;
}

AT(.text.effect_res)
u32 effect_res_len_get(uint effect_idx)
{
    u8 res_offset;
    if(effect_idx > EFFECT_IDX_MAX_NB) {
        return 0;
    }

    //EFFECT_IDX_MAX_NB 返回的是完整bin文件地址/长度
    if(effect_idx == EFFECT_IDX_MAX_NB) {
        return RES_LEN_EFFECT_EFFECT_BIN;
    }

    res_offset = effect_offset_tbl[effect_idx];
    if(res_offset >= CFG_MAX) {
        return 0;
    }

    u32 effect_len_offset = effect_info[res_offset].effect_len_offset;
    u32  res_inbin_len  = *(u32*)((u8*) (RES_BUF_EFFECT_EFFECT_BIN) + effect_len_offset);

    return res_inbin_len;
}

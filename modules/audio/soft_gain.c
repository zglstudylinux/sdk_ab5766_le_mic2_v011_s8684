/*
 * 文件名称: soft_gain.c
 * 功能描述: 本文件为软件增益处理模块
 ****************************************************************************************
 */
#include "include.h"
#include "soft_gain.h"

#if WIRELESS_MIC_SOFT_GAIN_EN

static struct {
    uint8_t fade_en;                        //淡入淡出使能
    uint8_t step;                           //淡入淡出步进
    uint8_t gain_tbl_index;                 //tbl index
    uint32_t curr_gain;                     //当前gain换算的数字量
    uint32_t target_gain;                   //目标gain换算的数字量
} soft_gain;

#if (SOFT_GAIN_MAX_LEVEL == 64)
static const u32 soft_gain_tbl_64[SOFT_GAIN_MAX_LEVEL+1] = {
    SOFT_GAIN_N0DB,  SOFT_GAIN_N1DB,  SOFT_GAIN_N2DB,  SOFT_GAIN_N3DB,  SOFT_GAIN_N4DB,
    SOFT_GAIN_N5DB,  SOFT_GAIN_N6DB,  SOFT_GAIN_N7DB,  SOFT_GAIN_N8DB,  SOFT_GAIN_N9DB,
    SOFT_GAIN_N10DB, SOFT_GAIN_N11DB, SOFT_GAIN_N12DB, SOFT_GAIN_N13DB, SOFT_GAIN_N14DB,
    SOFT_GAIN_N15DB, SOFT_GAIN_N16DB, SOFT_GAIN_N17DB, SOFT_GAIN_N18DB, SOFT_GAIN_N19DB,
    SOFT_GAIN_N20DB, SOFT_GAIN_N21DB, SOFT_GAIN_N22DB, SOFT_GAIN_N23DB, SOFT_GAIN_N24DB,
    SOFT_GAIN_N25DB, SOFT_GAIN_N26DB, SOFT_GAIN_N27DB, SOFT_GAIN_N28DB, SOFT_GAIN_N29DB,
    SOFT_GAIN_N30DB, SOFT_GAIN_N31DB, SOFT_GAIN_N32DB, SOFT_GAIN_N33DB, SOFT_GAIN_N34DB,
    SOFT_GAIN_N35DB, SOFT_GAIN_N36DB, SOFT_GAIN_N37DB, SOFT_GAIN_N38DB, SOFT_GAIN_N39DB,
    SOFT_GAIN_N40DB, SOFT_GAIN_N41DB, SOFT_GAIN_N42DB, SOFT_GAIN_N43DB, SOFT_GAIN_N44DB,
    SOFT_GAIN_N45DB, SOFT_GAIN_N46DB, SOFT_GAIN_N47DB, SOFT_GAIN_N48DB, SOFT_GAIN_N49DB,
    SOFT_GAIN_N50DB, SOFT_GAIN_N51DB, SOFT_GAIN_N52DB, SOFT_GAIN_N53DB, SOFT_GAIN_N54DB,
    SOFT_GAIN_N55DB, SOFT_GAIN_N56DB, SOFT_GAIN_N57DB, SOFT_GAIN_N58DB, SOFT_GAIN_N59DB,
    SOFT_GAIN_N60DB, SOFT_GAIN_N65DB, SOFT_GAIN_N110DB, SOFT_GAIN_N110DB, SOFT_GAIN_N110DB
};
#elif (SOFT_GAIN_MAX_LEVEL == 32)
static const u32 soft_gain_tbl_32[SOFT_GAIN_MAX_LEVEL+1] = {
    SOFT_GAIN_N0DB,  SOFT_GAIN_N2DB,  SOFT_GAIN_N4DB,  SOFT_GAIN_N5DB,  SOFT_GAIN_N7DB,
    SOFT_GAIN_N9DB,  SOFT_GAIN_N10DB, SOFT_GAIN_N12DB, SOFT_GAIN_N14DB, SOFT_GAIN_N15DB,
    SOFT_GAIN_N17DB, SOFT_GAIN_N19DB, SOFT_GAIN_N20DB, SOFT_GAIN_N22DB, SOFT_GAIN_N24DB,
    SOFT_GAIN_N25DB, SOFT_GAIN_N27DB, SOFT_GAIN_N29DB, SOFT_GAIN_N30DB, SOFT_GAIN_N32DB,
    SOFT_GAIN_N34DB, SOFT_GAIN_N35DB, SOFT_GAIN_N37DB, SOFT_GAIN_N39DB, SOFT_GAIN_N40DB,
    SOFT_GAIN_N42DB, SOFT_GAIN_N44DB, SOFT_GAIN_N45DB, SOFT_GAIN_N47DB, SOFT_GAIN_N49DB,
    SOFT_GAIN_N50DB, SOFT_GAIN_N52DB, SOFT_GAIN_N110DB,
};
#elif (SOFT_GAIN_MAX_LEVEL == 16)
static const u32 soft_gain_tbl_16[SOFT_GAIN_MAX_LEVEL+1] = {
    SOFT_GAIN_N0DB,  SOFT_GAIN_N1DB,  SOFT_GAIN_N2DB,  SOFT_GAIN_N3DB,  SOFT_GAIN_N4DB,
    SOFT_GAIN_N5DB,  SOFT_GAIN_N6DB,  SOFT_GAIN_N7DB,  SOFT_GAIN_N8DB,  SOFT_GAIN_N9DB,
    SOFT_GAIN_N10DB, SOFT_GAIN_N11DB, SOFT_GAIN_N12DB, SOFT_GAIN_N13DB, SOFT_GAIN_N14DB,
    SOFT_GAIN_N15DB, SOFT_GAIN_N110DB,
};
#elif (SOFT_GAIN_MAX_LEVEL == 8)
static const u32 soft_gain_tbl_8[SOFT_GAIN_MAX_LEVEL+1] = {
    SOFT_GAIN_N0DB,  SOFT_GAIN_N2DB,  SOFT_GAIN_N4DB,  SOFT_GAIN_N6DB,  SOFT_GAIN_N8DB,
    SOFT_GAIN_N10DB, SOFT_GAIN_N12DB, SOFT_GAIN_N15DB, SOFT_GAIN_N110DB,
};
#endif

void soft_gain_set_level(uint level)
{
    soft_gain.gain_tbl_index = (level > SOFT_GAIN_MAX_LEVEL)? SOFT_GAIN_MAX_LEVEL : level;

#if (SOFT_GAIN_MAX_LEVEL == 64)
    soft_gain.target_gain = soft_gain_tbl_64[soft_gain.gain_tbl_index];
#elif (SOFT_GAIN_MAX_LEVEL == 32)
    soft_gain.target_gain = soft_gain_tbl_32[soft_gain.gain_tbl_index];
#elif (SOFT_GAIN_MAX_LEVEL == 16)
    soft_gain.target_gain = soft_gain_tbl_16[soft_gain.gain_tbl_index];
#elif (SOFT_GAIN_MAX_LEVEL == 8)
    soft_gain.target_gain = soft_gain_tbl_8[soft_gain.gain_tbl_index];
#endif
}

void soft_gain_init(void)
{
    memset(&soft_gain, 0, sizeof(soft_gain));
    soft_gain.step      = 1;
    soft_gain.fade_en   = 1;

    uint first_level =  (SOFT_GAIN_DEFAULT_LEVEL > SOFT_GAIN_MAX_LEVEL)? SOFT_GAIN_MAX_LEVEL : SOFT_GAIN_DEFAULT_LEVEL;
    soft_gain_set_level(first_level);
}

void soft_gain_up(void)
{
    if (soft_gain.gain_tbl_index) {
        soft_gain_set_level(soft_gain.gain_tbl_index-1);
    }
}

void soft_gain_down(void)
{
    soft_gain_set_level(soft_gain.gain_tbl_index+1);
}

uint8_t soft_gain_level_get(void)
{
    return soft_gain.gain_tbl_index;
}

bool soft_gain_level_is_max_min(void)
{
    if ((soft_gain.gain_tbl_index == 0) || (soft_gain.gain_tbl_index == SOFT_GAIN_MAX_LEVEL)) {
        return true;
    }

    return false;
}

AT(.com_text.soft_gain)
s32 soft_gain_proc_16bit(s16 data)
{
    s32 temp = 0;

    if (soft_gain.fade_en) {
        if (soft_gain.target_gain != soft_gain.curr_gain) {
            if (soft_gain.curr_gain > soft_gain.target_gain) {
                soft_gain.curr_gain -= soft_gain.step;
            } else if(soft_gain.curr_gain < soft_gain.target_gain) {
                soft_gain.curr_gain += soft_gain.step;
            }
        }
    }

    temp = __builtin_muls_shift15(data, soft_gain.curr_gain);

    return (s32)temp;
}
#else
uint8_t soft_gain_level_get(void){return 0;}
void soft_gain_down(void){}
void soft_gain_up(void){}
bool soft_gain_level_is_max_min(void){return false;}
AT(.com_text.soft_gain)
s32 soft_gain_proc_16bit(s16 data){}
#endif

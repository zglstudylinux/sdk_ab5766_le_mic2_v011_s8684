#include "include.h"


//------------------------------------------------------------------------------------------
AT(.com_text.bsp.key)
uint8_t evt_type_2_msg_idx(uint16_t evt_type)
{
    uint8_t msg_idx = 0xff;

    switch(evt_type)
    {
        case KEY_SHORT_UP:
            msg_idx = 0;
            break;
        case KEY_LONG:
            msg_idx = 1;
            break;
        case KEY_HOLD:
            msg_idx = 2;
            break;
        case KEY_LONG_UP:
            msg_idx = 3;
            break;
        case KEY_DOUBLE:
            msg_idx = 4;
            break;
        case KEY_THREE:
            msg_idx = 5;
            break;
        case KEY_FOUR:
            msg_idx = 6;
            break;
        case KEY_FIVE:
            msg_idx = 7;
            break;
    }

    return msg_idx;
}

//mic emit模式按键消息
#if WIRELESS_MIC_K12_KEY_EN
const u8 mic_emit_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,              长按下,                 HOLD,               长按抬起,               双击,               三击                四击            五击
    [KEY_ID_PP] ={MSG_VOL_MUTE,      MSG_NO,                 MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K1] ={MSG_VOL_UP,        MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_ECHO_LEVEL_UP,  MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K2] ={MSG_VOL_DOWN,      MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_ECHO_LEVEL_DOWN,MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K3] ={MSG_CHANGE_MAGIC,  MSG_VOICE_RM,           MSG_NO,             MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
};
#else
const u8 mic_emit_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,              长按下,                 HOLD,               长按抬起,               双击,               三击                四击            五击
    [KEY_ID_PP] ={MSG_VOL_MUTE,      MSG_NO,                 MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K1] ={MSG_VOL_UP,        MSG_NO,                 MSG_NO,             MSG_TEST_X,                 MSG_ECHO_LEVEL_UP,MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K2] ={MSG_VOL_DOWN,      MSG_NO,                 MSG_NO,             MSG_CHANGE_MAGIC,                 MSG_ECHO_LEVEL_DOWN,MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K3] ={MSG_NO,            MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
};
#endif

//ADAPTER模式按键消息
const u8 adapter_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,              长按下,                 HOLD,               长按抬起,               双击,               三击                四击            五击
    [KEY_ID_PP] ={MSG_NO,            MSG_NO,                 MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K1] ={MSG_NO,            MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K2] ={MSG_NO,            MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_ID_K3] ={MSG_NO,            MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_NO,             MSG_NO,             MSG_NO,         MSG_NO},
};

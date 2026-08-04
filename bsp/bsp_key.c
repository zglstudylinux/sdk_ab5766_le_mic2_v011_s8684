#include "include.h"
#include "driver_com.h"
#include "bsp_saradc_key.h"
#include "bsp_io_key.h"

#if BSP_KEY_EN

#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
#define KEY_HOLD_CNT    100
typedef struct {
    uint16_t key_up_cnt;
    uint16_t key_press_cnt;
    ///弄成变量的形式方便后面rom版打patch修改
    uint16_t key_hold_cnt;
} key_press_mute_str_t;
static key_press_mute_str_t key_press_mute_str;
#endif

static bool power_on_flag;
static u8 key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];

AT(.com_rodata.key)
u16 allow_multiple_key_table[] = {
    KEY_ID_PP | KEY_SHORT_UP,
    KEY_ID_K1 | KEY_SHORT_UP,
    KEY_ID_K2 | KEY_SHORT_UP,
    KEY_ID_K3 | KEY_SHORT_UP,
    KEY_ID_K4 | KEY_SHORT_UP,
};

AT(.com_rodata.key)
key_param_t key_param = {
    .key_scan_times = KEY_SCAN_TIMES,
    .key_up_times   = KEY_UP_TIMES,
    .key_long_times = KEY_LONG_TIMES,
    .key_hold_times = KEY_HOLD_TIMES,
};

AT(.text.bsp.key.init) ALIGNED(128)
void key_set_msg_tbl(const void *msg_tbl)
{
    u8 msg_buf[KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX];

    if(msg_tbl == NULL) {
        memset(msg_buf, MSG_NO, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    } else {
        memcpy(msg_buf, msg_tbl, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    }

    GLOBAL_INT_DISABLE();
    memcpy(key_msg_tbl, msg_buf, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    GLOBAL_INT_RESTORE();
}
AT(.com_text.bsp.key)
static u16 bsp_key_get_msg(u16 key_evt)
{
    u16 key_id  = key_evt & KEY_ID_MASK;
    u16 evt_type = key_evt & 0xff00;
    if(key_id < KEY_TBL_MAX_NB) {
        u8 msg_idx = evt_type_2_msg_idx(evt_type);
        if(msg_idx < KEY_MSG_MAX_IDX) {
            return key_msg_tbl[key_id][msg_idx];
        }
    }

    return NO_KEY;
}

// mode: 0-事件检测  1-状态检测
AT(.com_text.key.scan)
u16 bsp_key_scan(u8 mode)
{
    u16 key, key_id = KEY_ID_NO;

#if BSP_IOKEY_EN
    key_id = bsp_get_io_key_id();
#endif // BSP_IOKEY_EN

#if BSP_ADKEY_EN
    if (key_id == KEY_ID_NO) {
        key_id = bsp_get_adkey_id();            // 根据ADC采样值返回键值
#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
        if (key_id) {
            if (!mic_enc_key_press_mute_en_get() && !key_press_mute_str.key_press_cnt) {
                mic_enc_key_press_mute_en_set(1);
            }
            if (mic_enc_key_press_mute_en_get()) {
                key_press_mute_str.key_press_cnt++;
                if (key_press_mute_str.key_press_cnt > key_press_mute_str.key_hold_cnt) {
                    mic_enc_key_press_mute_en_set(0);
                    key_press_mute_str.key_press_cnt =  key_press_mute_str.key_hold_cnt;
                }
            }
            key_press_mute_str.key_up_cnt = 0;
        } else {
            if (!mic_enc_key_press_mute_en_get() && !key_press_mute_str.key_up_cnt) {
                mic_enc_key_press_mute_en_set(1);
            }

            if (mic_enc_key_press_mute_en_get()) {
                key_press_mute_str.key_up_cnt++;
                if (key_press_mute_str.key_up_cnt > key_press_mute_str.key_hold_cnt) {
                    mic_enc_key_press_mute_en_set(0);
                    key_press_mute_str.key_up_cnt = key_press_mute_str.key_hold_cnt;
                }
            }

            key_press_mute_str.key_press_cnt = 0;
        }
#endif
    }
#endif // BSP_ADKEY_EN

    if (mode) {
        return key_id;
    }

    if (bsp_key_get_power_on_flag()) {
        if (key_id == KEY_ID_PP) {
            key_id = KEY_ID_NO;
        } else {
            bsp_key_set_power_on_flag(false);
        }
    }

    key = key_status_process(key_id, &key_param);              // 短按、长按、连按处理
    key = key_multi_press_process(key);     // 多击处理

    if(key){
        key = bsp_key_get_msg(key);
        msg_enqueue(key);
    }

    return key;
}

void bsp_key_init(void)
{

#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
    multiple_key_init(250, allow_multiple_key_table, sizeof(allow_multiple_key_table) / sizeof(u16));
    memset(&key_press_mute_str, 0, sizeof(key_press_mute_str));
    key_press_mute_str.key_hold_cnt = KEY_HOLD_CNT;
#else
    multiple_key_init(((DOUBLE_KEY_TIME*100)/5), allow_multiple_key_table, sizeof(allow_multiple_key_table) / sizeof(u16));
#endif

#if BSP_ADKEY_EN
    bsp_adkey_init();
#endif // BSP_ADKEY_EN

#if BSP_IOKEY_EN
    bsp_io_key_init();
#endif // BSP_IOKEY_EN
}

AT(.com_text.key.set.power_on)
void bsp_key_set_power_on_flag(bool flag)
{
    power_on_flag = flag;
}

AT(.com_text.key.get.power_on)
bool bsp_key_get_power_on_flag(void)
{
    return power_on_flag;
}

#endif // BSP_KEY_EN

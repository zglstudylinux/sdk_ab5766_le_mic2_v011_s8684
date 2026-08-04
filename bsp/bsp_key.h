#ifndef _BSP_KEY_H
#define _BSP_KEY_H


// Key Property
#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
#define KEY_SCAN_TIMES          15          //按键防抖的扫描次数
#define KEY_UP_TIMES            15          //抬键防抖的扫描次数
#define KEY_LONG_TIMES          375         //长按键的次数
#define KEY_HOLD_TIMES          87          //连按的频率次数
#else
#define KEY_SCAN_TIMES          4           //按键防抖的扫描次数
#define KEY_UP_TIMES            4           //抬键防抖的扫描次数
#define KEY_LONG_TIMES          150         //长按键的次数
#define KEY_HOLD_TIMES          35          //连按的频率次数
#endif
#define KEY_LONG_HOLD_TIMES     (KEY_LONG_TIMES + KEY_HOLD_TIMES)

// Key ID
#define KEY_ID_NO               0x00
#define KEY_ID_PP               0x01
#define KEY_ID_K1               0x02
#define KEY_ID_K2               0x03
#define KEY_ID_K3               0x04
#define KEY_ID_K4               0x05
#define KEY_ID_MASK             0xff

#define KEY_TBL_MAX_NB          (KEY_MAX_NB+1)
#define KEY_MSG_MAX_IDX         8           //需要响应的按键消息

#define NO_KEY                  0x0000

void bsp_key_init(void);
u16 bsp_key_scan(u8 mode);
void bsp_key_set_power_on_flag(bool flag);
bool bsp_key_get_power_on_flag(void);
void key_set_msg_tbl(const void *msg_tbl);
uint8_t evt_type_2_msg_idx(uint16_t evt_type);

extern const u8 key_evt_idx_tbl[];

#endif // _BSP_KEY_H

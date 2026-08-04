#ifndef _BSP_SYS_H
#define _BSP_SYS_H

//pwroff key status
enum {
    PWROFF_IDLE,
    PWROFF_W4_TIMEOUT,
    PWROFF_END,
};

struct pwroff_tag {
    volatile union {
        struct {
            u8 charge_full_ind  : 1;
            u8 ntc_ind          : 1;
            u8 low_bat_ind      : 1;
            u8 aux_insert_ind   : 1;
            u8 pwr_key_ind      : 1;
            u8 timeout_ind      : 1;
            u8 user_ind         : 1;    //app、tws等操作
        };
        u8 all_flag;
    };
    u8 tone_en;                         //是否播放pwroff提示音
    u8 key_state;                       //PWR键状态
    u32 key_ticks;                      //PWR键按下的时刻
    u32 delay_ticks;                    //PWR键长按超时时间
};

typedef struct {
    u32 rst_reason;
    u32 wakeup_reason;
    uint8_t disp_sta;
    u32 pwroff_time;
    u32 pwroff_delay;
    struct pwroff_tag pwroff;
    u8 mic_alg_en;              //控制mic算法开关
    u8 emit_send_flag;          //控制发送私有命令
    u8 charge_box_type;         //0:不支持充电仓功能 1:常5V类型 2:维持电压类型
    u32 cache_miss_cnt;         //cache_miss的统计
    u32 cache_miss_ticks;
#if WIRELESS_CON_PAIR_MODE
    u8 con_role_data[3];
    u8 con_role;
#endif
} sys_cb_t;
extern sys_cb_t sys_cb;

void bsp_sys_init(void);
void bsp_periph_init(void);
void code_info_dump(void);
void usr_tmr5ms_thread_do(void);

#endif // _BSP_SYS_H


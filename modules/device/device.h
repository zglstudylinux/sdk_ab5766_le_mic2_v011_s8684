#ifndef _DEVICE_H
#define _DEVICE_H

#ifndef DEV_TOTAL_NUM
    #define DEV_TOTAL_NUM   1
#endif

enum {
    DEV_USB           = 0,
    //DEV_TOTAL_NUM,
};

typedef struct {
    u8 sta;
    u8 online;
    u8 invalid;                         //无效设备
    u16 delay_online[DEV_TOTAL_NUM];    //online消抖延时
    u16 delay_offline[DEV_TOTAL_NUM];   //offline消抖延时
    u16 cnt[DEV_TOTAL_NUM];
} dev_cb_t;

void dev_init(u16 online_cnt, u16 offline_cnt);
bool dev_is_online(u8 dev_num);
bool dev_online_filter(u8 dev_num);
bool dev_offline_filter(u8 dev_num);
void dev_delay_times(u8 dev_num, u16 times);          //同时设置, online去抖延时 和 offline 去抖延时
void dev_delay_online_times(u8 dev_num, u16 times);   //单独设置online去抖延时
void dev_delay_offline_times(u8 dev_num, u16 times);  //单独设置offline 去抖延时
void dev_clr_sta(u8 dev_num);

#endif //_DEVICE_H

#include "include.h"
#include "device.h"

dev_cb_t dev_cb AT(.buf.device.cb);


AT(.text.device.init)
void dev_init(u16 online_cnt, u16 offline_cnt)
{
    memset(&dev_cb, 0, sizeof(dev_cb));

    for (uint8_t i = 0; i < DEV_TOTAL_NUM; i++) {
        dev_cb.delay_online[i] = online_cnt;
        dev_cb.delay_offline[i] = offline_cnt;
    }
}

AT(.com_text.device)
bool dev_is_online(u8 dev_num)
{
    return (dev_cb.online & BIT(dev_num)) ? true : false;
}

///设备在线检测去抖
AT(.com_text.device)
bool dev_online_filter(u8 dev_num)
{
    if (!(dev_cb.sta & BIT(dev_num))) {
        dev_cb.sta |= BIT(dev_num);
        dev_cb.cnt[dev_num] = 0;
    } else {
        dev_cb.cnt[dev_num]++;
    }
    if (dev_cb.online & BIT(dev_num)) {
        return false;
    }
    if (dev_cb.cnt[dev_num] < dev_cb.delay_online[dev_num]) {
        return false;
    }
    dev_cb.online |= BIT(dev_num);
    dev_cb.invalid &= ~BIT(dev_num);
    return true;
}

///设备离线检测去抖
AT(.com_text.device)
bool dev_offline_filter(u8 dev_num)
{
    if (dev_cb.sta & BIT(dev_num)) {
        dev_cb.sta &= ~BIT(dev_num);
        dev_cb.cnt[dev_num] = 0;
    } else {
        dev_cb.cnt[dev_num]++;
    }
    if (!(dev_cb.online & BIT(dev_num))) {
        return false;
    }
    if (dev_cb.cnt[dev_num] < dev_cb.delay_offline[dev_num]) {
        return false;
    }
    dev_cb.online &= ~BIT(dev_num);
    return true;
}

AT(.text.device.init)
void dev_delay_times(u8 dev_num, u16 times)
{
    dev_cb.delay_online[dev_num] = times;
    dev_cb.delay_offline[dev_num] = times;
}

AT(.text.device.init)
void dev_delay_online_times(u8 dev_num, u16 times)
{
    dev_cb.delay_online[dev_num] = times;
}

AT(.text.device.init)
void dev_delay_offline_times(u8 dev_num, u16 times)
{
    dev_cb.delay_offline[dev_num] = times;
}

AT(.text.device.init)
void dev_clr_sta(u8 dev_num)
{
    dev_cb.sta &= ~BIT(dev_num);
    dev_cb.online &= ~BIT(dev_num);
}

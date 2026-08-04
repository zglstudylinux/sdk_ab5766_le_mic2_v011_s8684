#ifndef _BSP_CHARGE_H
#define _BSP_CHARGE_H

#define CHARGE_DC_IN()     ((RTCCON >> 19) & 0x02)
#define CHARGE_INBOX()     ((RTCCON >> 22) & 0x01)

//charge box type
enum {
    CHARGE_BOX_DISABLE = 0,
    CHARGE_BOX_CONST_5V,     //常5V类型(充满电后充电仓仍然维持在5V, 充电仓低电的时候可能掉到维持电压)
    CHARGE_BOX_MAINTAIN,     //维持电压类型(充满电后充电仓掉到维持电压)
};

void bsp_charge_init(void);
void bsp_charge_process(void);


#endif // _BSP_CHARGE_H

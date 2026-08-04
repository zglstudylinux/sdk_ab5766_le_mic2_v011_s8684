#include "include.h"
#include "driver_charge.h"
#include "bsp_charge.h"
#include "bsp_saradc_vbat.h"


#if BSP_CHARGE_EN

#define RECHARGE_EN                             0           //复充使能
#define CHARGE_FULL_PWROFF_EN                   1           //充满电是否进关机


#if RECHARGE_EN
    #define RECHARGE_THRESHOLD                  (4200)      //4.2V, this value should not less than 3V
#endif

//const char charge_status_info[][29] = {
//    "[charge] uninitialized\n",
//    "[charge] vusb disconnected\n",
//    "[charge] finish-but-connect\n",
//    "[charge] trickle cur state\n",
//    "[charge] const cur state\n",
//    "[charge] const vol state\n",
//};

static u8 charge_status_last = CHARGE_STA_OFF;

// start charge -> [trigle stage] -> constant current & constant voltage -> finish charge
void bsp_charge_init(void)
{
    charge_init_typedef charge_init_struct;

    memset(&charge_init_struct, 0, sizeof(charge_init_struct));

//    charge_init_struct.const_curr = CHARGE_CUR_100mA;
//    charge_init_struct.cutoff_curr = CHARGE_CUTOFF_CUR_30mA;
//    charge_init_struct.cutoff_volt = CHARGE_CUTOFF_VOL_4V35;
//    charge_init_struct.dcin_reset = CHARGE_DCIN_RESET_DIS;
//    charge_init_struct.trick_curr = CHARGE_CUR_20mA;
//    charge_init_struct.trick_curr_en = ENABLE;
//    charge_init_struct.mode = CHARGE_MODE_FULL_DISCONNECT;
//    charge_init_struct.vol_follow = DISABLE;
//    charge_init_struct.vol_follow_sel = CHARGE_VOL_FOLLOW_375mV;

    //部分常用设置关联配置文件
    charge_init_struct.const_curr    = xcfg_cb.charge_constant_curr;
    charge_init_struct.cutoff_curr   = xcfg_cb.charge_stop_curr*0x800;
    charge_init_struct.cutoff_volt   = CHARGE_CUTOFF_VOL_4V35;

    if (xcfg_cb.charge_dc_in_rst) {
        charge_init_struct.dcin_reset = CHARGE_DCIN_RESET_EN;
    } else {
        charge_init_struct.dcin_reset = CHARGE_DCIN_RESET_DIS;
    }

    charge_init_struct.trick_curr    = xcfg_cb.charge_trickle_curr;
    charge_init_struct.trick_curr_en = xcfg_cb.charge_trick_en;
    charge_init_struct.mode          = CHARGE_MODE_FULL_DISCONNECT;
    charge_init_struct.vol_follow    = DISABLE;
    charge_init_struct.vol_follow_sel = CHARGE_VOL_FOLLOW_375mV;

    charge_init(&charge_init_struct);

    if (!xcfg_cb.charge_working_while_charging) {
        while (charge_detect_dc() == CHARGE_DC_STATUS_ONLINE) {
            WDT_CLR();
            bsp_charge_process();
#if CHARGE_FULL_PWROFF_EN
            //已经充满电, 退出充电并进入关机流程
            if((charge_get_status() == CHARGE_STA_OFF) && func_cb.sta == FUNC_PWROFF) {
                printf("charge exit\n");
                break;
            }
#endif
        }

        charge_status_last = CHARGE_STA_OFF;
        led_charge_off();

#if CHARGE_FULL_PWROFF_EN
#if BSP_CHARGE_BOX_EN
        if(sys_cb.charge_box_type != CHARGE_BOX_DISABLE){
            //printf("charge_box_type:%d 0x%x 0x%x\n",sys_cb.charge_box_type, CHARGE_DC_IN(), CHARGE_INBOX());
            u8 tick_ms = 0;
            while(CHARGE_INBOX()){   //假如设备正在充电, 并从仓里面拿出来, VUSB还没掉电那么快, 检测一小段时间CHARGE_INBOX, 看看是出仓还是入仓
                delay_ms(100);
                tick_ms++;
                WDT_CLR()
                if(tick_ms >= 5) {
                    break;
                }
            }

            //1、充满电关机func_cb.sta == FUNC_PWROFF
            //2、入仓关机
            if(func_cb.sta == FUNC_PWROFF || CHARGE_INBOX()) {
                func_pwroff();
            }
        } else
#endif
        if(func_cb.sta == FUNC_PWROFF) {
            func_pwroff();
        }
#endif
    }
}

// 测试函数
AT(.text.app.proc.charge)
void bsp_charge_process(void)
{
    static u32 delay_cnt1;
    static u32 delay_cnt2;
    u8 charge_status = charge_status_last;

    if (!xcfg_cb.charge_en) {
        return;
    }

    charge_status = charge_get_status();
    if (charge_status != charge_status_last) {
        charge_status_last = charge_status;
        printf("charge: %d\n", charge_status);
        if (charge_status >= CHARGE_STA_ON_CON_CURR) {
            led_charge_on();
        } else {
            led_charge_off();
        }
//        printf(charge_status_info[charge_status]);
    }

    if (charge_status_last > CHARGE_STA_OFF_BUT_DC_IN) {
        lowpwr_pwroff_delay_reset();
        lowpwr_sleep_delay_reset();
    }

#if CHARGE_FULL_PWROFF_EN
     if (charge_status_last == CHARGE_STA_OFF_BUT_DC_IN) {
        printf("-->charge full, enter pwroff\n");
        charge_status_update(CHARGE_STA_OFF);
        func_cb.sta = FUNC_PWROFF;
        return;
     }
#endif

    if (tick_check_expire(delay_cnt1, 100)) {
        delay_cnt1 = tick_get();
        charge_process();

        /*if stop charging by battery fully charged, we do recharging here*/
        if ((charge_get_status() == CHARGE_STA_OFF_BUT_DC_IN) && (charge_get_mode() != CHARGE_MODE_FULL_KEEP)) {
            #if (BSP_VBAT_DETECT_EN && RECHARGE_EN)
                uint32_t vbat_cur = bsp_get_vbat_level(1);

                if(vbat_cur < RECHARGE_THRESHOLD) {
                    printf("[charge] start re-charging\n");
                    charge_status_update(CHARGE_STA_ON_CON_CURR);
                }
            #endif
        }
    }

    if (tick_check_expire(delay_cnt2, 2000)) {
        delay_cnt2 = tick_get();
        if (charge_detect_dc() == CHARGE_DC_STATUS_ONLINE_BUT_ERR) {
            printf("[charge] CHARGE LINE VOLTAGE ERROR!!!\n");
        }
    }
}

#endif // BSP_CHARGE_EN

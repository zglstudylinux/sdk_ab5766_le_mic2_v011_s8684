#include "include.h"
#include "wireless.h"

AT(.text.bsp.wireless_mic)
static void wireless_mic_role_init(void)
{
    if (xcfg_cb.wireless_adapter_en) {
        cfg_wireless_role = true;
        return;
    } else if (xcfg_cb.wireless_mic_emit_en) {
        cfg_wireless_role = false;
        return;
    } else {
        ///都被去掉默认发射端
        cfg_wireless_role = false;
        return;
    }
}

extern u32 __comm_adapter_vma, __comm_apapter_lma, __comm_apapter_size;
extern u32 __comm_emit_vma, __comm_emit_lma, __comm_emit_size;

void load_code_wireless_mic(void);
static void wireless_mic_load_code(void)
{
    load_code_wireless_mic();
    if(xcfg_cb.wireless_adapter_en) {
        memcpy(&__comm_adapter_vma, &__comm_apapter_lma, (u32)&__comm_apapter_size);
    } else {
        memcpy(&__comm_emit_vma, &__comm_emit_lma, (u32)&__comm_emit_size);
    }
}

AT(.text.bsp.unicast)
uint wireless_mic_name_get(uint8_t *le_name)
{
#if BSP_TBOX_TEST_EN
    if (vusb_test_is_sucess()) {
        return vusb_test_name_get(le_name);
    }
#endif
    int len;
    len = strlen(xcfg_cb.le_name);

    memcpy(le_name, xcfg_cb.le_name, len);

    return len;
}

//蓝牙连接状态返回接口 ，0 为无连接
AT(.text.bsp.wireless_mic)
uint wireless_mic_connected_sta_get(void)
{
    return wireless_mic.connected_sta;
}

#if WIRELESS_CON_PAIR_MODE
AT(.text.bsp.wireless_mic)
void role_defind(u8 index)
{
    if(wireless_mic.connected_sta == 3) {
        sys_cb.con_role_data[0] = 1;               //主副麦区分完毕标志位
    } else {
        sys_cb.con_role_data[0] = 0;               //主副麦区分完毕标志位
    }
}

AT(.text.bsp.wireless_mic)
u8 role_get(u8 index)
{
    if(sys_cb.con_role_data[0]) {
        if(index) {
            if(sys_cb.con_role_data[1]) {
                return 1;                    //主麦
            } else {
                return 2;                    //副麦
            }
        } else {
            if(!sys_cb.con_role_data[1]) {
                return 1;                    //主麦
            } else {
                return 2;                    //副麦
            }
        }
    }
    return 0;
}
#endif

AT(.text.bsp.wireless_mic)
void wireless_emit_notice(uint evt, void *params)
{
//    u32 tmp;
    u8 *packet = params;
    u8 mic_num;

    switch(evt) {
    case BT_NOTICE_WIRELESS_CONNECTED:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("WIRELESS_CONNECTED, %d\n", mic_num);
            if(wireless_mic.connected_sta == 0) {
                sys_clk_req(INDEX_DECODE, SYS_160M);
                if(wireless_role_is_adapter()) {
                    adapter_init();
                } else {
                    mic_emit_init();
                    lowpwr_pwroff_auto_dis();
                }
                sys_cb.mic_alg_en = 1;                  //再使能算法
            }
            #if ADAPTER_UART_COMMAND_EN
            if(wireless_role_is_adapter()) {
                uart_command_tx_data(AB_SET_CON_DISC,1);
            }
            #endif

#if ADAPTER_SYNC_PARAM_EN
            if(wireless_role_is_adapter()) {
                wireless_sync_all_param();
            }
#endif
            wireless_mic.connected_sta |= BIT(mic_num);
            wireless_mic.change_sta[mic_num] = 0;
            wireless_mic.change_flag = 1;
        }
#if WIRELESS_CON_PAIR_MODE
            printf("WIRELESS_CON_ROLE, %d\n", sys_cb.con_role);
            if(sys_cb.con_role) {
                sys_cb.con_role_data[1] = mic_num;       //主麦
            } else {
                sys_cb.con_role_data[2] = mic_num;       //副麦
            }
#endif
#if WIRELESS_CON_BONDING_EN
        if(wireless_role_is_adapter()){
            if ((wireless_emit_get_connected_sta() == (BIT(0)|WIRELESS_CON_LINK_NB)) && !(memcmp(bongding_addr_get(),0x00,6))) {  //若连接设备已满且绑定地址为空，则设置一次 ble_reset_con()
                ble_get_local_bd_addr(bongding_addr_get());                     //取RX本地地址作共同绑定的地址
                wireless_tx_bonding_sync(0,BONDING_SYNC,bongding_addr_get(),7);      //同步需要共同绑定的地址
                if (WIRELESS_CON_LINK_NB == 2){
                    wireless_tx_bonding_sync(1,BONDING_SYNC,bongding_addr_get(),7);
                }
                param_write_bonding_addr(bongding_addr_get());                       //共同绑定的地址写进flash内
                btstack_ble_reset_hash(2);
            }
        }
#endif // WIRELESS_CON_BONDING_EN
#if WIRELESS_CON_PAIR_MODE
            role_defind(mic_num);
#endif
        break;

    case BT_NOTICE_WIRELESS_CONNECT_FAIL:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("WIRELESS_CONNECT_FAIL, %d\n", mic_num);
            wireless_mic.change_sta[mic_num] = 1;
            wireless_mic.change_flag = 1;
        }
        break;

    case BT_NOTICE_WIRELESS_DISCONNECT:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("WIRELESS_DISCONNECT, %d\n", mic_num);
            wireless_mic.connected_sta &= ~BIT(mic_num);
            wireless_mic.change_sta[mic_num] = 2;
            wireless_mic.change_flag = 1;
#if WIRELESS_CON_PAIR_MODE
            wireless_mic.con_temp = 0;
            if(role_get(mic_num) == 2) {                        //副麦断开
                sys_cb.con_role = 1;
            } else if(role_get(mic_num) == 1) {                 //主麦断开
                sys_cb.con_role = 0;
            }
#endif
            if(wireless_mic.connected_sta == 0) {
                sys_cb.mic_alg_en = 0;                  //先关闭算法

                if(wireless_role_is_adapter()) {
                    adapter_reset(mic_num, 0);
                } else {
                    mic_emit_reset();
                    lowpwr_pwroff_auto_en();
                }
                sys_clk_free(INDEX_DECODE);             //再还原主频
#if BSP_TBOX_TEST_EN
                if (vusb_test_is_sucess() && cfg_discon_auto_pwroff) {
                    func_cb.sta = FUNC_PWROFF;
                }
#endif
            } else {
                adapter_reset(mic_num, wireless_mic.connected_sta);
            }

            #if ADAPTER_UART_COMMAND_EN
            if(wireless_role_is_adapter()) {
                uart_command_tx_data(AB_SET_CON_DISC,0);
            }
            #endif
        }
        break;

#if WIRELESS_CON_CODEC_SEL == CODEC_ESBC
    case BT_NOTICE_WIRELESS_VIRTUAL_DATA_CONNECTED:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("BT_NOTICE_WIRELESS_VIRTUAL_DATA_CONNECTED, %d\n", mic_num);
            if(wireless_mic.connected_sta == 0) {
                sys_clk_req(INDEX_DECODE, SYS_160M);    //先抬高主频
                if(!wireless_role_is_adapter()) {
                    dis_auto_pwroff();
                }
            }
            wireless_mic.connected_sta |= BIT(mic_num);
            wireless_mic.change_sta[mic_num] = 0;
            wireless_mic.change_flag = 1;

        }
        break;

    case BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_CONNECTED:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_CONNECTED, %d\n", mic_num);
            wireless_mic.audio_sta |= BIT(mic_num);
            if(wireless_role_is_adapter()) {
                adapter_init();
            } else {
                mic_emit_init();
            }
            sys_cb.mic_alg_en = 1;                  //再使能算法
            #if ADAPTER_UART_COMMAND_EN
            if(wireless_role_is_adapter()) {
                uart_command_tx_data(AB_SET_CON_DISC,1);
            }
            #endif

#if ADAPTER_SYNC_PARAM_EN
            if(wireless_role_is_adapter()) {
                wireless_sync_all_param();
            }
#endif
        }
        break;

    case BT_NOTICE_WIRELESS_VIRTUAL_DATA_DISCONNECT:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("BT_NOTICE_WIRELESS_VIRTUAL_DATA_DISCONNECT, %d\n", mic_num);

            wireless_mic.connected_sta &= ~BIT(mic_num);
            wireless_mic.change_sta[mic_num] = 2;
            wireless_mic.change_flag = 1;

            if(wireless_mic.connected_sta == 0) {
                if(!wireless_role_is_adapter()) {
                    en_auto_pwroff();
                }
                sys_clk_free(INDEX_DECODE);             //再还原主频
            }
        }
        break;

    case BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_DISCONNECT:
        mic_num = packet[0];
        if(mic_num < WIRELESS_CON_LINK_NB) {
            printf("BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_DISCONNECT, %d\n", mic_num);

            wireless_mic.audio_sta &= ~BIT(mic_num);
            if(wireless_mic.audio_sta == 0) {
                sys_cb.mic_alg_en = 0;                  //先关闭算法

                if(wireless_role_is_adapter()) {
                    adapter_reset(mic_num, 0);
                } else {
                    mic_emit_reset();
                }
            } else {
                adapter_reset(mic_num, wireless_mic.audio_sta);
            }

            #if ADAPTER_UART_COMMAND_EN
            if(wireless_role_is_adapter()) {
                uart_command_tx_data(AB_SET_CON_DISC,0);
            }
            #endif
        }
        break;
#endif

    default:
        break;
    }
}

u8 wireless_emit_get_connected_sta(void)
{
    return wireless_mic.connected_sta;
}


void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len)
{
//    printf("user_cmd%d: \n", index);
//    print_r(ptr, len);
    if (wireless_role_is_adapter()){                          //适配器

    } else {                                                  //设备端
#if WIRELESS_CON_BONDING_EN
        if(ptr[0] == 12){
            memcpy(bongding_addr_get(),ptr + 1,6);            //TX端同步RX端获取到的地址
            msg_enqueue(EVT_CONNECT_BONDDING);					//发到主线程处理
        }
#endif // WIRELESS_CON_BONDING_EN
    }
}

AT(.text.func.process.wireless)
void wireless_mic_proc(void)
{
    if(wireless_mic.connected_sta != sys_cb.disp_sta) {
        sys_cb.disp_sta = wireless_mic.connected_sta;
//        switch(sys_cb.disp_sta) {
//        case 0:
//            led_bt_idle();
//            break;
//        case 1:
//            led_bt_connected();
//            break;
//        }
#if BSP_LED_EN
        if(sys_cb.disp_sta == 0) {
            led_bt_idle();
        } else {
            led_bt_connected();
        }
#endif
    }
}

AT(.text.bsp.wireless_mic)
void wireless_mic_var_init(void)
{
    wireless_mic_role_init();
    wireless_mic_load_code();
    wireless_cmd_init();
}

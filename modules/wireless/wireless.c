#include "include.h"
#include "wireless.h"

#if LE_FCC_TEST_EN
    #define WORK_MODE           MODE_FCC_TEST
#elif LE_BQB_RF_EN
    #define WORK_MODE           MODE_BQB_RF_BLE
#else
    #define WORK_MODE           MODE_NORMAL
#endif
uint8_t cfg_le_rssi_thr             = WIRELESS_CON_RSSI_THR;
uint8_t cfg_wireless_link_nb        = WIRELESS_CON_LINK_NB;
uint8_t cfg_wireless_con_interval   = WIRELESS_CON_INTERVAL*WIRELESS_CON_LINK_NB;    //传输interval，单位1.25ms
uint8_t cfg_wireless_tx_comb_nb     = WIRELESS_MIC_COMB_NB;
uint8_t cfg_wireless_tx_interval    = WIRELESS_MIC_TX_INTERVAL;
uint8_t cfg_wireless_tx_retry       = WIRELESS_MIC_RETRY_NB;
uint8_t cfg_wireless_d2a_tx_size    = MIC_TX_BUFFER_SIZE;                                 //传输封包大小
uint16_t cfg_wireless_feat          = ((WIRELESS_CON_VERS<<8)&FEAT_VERS) | FEAT_D2A | (FEAT_HOP_V1) | (FEAT_BONDING*WIRELESS_CON_BONDING_EN) | (WIRELESS_CON_LINK_NB&0xf);
uint16_t cfg_wireless_d2a_enc_us    = (WIRELESS_MIC_ENC_MAX_US + WIRELESS_MIC_EQ_DRC_DELAY + WIRELESS_MIC_MAGIC_DELAY + WIRELESS_MIC_ECHO_DELAY + WIRELESS_FREQ_SHIFT2_DELAY
                                       + WIRELESS_MIC_ROOM_REVERB_DELAY + WIRELESS_ALLPASS_FILTER_CHANGE_DELAY + WIRELESS_MIC_AGC_DELAY + WIRELESS_MIC_SRC_DELAY + WIRELESS_MIC_DNR_FRE_DELAY );

uint16_t cfg_wireless_d2a_dec_us    = (WIRELESS_MIC_DEC_MAX_US+ADAPTER_MIX_DRC_DELAY);
uint8_t cfg_wireless_reconnect_strong  = 2;
uint8_t cfg_discon_auto_pwroff      = 0;
bool cfg_wireless_role              = true;

uint8_t cfg_wireless_codec[2]       = {
    WIRELESS_CON_CODEC_SEL | CODEC_MONO,       //D2A Codec
    0,                              //A2D Codec
};

uint8_t cfg_bb_rf_26m_oscillator    = 0; //0: 24M  1:26M
//uint8_t cfg_wireless_mic_rssi_thr   = WIRELESS_MIC_RSSI_THR;
uint8_t cfg_bb_rf_freq_bands        = WIRELESS_CON_FREQ_BAND; //0:2402M~2480M  1:2200M~2278M  2:2320M~2398M  3:2500M~2578M  4: 2482M~2560M
WEAK uint8_t  cfg_bt_tws_tick_freq              = 1;
uint8_t cfg_bb_rf_gfsk_pwr     = 127;                             //调节GFSK TXPWR，范围：90~127
struct wireless_mic_tag wireless_mic;

///stack
uint8_t cfg_bt_work_mode            = WORK_MODE;

uint8_t cfg_wireless_con_id         = 0;       //无线配对码   ///下面有AB麦配对码参考示例
///AB麦配对码参考示例
/*
A麦sdk
cfg_wireless_con_id = 1;
B麦sdk
cfg_wireless_con_id = 2;

接收端sdk
需要动态去切换ble_set_con_id(1)和ble_set_con_id(2);
接收端要配合广播接口一起进行动态切换
思路是可以交替广播两接口，也可以只广播一个等连接再广播一个

*/

#if WIRELESS_CON_BONDING_EN
u8 bt_bongding_addr[6];
u8 *bongding_addr_get(void)
{
    return bt_bongding_addr;
}
#endif //WIRELESS_CON_BONDING_EN

///无线麦频点映射处理，可以去掉部分频点
const struct le_chnl_map host_ch_class_2_3g = {
    .map = {0xe7, 0xbd, 0xcf, 0xf3, 0x1e},
};
const struct le_chnl_map host_ch_class_2_4g = {
    .map = {0xcf, 0x7b, 0xde, 0xe7, 0x1d},
};
const struct le_chnl_map host_ch_class_2_5g = {
    .map = {0xbe, 0xff, 0x7d, 0xdf, 0x17},
};

void wireless_host_ch_class_set(void)
{
    if (WIRELESS_CON_FREQ_BAND == 0) {
        mgr_host_ch_class_set(&host_ch_class_2_4g);
    } else if (WIRELESS_CON_FREQ_BAND == 2) {
        mgr_host_ch_class_set(&host_ch_class_2_3g);
    } else if (WIRELESS_CON_FREQ_BAND == 4) {
        mgr_host_ch_class_set(&host_ch_class_2_5g);
    }
}

u8 bt_rf_get_tx_pwr(void)
{
    return xcfg_cb.rf_tx_pwr;
}

AT(.text.wireless.init)
void printf_connect_message(void)
{
    my_printf("<TX_INTERVAL>         %d\n",cfg_wireless_tx_interval);
    my_printf("<CON_INTERVAL>        %d\n",cfg_wireless_con_interval);
    my_printf("<WIRELESS_FEAT>       %d\n",cfg_wireless_feat);
    my_printf("<WIRELESS_CODEC>      %d\n",cfg_wireless_codec[0]);
    my_printf("<FREQ_BAND>           %d\n",cfg_bb_rf_freq_bands);
    my_printf("<RETRY>               %d\n",cfg_wireless_tx_retry);
    my_printf("<DISCON_AUTO_PWROFF>  %d\n",cfg_discon_auto_pwroff);
    my_printf("<CONFIG_RSSI>         %d\n",cfg_le_rssi_thr);
    my_printf("<WS_NAME>             %s\n",xcfg_cb.le_name);
}

AT(.text.adapter.proc)
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus)
{
#if WIRELESS_MIC_DUMP_PER_BER
    frame_cnt++;
    if(bfi) {
        bfi_cnt++;
    } else if(chstatus == 250) {
        fer250_cnt++;
    } else {
        if(chstatus != 0) {
            fer_cnt++;
        }
        if(chstatus < 250) {
            bit_err_cnt += chstatus;
        }
    }
#endif

    if(bfi || chstatus != 0) {
        if(rssi > -75 || (rssi > -80 && chstatus == 250)) {
            ble_con_channel_assess(chidx, false);
        }
    } else {
        ble_con_channel_assess(chidx, true);
    }

    wireless_dump_set_rx_status(idx, chidx, rssi, (bfi || chstatus != 0));
}

void ble_get_link_info_flash(void *buf, u16 addr, u16 size)
{
//    printf("bt_read: %04x,%04x, %08lx\n", addr, size, BLE_CM_PAGE(addr));
    if ((addr + size) <= PAGE_DATA_SIZE) {
        cm_read(buf, BLE_CM_PAGE(addr), size);
    }
//    print_r(buf, size);
}

void ble_put_link_info_flash(void *buf, u16 addr, u16 size)
{
//    printf("bt_write: %04x,%04x, %08lx\n", addr, size, BLE_CM_PAGE(addr));
//    print_r(buf, size);
    if ((addr + size) <= PAGE_DATA_SIZE) {
        cm_write(buf, BLE_CM_PAGE(addr), size);

    }
}

void ble_get_link_info(void *buf, u16 addr, u16 size)
{
//#if TEST_MODE_BT_INFO
//    if (bt_is_test_mode()) {
//        bt_get_link_info_ram(buf,addr,size);
//    } else
//#endif
    {
        cm_read(buf, BLE_CM_PAGE(addr), size);
    }
}

void ble_put_link_info(void *buf, u16 addr, u16 size)
{
//#if TEST_MODE_BT_INFO
//    if (bt_is_test_mode()) {
//        bt_put_link_info_ram(buf,addr,size);
//    } else
//#endif
    {
        cm_write(buf, BLE_CM_PAGE(addr), size);
    }
}

void ble_sync_link_info(void)
{
    cm_sync();
}


u8 con_req_addr[6];

//发送ble_connect_req消息之后，库获取连接地址的回调函数
bool ble_get_con_req_addr_cb(uint8_t *addr)
{
    memcpy(addr, con_req_addr, 6);
    return true;
}

//发送ble_scan_set_enable消息之后，底层收到数据包的回调函数
//返回值：0=错误的地址，继续扫描；1=地址正确，自动关闭扫描，可发起连接请求
bool ble_scan_rx_rep_cb(uint8_t *addr, uint8_t addr_type)
{
    printf("addr: ");
    print_r(addr, 6);

    ///准备发起连接，关闭scan
    ble_scan_set_enable(0);
    memcpy(con_req_addr, addr, 6);

    ble_connect_req(1000);     //发消息后，通过回调函数ble_get_con_req_addr_cb获取连接地址
    return true;
}

//addr: 连接的设备地址
//timeout: 超时时间（单位ms，0xffff时一直连接不超时）
void ble_create_con_for_addr(uint8_t *addr, uint16_t timeout)
{
    memcpy(con_req_addr, addr, 6);
    ble_connect_req(timeout);   //发消息后，通过回调函数ble_get_con_req_addr_cb获取连接地址
}

void bsp_26m_oscillator_init(void)
{
    cfg_bb_rf_26m_oscillator = WIRELESS_MIC_26M_OSCILLATOR;
}

//------------------------------------------------------------------------------------------
void bsp_ble_init(void)
{
#if BSP_TBOX_TEST_EN
    u8 vusb_config_rssi = vusb_test_rssi_get();
    if (vusb_config_rssi) {
        cfg_le_rssi_thr = vusb_config_rssi;
    }
#endif
    if (wireless_role_is_adapter()) {
        wireless_con_adapter_init();
    } else {
        wireless_con_device_init();
		printf_connect_message();
    }
    bt_setup();
    wireless_dump_init();
}


#if (WIRELESS_CON_BONDING_EN)

WEAK void btstack_ble_reset_hash(uint param)
{
    printf("WEAK FUNCTION\n");
}


///清除绑定
void wireless_mic_unbonding(void)
{
    if (wireless_role_is_adapter()){
        bt_delete_all_link_info(NULL,0);                           //清除配对信息
        memset(bongding_addr_get(),0x00,6);                        //清除绑定地址
        param_write_bonding_addr(bongding_addr_get());             //擦除地址
        btstack_ble_reset_hash(1);
        if(wireless_mic_is_bonding() ){                                //组队绑定
            if(!wireless_emit_get_connected_sta()){                                //无蓝牙连接
//                ble_reset_con(1);
                wireless_mic.change_flag = 1;                           //置位连接状态
            }else{
//                ble_reset_con(1);
                ble_disconnect_req(0);
                ble_disconnect_req(1);
            }
        }
    }else{
        if(wireless_mic_is_bonding()){                                 //组队绑定
            bt_delete_all_link_info(NULL,0);                           //清除配对信息
            memset(bongding_addr_get(),0x00,6);                        //清除绑定地址
            param_write_bonding_addr(bongding_addr_get());             //擦除地址
            if(!wireless_emit_get_connected_sta()){                                //无蓝牙连接
                ble_scan_set_enable(0);                                //先关闭scan
                btstack_ble_reset_hash(1);                             //修改hash
                wireless_mic.change_flag = 1;                          //重新开始扫描
            }else{
                ble_disconnect_req(0);
                btstack_ble_reset_hash(1);
            }
        }
    }
}
#endif

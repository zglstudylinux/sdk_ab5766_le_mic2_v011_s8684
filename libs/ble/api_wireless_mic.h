#ifndef _API_WIRELESS_MIC_H
#define _API_WIRELESS_MIC_H

//WORK MODE
#define MODE_NORMAL                                 0       //normal mode
#define MODE_FCC_TEST                               2       //fcc test mode, use uart for communication
#define MODE_BQB_RF_BLE                             4       //bqb test mode，use uart for communication



//蓝牙通知
enum {
    BT_NOTICE_WIRELESS_CONNECTED = 0x80,        //无线mic连接成功, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_DISCONNECT,              //无线mic断开连接, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_CONNECT_FAIL,            //无线mic连接失败, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_RX_BUF,                  //无线mic接受到的数据回调, param[0]=index, param[1]=buf_len, param[11:2]=buf
    BT_NOTICE_WIRELESS_VIRTUAL_DATA_CONNECTED,  //无线mic只有通信链路连接，音频链路还未连
    BT_NOTICE_WIRELESS_VIRTUAL_DATA_DISCONNECT, //无线mic只有通信链路断开，音频链路没连接的时候断开通信链路
    BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_CONNECTED, //无线mic只有音频链路连接
    BT_NOTICE_WIRELESS_VIRTUAL_AUDIO_DISCONNECT,//无线mic只有音频链路断开

    BT_NOTICE_WIRELESS_END       = 0x9F,        //无线mic回调消息最后一个
};

struct le_chnl_map
{
    ///5-byte channel map array
    uint8_t map[5];
};

//feature
#define FEAT_D2A                BIT(7)      //DEVICE to ADAPTER
#define FEAT_A2D                BIT(6)      //ADAPTER to DEVICE
#define FEAT_BONDING            BIT(5)      //组队绑定，需要清除配对才能重新组队
#define FEAT_VERS               (0x7<<8)    //bit[10:8]
#define FEAT_RATE               (0x7<<11)   //bit[13:11]，传输速率
#define FEAT_HOP_V1             BIT(14)     //bit[14]，新版本跳频计算
#define FEAT_ADAPTER_SAVE       BIT(15)     //bit[15]，发射端算法音频参数由接收端发送同步

extern uint8_t cfg_bt_work_mode;

extern bool cfg_wireless_role;
extern uint16_t cfg_wireless_feat;
extern uint16_t cfg_wireless_d2a_dec_us;
extern uint8_t cfg_wireless_tx_interval;
extern uint8_t cfg_wireless_con_interval;
extern uint8_t cfg_wireless_tx_retry;
extern uint8_t cfg_discon_auto_pwroff;
extern uint8_t cfg_bb_rf_freq_bands;
extern uint8_t cfg_wireless_codec[2];

#define wireless_role_is_adapter()              cfg_wireless_role
#define wireless_mic_is_bonding()               (bool)(cfg_wireless_feat & FEAT_BONDING)

//control
void bt_init(void);                             //初始化蓝牙变量
int bt_setup(void);                             //打开蓝牙模块
void bt_off(void);                              //关闭蓝牙模块
void bt_wakeup(void);                           //唤醒蓝牙模块
void bt_run_loop(void);
u32 bt_get_rand(void);


void ble_adv_set_enable(bool pscan, bool iscan);    //广播使能，pscan=可被连接，iscan=可被发现
void ble_scan_set_enable(bool enable);              //扫描使能
void ble_connect_req(uint timeout_ms);              //发起连接，ms=连接超时（单位毫秒，0xffff时一直连接不超时）
void ble_disconnect_req(uint8_t idx);               //发起断开，idx=连接通道

void ble_create_con_for_addr(uint8_t *addr, uint16_t timeout);
void ble_con_channel_assess(uint8_t chidx, bool rx_ok);
uint8_t ble_con_get_status(void);
uint16_t ble_single_link_duration_get(void);
bool ble_con_user_cmd_tx_req(uint8_t index);        //请求发送用户私有命令，请求后库通过ble_con_user_cmd_get_tx_cb获取，返回值1代表成功，0代表失败
void ble_audio_pwr_ctr_set(uint8_t idx, uint8_t pwr_level);

//link info
uint  bt_get_link_info_addr(uint8_t *bd_addr, int order);
uint bt_get_link_info_nb(void);
void bt_delete_link_info(uint8_t *bd_addr);
void bt_delete_all_link_info(const char *tag, uint32_t ra_addr);
void btstack_ble_reset_hash(uint param);

void mgr_host_ch_class_set(const struct le_chnl_map* host_ch_class);
//void ble_audio_ctr(uint8_t index, uint8_t enable, uint8_t test_rx);

//FCC TEST和BQB_RF通用接口
void ble_hci_tx_callback_set(void *handle);
void ble_dut_pkt_buf_set(void *buf);
void ble_hci_cmd_check(void);

uint8_t bb_crc8(const uint8_t *data, size_t len);


#endif //_API_WIRELESS_MIC_H

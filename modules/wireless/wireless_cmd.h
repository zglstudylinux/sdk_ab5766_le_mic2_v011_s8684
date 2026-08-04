#ifndef _WIRELESS_CMD_API_H
#define _WIRELESS_CMD_API_H

#define TX_MAX_BUF_SIZE         8

#define UDE_HID_NEXTFILE        0x0100
#define UDE_HID_PREVFILE        0x0200
#define UDE_HID_PLAYPAUSE       0x0400
#define UDE_HID_MUTE            0x0800

#define UDE_HID_STOP            0x0010
#define UDE_HID_PROMOTE         0x0020
#define UDE_HID_DEMOTE          0x0040
#define UDE_HID_CART            0x0080

#define UDE_HID_VOLUP           0x0001
#define UDE_HID_VOLDOWN         0x0002
#define UDE_HID_CENTER          0x0004
#define UDE_HID_NONE            0x0008

#define UDE_HID_WHEEL_UP        0x0001
#define UDE_HID_WHEEL_DN        0x00FF

//内部定义
typedef enum{
    PRIVATE_WS_MIC_CMD      = 0,
    PRIVATE_USB_CMD,
    PRIVATE_SYNC_CMD,           ///tx端发送变化信息给rx端，rx端再把信息发给另一个tx端实现两个tx同步
    PRIVATE_USER_DATA,
    PRIVATE_PWR_CTR_CMD,
    PRIVATE_AUDIO_CTR_CMD,
} cmd_t;

typedef struct wireless_cmd{
    u8 cmd;
    u8 buf[TX_MAX_BUF_SIZE];
} wireless_cmd_t;

//无线麦控制
typedef enum{
    ECHO_AUDIO_MUTE         = 0,
    ADPCM_CODER_MUTE,

    ADPCM_DECODER_MUTE,
    PLC_ADPCM_AUDIO_MUTE,
    SNDP_AUDIO_MUTE,
    AINS3_AUDIO_MUTE,
    SRC_AUDIO_MUTE,
    DAC_OUT_AUDIO_MUTE,
    USB_MIC_IN_AUDIO_MUTE,
    MAGIC_AUDIO_MUTE,
    HOWLING_AUDIO_MUTE,
    MIC_MUTE,
    SOFT_POWER_ON_OFF_MUTE,
    DNN_AUDIO_MUTE,
    DNN_AUDIO_LEVEL,
    BONDING_SYNC,

    ECHO_DELAY_LEVEL_UP     = 0x40,     //混响等级加
    ECHO_DELAY_LEVEL_DOWN,              //混响等级减
    MAGIC_LEVEL_CHANGE,                 //切换魔音音效

    MUSIC_CTL_1ST           = 0x60,
    MUSIC_CTL_VOL_UP = MUSIC_CTL_1ST,   //音乐音量加
    MUSIC_CTL_VOL_DOWN,                 //音乐音量减
    MUSIC_CTL_PREV,                     //上一曲
    MUSIC_CTL_NEXT,                     //下一曲
    MUSIC_CTL_PLAY_PAUSE,               //播放/暂停
    MUSIC_CTL_VOICE_REMOVE,             //人声消除切换

    DISCONNECT_NUM,                     //断开无线麦连接
} sub_ws_mic_cmd_t;

//USB播放控制
typedef enum{
    USB_SET_SPK_VOLUME      = 0,
    USB_CTL_MIC_STA,
    USB_CTL_PLAY_PAUSE,
    USB_CTL_VOLUME_UP,
    USB_CTL_VOLUME_DOWN,
    USB_CTL_PREVFILE,
    USB_CTL_NEXTFILE,
} sub_usb_cmd_t;


//device <==> adapter私有命令api
void wireless_cmd_init(void);
void wireless_rx_cmd(u8 index, u8 *ptr, u8 len);
void wireless_tx_usb_cmd(u8 msg, u8 param);
void wireless_tx_user_cmd(u8 *ptr, u8 len);
void wireless_tx_echo_delay_level(u8 echo_delay_level, u8 max_min_flag);
void wireless_tx_soft_gain_level(u8 soft_gain_level, u8 max_min_flag);
void wireless_tx_magic_effect_level(u8 magic_level);
void wireless_tx_mic_mute_level(u8 mic_mute_level);
void wireless_tx_voice_rm_en(u8 voice_rm_en);
void wireless_tx_user_link_cmd(u8 msg, u8 len);

//device <==> adpater USB播放控制api
#if !ADAPTER_USB_SPK_TX_EN
    #define wireless_music_play_pause()
    #define wireless_music_volup()
    #define wireless_music_voldown()
    #define wireless_music_prev()
    #define wireless_music_next()
    #define wireless_set_usbspk_vol(vol)
#else
    #define wireless_music_play_pause()             wireless_tx_usb_cmd(USB_CTL_PLAY_PAUSE, 0)      //切换播放/暂停
    #define wireless_music_volup()                  wireless_tx_usb_cmd(USB_CTL_VOLUME_UP, 0)       //音量加
    #define wireless_music_voldown()                wireless_tx_usb_cmd(USB_CTL_VOLUME_DOWN, 0)     //音量减
    #define wireless_music_prev()                   wireless_tx_usb_cmd(USB_CTL_PREVFILE, 0)        //上一曲
    #define wireless_music_next()                   wireless_tx_usb_cmd(USB_CTL_NEXTFILE, 0)        //下一曲
    #define wireless_set_usbspk_vol(vol)            wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, vol)    //spk_vol
#endif

#if !ADAPTER_USB_MIC_RX_EN
    #define wireless_set_usbmic_status(start)
#else
    #define wireless_set_usbmic_status(start)       wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, start)  //mic_status
#endif

//device <==> adapter无线麦命令
#define wirless_echo_level_up()                     wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_UP, 1)
#define wirless_echo_level_down()                   wireless_tx_ws_mic_cmd(ECHO_DELAY_LEVEL_DOWN, 1)
#define wirless_magic_change()                      wireless_tx_ws_mic_cmd(MAGIC_LEVEL_CHANGE, 1)
#define wirless_music_vol_up()                      wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_UP, 1)
#define wirless_music_vol_up_max()                  wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_UP, 2)
#define wirless_music_vol_down()                    wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_DOWN, 1)
#define wirless_music_vol_down_min()                wireless_tx_ws_mic_cmd(MUSIC_CTL_VOL_DOWN, 2)
#define wirless_music_prev()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_PREV, 1)
#define wirless_music_next()                        wireless_tx_ws_mic_cmd(MUSIC_CTL_NEXT, 1)
#define wirless_music_play_pause()                  wireless_tx_ws_mic_cmd(MUSIC_CTL_PLAY_PAUSE, 1)
#define wirless_music_voice_remove()                wireless_tx_ws_mic_cmd(MUSIC_CTL_VOICE_REMOVE, 1)
#define wirless_music_mic_mute()                    wireless_tx_ws_mic_cmd(MIC_MUTE, 1)

#define wireless_tx_user_discon_cmd()               wireless_tx_user_link_cmd(DISCONNECT_NUM,1);

//外部函数声明
void wireless_cmd_buf_init(void);
bool wireless_send_cmd(u8 index, u8 *cmd, u8 len);
void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len);
void wireless_tx_audio_ctr_cmd(uint8_t index, uint8_t enable, uint8_t test_rx);
void wireless_tx_pwr_ctr_cmd(u8 index, u8 pwr_level);

void wireless_tx_bonding_sync(u8 index, u8 cmd_msg, u8 *bonding_addr, u8 len);

#endif

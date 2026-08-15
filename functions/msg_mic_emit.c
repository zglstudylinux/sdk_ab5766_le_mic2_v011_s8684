#include "include.h"
#include "func.h"

AT(.text.func.mic_emit)
void func_mic_emit_message(u16 msg)
{
    switch (msg) {
//    case KU_PLAY:
//    case KU_PLAY_USER_DEF:
//    case KU_PLAY_PWR_USER_DEF:
////        my_printf("pp\n");
//        break;


#if WIRELESS_CON_BONDING_EN
    case EVT_CONNECT_BONDDING:
        param_write_bonding_addr(bongding_addr_get());              //保存绑定的地址
        btstack_ble_reset_hash(1);                                 //设置hash值
       // my_printf("%s %d \n",__func__,__LINE__);
        break;
#endif

    case MSG_SYS_1S:
#if BSP_VBAT_DETECT_EN
        ///低电处理，1s判断一次，仅作用发射端
        bsp_vbat_proc();
#endif
        break;

#if BSP_VBAT_DETECT_EN
    case EVT_LPWR_WARNING:
        ///低电提醒回调
        printf("EVT_LPWR_WARNING\n");
        break;

    case EVT_LPWR_WARNING_EXIT:
        ///从低电状态转化成正常电量状态，一般是充电电压达到
        printf("EVT_LPWR_WARNING_EXIT\n");
        break;

    case EVT_LPWR_OFF:
        ///低电关机回调
        printf("EVT_LPWR_OFF\n");
        func_cb.sta = FUNC_PWROFF;
        break;
#endif

    case MSG_VOL_MUTE:
        printf("MSG_VOL_MUTE\n");
        if(!wireless_mic_connected_sta_get()) {
            break;
        }
        mic_enc_mute_en_set(!mic_enc_mute_en_get());    //mic mute 异或/取反
        wireless_tx_mic_mute_level(mic_enc_mute_en_get());
        break;

    case MSG_VOL_UP:
        printf("MSG_VOL_UP\n");
        soft_gain_up();                    //话筒音量VOL+

        if(!wireless_mic_connected_sta_get()) {
            break;
        }

        wireless_tx_soft_gain_level(soft_gain_level_get(),soft_gain_level_is_max_min());
        break;

    case MSG_VOL_DOWN:
        printf("MSG_VOL_DOWN\n");
    	soft_gain_down();                  //话筒音量VOL-

        if(!wireless_mic_connected_sta_get()) {
            break;
        }
        wireless_tx_soft_gain_level(soft_gain_level_get(),soft_gain_level_is_max_min());
        break;

    case MSG_ECHO_LEVEL_UP:
        printf("MSG_ECHO_LEVEL_UP\n");

        if(!wireless_mic_connected_sta_get()) {
            break;
        }
		echo_delay_level_up();             //混响音量VOL+
        wireless_tx_echo_delay_level(echo_delay_level_get(),echo_delay_level_is_max_min());
        break;

    case MSG_ECHO_LEVEL_DOWN:
        printf("MSG_ECHO_LEVEL_DOWN\n");
        echo_delay_level_down();            //混响音量VOL-

        if(!wireless_mic_connected_sta_get()) {
            break;
        }
        wireless_tx_echo_delay_level(echo_delay_level_get(),echo_delay_level_is_max_min());
        break;

    case MSG_CHANGE_MAGIC:
        printf("MSG_CHANGE_MAGIC\n");

        if(!wireless_mic_connected_sta_get()) {
            break;
        }
        magic_audio_mute_set(1);             //mute 魔音，防连续切换导致复位关机
        magic_effect_level_change();         //设置魔音效果
        magic_audio_mute_set(0);

        wireless_tx_magic_effect_level(magic_effect_level_get());
        break;

    case MSG_VOICE_RM:
		printf("MSG_VOICE_RM\n");
        if(!wireless_mic_connected_sta_get()) {
            break;
        }

//        if (user_private_ctl_flag.voice_rm_en){
//            user_private_ctl_flag.voice_rm_en = false;
//        } else {
//            user_private_ctl_flag.voice_rm_en = true;
//        }
//        wireless_tx_voice_rm_en(user_private_ctl_flag.voice_rm_en); //消原音
        break;

    case MSG_TEST_X:
        printf("Test X triggered\n");
        break;

#if WARNING_MIC_MIX_WAV_PLAY_EN
    case EVT_TEST_WAV:
        printf("EVT_TEST_WAV\n");
        extern void mic_mix_play_wav(u32 addr, u32 len);
        mic_mix_play_wav(RES_BUF_TEST_ACCOMPLISH_WAV,RES_LEN_TEST_ACCOMPLISH_WAV);
        break;
#endif // WARNING_MIC_MIX_WAV_PLAY_EN    
    default:
        func_message(msg);
        break;
    }
}

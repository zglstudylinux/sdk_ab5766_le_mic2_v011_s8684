#include "include.h"
#include "wireless_sync_param.h"

static wireless_sync_prarm_t wireless_sync_prarm AT(.buf.adapter.proc);

/*****************************************************************************
 * Module    : 作用于发射端代码
 *****************************************************************************/
AT(.text.wireless_cmd.private_sync)
void wireless_tx_echo_delay_level(u8 echo_delay_level, u8 max_min_flag)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_ECHO_DELAY_LEVEL;
    pdu.buf[1] = echo_delay_level;
    pdu.buf[2] = max_min_flag;

    wireless_send_cmd(0, (u8 *)&pdu, 4);
}

AT(.text.wireless_cmd.private_sync)
void wireless_tx_soft_gain_level(u8 soft_gain_level, u8 max_min_flag)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_SOFT_GAIN_LEVEL;
    pdu.buf[1] = soft_gain_level;
    pdu.buf[2] = max_min_flag;

    wireless_send_cmd(0, (u8 *)&pdu, 4);
}

AT(.text.wireless_cmd.private_sync)
void wireless_tx_magic_effect_level(u8 magic_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_MAGIC_EFFECT_LEVEL;
    pdu.buf[1] = magic_level;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.private_sync)
void wireless_tx_mic_mute_level(u8 mic_mute_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_MIC_MUTE_LEVEL;
    pdu.buf[1] = mic_mute_level;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}

///通用动作互发函数
AT(.text.wireless_cmd.private_sync)
void wireless_tx_mutual_hair_act(u8 sync_num, u8 act)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = sync_num;
    pdu.buf[1] = act;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
    wireless_send_cmd(1, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.private_sync)
void wireless_tx_voice_rm_en(u8 voice_rm_en)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_VOICE_RM;
    pdu.buf[1] = voice_rm_en;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.usb)
void wireless_rx_mic_emit_private_sync_cmd(wireless_cmd_t *pdu)
{
//    u8 sub_cmd = pdu->buf[0];

//    printf("wireless_rx_mic_emit_private_sync_cmd, %d, %d\n", sub_cmd, pdu->buf[1]);
//    switch(sub_cmd) {
//        case PRIVATE_SYNC_ECHO_DELAY_LEVEL:
//            echo_delay_level_set(pdu->buf[1]);
//            break;
//
//        case PRIVATE_SYNC_SOFT_GAIN_LEVEL:
//            soft_gain_level_set(pdu->buf[1]);
//            break;
//
//        case PRIVATE_SYNC_MAGIC_EFFECT_LEVEL:
//            magic_effect_level_set(pdu->buf[1]);
//            break;
//
//        case PRIVATE_SYNC_MIC_MUTE_LEVEL:
//            mic_enc_mute_en_set(pdu->buf[1]);
//            break;
//
//        #if ADAPTER_UART_COMMAND_EN && WIRELESS_MIC_USER_UI_EN
//        case AB_SET_POWERONOFF:
//            mic_enc_mute_en_set(0);                             //TX恢复mic mute
//            soft_gain_level_set(SOFT_GAIN_DEFAULT_LEVEL - 1);   //TX恢复soft gain
//            break;
//        #endif
//
//        case PRIVATE_SYNC_ALL_PARAM:
//            echo_delay_level_set(pdu->buf[1]);
//            magic_effect_level_set(pdu->buf[2]);
//            soft_gain_level_set(pdu->buf[3]);
//            mic_enc_mute_en_set(pdu->buf[4]);
//            break;
//
//        default:
//            break;
//    }
}

/*****************************************************************************
 * Module    : 作用于接收端代码
 *****************************************************************************/
#if ADAPTER_SYNC_PARAM_EN
AT(.text.wireless_cmd.private_sync)
void wireless_sync_echo_delay_level(u8 echo_delay_level, u8 max_min_flag)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_ECHO_DELAY_LEVEL;
    pdu.buf[1] = echo_delay_level;
    pdu.buf[2] = max_min_flag;

    wireless_send_cmd(0, (u8 *)&pdu, 4);
    wireless_send_cmd(1, (u8 *)&pdu, 4);
}

AT(.text.wireless_cmd.private_sync)
void wireless_sync_soft_gain_level(u8 soft_gain_level, u8 max_min_flag)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_SOFT_GAIN_LEVEL;
    pdu.buf[1] = soft_gain_level;
    pdu.buf[2] = max_min_flag;

    wireless_send_cmd(0, (u8 *)&pdu, 4);
    wireless_send_cmd(1, (u8 *)&pdu, 4);
}

AT(.text.wireless_cmd.private_sync)
void wireless_sync_magic_effect_level(u8 magic_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_MAGIC_EFFECT_LEVEL;
    pdu.buf[1] = magic_level;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
    wireless_send_cmd(1, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.private_sync)
void wireless_sync_mic_mute_level(u8 mic_mute_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_MIC_MUTE_LEVEL;
    pdu.buf[1] = mic_mute_level;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
    wireless_send_cmd(1, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.private_sync)
void wireless_sync_all_param(void)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_SYNC_CMD;
    pdu.buf[0] = PRIVATE_SYNC_ALL_PARAM;
    pdu.buf[1] = wireless_sync_prarm.echo_delay_level;
    pdu.buf[2] = wireless_sync_prarm.magic_effect_level;
    pdu.buf[3] = wireless_sync_prarm.soft_gain_level;
    pdu.buf[4] = wireless_sync_prarm.mic_mute_en;

    wireless_send_cmd(0, (u8 *)&pdu, 6);
    wireless_send_cmd(1, (u8 *)&pdu, 6);
}

#endif
AT(.text.wireless_cmd.usb)
void wireless_rx_adapter_private_sync_cmd(wireless_cmd_t *pdu)
{
    u8 sub_cmd = pdu->buf[0];

//    printf("wireless_rx_adapter_private_sync_cmd, %d, %d, %d\n", sub_cmd, pdu->buf[1], pdu->buf[2]);
    switch(sub_cmd) {
        case PRIVATE_SYNC_ECHO_DELAY_LEVEL:
#if ADAPTER_SYNC_PARAM_EN
            wireless_sync_prarm.echo_delay_level = pdu->buf[1];
            wireless_sync_echo_delay_level(pdu->buf[1], pdu->buf[2]);
#endif
#if ADAPTER_SYNC_PARAM_MEMORY_EN
            param_echo_delay_level_write((u8 *)&pdu->buf[1]);
#endif
#if ADAPTER_UART_COMMAND_EN
            uart_command_tx_data(AB_ASK_VOICE,0);
#endif
            ///通过udet发送给560x
            break;

        case PRIVATE_SYNC_SOFT_GAIN_LEVEL:
#if ADAPTER_SYNC_PARAM_EN
            wireless_sync_prarm.soft_gain_level = pdu->buf[1];
            wireless_sync_soft_gain_level(pdu->buf[1], pdu->buf[2]);
#endif
#if ADAPTER_SYNC_PARAM_MEMORY_EN
            param_soft_gain_level_write((u8 *)&pdu->buf[1]);
#endif
#if ADAPTER_UART_COMMAND_EN
            if(!(pdu->buf[2])) {
                uart_command_tx_data(AB_ASK_VOICE,0);
            } else {
                uart_command_tx_data(AB_ASK_VOICE_MAX_MIN,0);
            }
#endif
            ///通过udet发送给560x
            break;

        case PRIVATE_SYNC_MAGIC_EFFECT_LEVEL:
#if ADAPTER_SYNC_PARAM_EN
            wireless_sync_prarm.magic_effect_level = pdu->buf[1];
            wireless_sync_magic_effect_level(pdu->buf[1]);
#endif
#if ADAPTER_SYNC_PARAM_MEMORY_EN
            param_magic_effect_level_write((u8 *)&pdu->buf[1]);
#endif
#if ADAPTER_UART_COMMAND_EN
            uart_command_tx_data(AB_ASK_VOICE,0);
#endif
            ///通过udet发送给560x
            break;

        case PRIVATE_SYNC_MIC_MUTE_LEVEL:
#if ADAPTER_SYNC_PARAM_EN
            wireless_sync_prarm.mic_mute_en = pdu->buf[1];
            wireless_sync_mic_mute_level(pdu->buf[1]);
#endif
#if ADAPTER_SYNC_PARAM_MEMORY_EN
            param_mic_mute_level_write((u8 *)&pdu->buf[1]);
#endif
#if ADAPTER_UART_COMMAND_EN
            uart_command_tx_data(AB_ASK_VOICE,0);
#endif
            ///通过udet发送给560x
            break;

        case PRIVATE_SYNC_VOICE_RM:
            ///通过udet发送给560x
#if ADAPTER_UART_COMMAND_EN
            uart_command_tx_data(AB_ASK_VOICE_RM,0);
#endif
            break;
        default:
            break;
    }
}

void wireless_sync_param_init(void)
{
    if (wireless_role_is_adapter()) {
        memset(&wireless_sync_prarm, 0, sizeof(wireless_sync_prarm_t));
#if ADAPTER_SYNC_PARAM_MEMORY_EN
        param_echo_delay_level_read((u8 *)&wireless_sync_prarm.echo_delay_level);
        param_magic_effect_level_read((u8 *)&wireless_sync_prarm.magic_effect_level);
        param_soft_gain_level_read((u8 *)&wireless_sync_prarm.soft_gain_level);
        param_mic_mute_level_read((u8 *)&wireless_sync_prarm.mic_mute_en);
#else
        wireless_sync_prarm.echo_delay_level = ECHO_DELAY_DEFAULT_LEVEL - 1;
        wireless_sync_prarm.magic_effect_level = MAGIC_EFFECT_DEFAULT_LEVEL;
        wireless_sync_prarm.soft_gain_level = SOFT_GAIN_DEFAULT_LEVEL - 1;
        wireless_sync_prarm.mic_mute_en = 0;
#endif
    }
}

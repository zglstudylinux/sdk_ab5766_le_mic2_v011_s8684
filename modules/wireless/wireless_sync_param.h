#ifndef _WIRELESS_SYNC_PARAM_H
#define _WIRELESS_SYNC_PARAM_H

//private control
typedef enum{
    ///发射端同步命令
    PRIVATE_SYNC_ECHO_DELAY_LEVEL      = 0,
    PRIVATE_SYNC_SOFT_GAIN_LEVEL,
    PRIVATE_SYNC_MAGIC_EFFECT_LEVEL,
    PRIVATE_SYNC_MIC_MUTE_LEVEL,
    PRIVATE_SYNC_VOICE_RM,

    ///同步所有的参数给发射端
    PRIVATE_SYNC_ALL_PARAM,
} sub_private_ctl_cmd_t;

typedef struct{
    u8 echo_delay_level;
    u8 magic_effect_level;
    u8 soft_gain_level;
    u8 mic_mute_en;
} wireless_sync_prarm_t;

void wireless_rx_mic_emit_private_sync_cmd(wireless_cmd_t *pdu);
void wireless_rx_adapter_private_sync_cmd(wireless_cmd_t *pdu);
void wireless_tx_mutual_hair_act(u8 sync_num, u8 act);
void wireless_sync_all_param(void);
void wireless_sync_param_init(void);
#endif

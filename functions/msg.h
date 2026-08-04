#ifndef _MSG_H
#define _MSG_H

//Event Message, 范围：0x700 ~ 0x7ff
enum{
    MSG_NO                  = 0,
    MSG_PWR_OFF,                    //按键关机
    MSG_PWR_HOLD,                   //PWR键按住
    MSG_PWR_RELEASE,                //PWR键释放
    MSG_CHANGE_MAGIC,
    MSG_ECHO_LEVEL_UP,
    MSG_ECHO_LEVEL_DOWN,
    MSG_VOICE_RM,
    MSG_VOL_UP,
    MSG_VOL_DOWN,
    MSG_VOL_MUTE,

    MSG_SYS_1S = 0x700,
    MSG_SYS_500MS,
    EVT_LPWR_OFF,
    EVT_ONLINE_SET_EQ,
    EVT_UART_COMMAND_PROC,
    EVT_PC_INSERT,
    EVT_PC_REMOVE,
    EVT_HCI_CMD,
    EVT_ONLINE_SET_EFFECT,
    EVT_ONLINE_SET_EFFECT2,
    EVT_CONNECT_BONDDING,
    EVT_LPWR_WARNING,
    EVT_LPWR_WARNING_EXIT,
    EVT_TEST_WAV,

    MSG_SYS_MAX = 0x7ff,
};

#endif

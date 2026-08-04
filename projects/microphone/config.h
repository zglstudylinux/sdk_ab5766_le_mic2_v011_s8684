/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_H
#define USER_CONFIG_H


//用户可选 CONFIG 方案配置
#define CONFIG_AB5766_LE_MIC        0          //搭配SDK_AB5766_LE_MIC的无线话筒\领夹麦， 默认打开领夹麦功能，适用PCB25-382的按键UI
#define CONFIG_AB5706A_LE_MIC       2          //搭配SDK_AB5706A_LE_MIC的无线话筒\领夹麦，适用PCB25-382的按键UI
#define CONFIG_AB5700_KBOX          3          //搭配SDK_AB5700_BOOMBOX的K歌音箱无线话筒，适用K歌话筒的按键UI


#define USER_CONFIG                 CONFIG_AB5766_LE_MIC

#if (USER_CONFIG == CONFIG_AB5766_LE_MIC)
    #include "config_ab5766_le_mic.h"
#elif (USER_CONFIG == CONFIG_AB5700_KBOX)
    #include "config_ab5700_kbox.h"
#elif (USER_CONFIG == CONFIG_AB5706A_LE_MIC)
    #include "config_ab5706a_le_mic.h"
#else
    #include "config_ab5766_le_mic.h"
#endif


#endif

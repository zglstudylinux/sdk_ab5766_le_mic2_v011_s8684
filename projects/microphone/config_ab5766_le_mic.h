/*****************************************************************************
 * Module    : Config
 * File      : config_ab5666_le_mic.h
 * Function  : 搭配AB5666的无线话筒配置文件
 *****************************************************************************/

#ifndef CONFIG_AB5766_LE_MIC_H
#define CONFIG_AB5766_LE_MIC_H
#include "config_define.h"

/*****************************************************************************
 * Module    : Function选择相关配置
 *****************************************************************************/
#define FUNC_LE_DUT_EN                  0                       //是否打开蓝牙DUT测试功能, 默认PA7单线串口，波特率9600

/*****************************************************************************
 * Module    : 系统功能选择配置
 *****************************************************************************/
#define SYS_CLK_SEL                     SYS_24M                 //选择系统时钟
#define BUCK_MODE_EN                    xcfg_cb.buck_mode_en    //是否BUCK MODE, 通过配置文件进行选择,0:LDO MODE  1:BUCK MODE
#define PMU_CFG_VBAT                    PMU_CFG_VBAT_4V2        //VBAT供电类型选择
#define WK0_10S_RESET                   0                       //WK pin 10秒复位功能
#define SYS_PWROFF_MODE                 PWROFF_MODE2            //软关机模式选择，模式1 VDDIO掉电，模式2 VDDIO不掉电
#define BSP_UART_DEBUG_EN               GPIO_PB3                //串口打印调试使能[Baud=1500000]
#define BSP_ADKEY_EN                    1                       //AD按键使能[Default: PB0]
#define BSP_IOKEY_EN                    0                       //IO按鍵使能[WK0:PP]
#define BSP_CHARGE_EN                   0                       //充电功能使能
#define BSP_CHARGE_BOX_EN               0                       //是否支持充电仓功能
#define BSP_TSEN_EN                     0                       //内置温度传感器采集功能
#define BSP_LED_EN                      1                       //LED灯控制使能, 配置工具的蓝灯跟红灯IO选一样时, 一个IO推两个灯
#define BSP_DLED_EN                     0                       //自定义一个IO推两个灯, 配置工具的蓝灯跟红灯IO选一样
#define BSP_VBAT_DETECT_EN              0                       //电池电量检测

/*****************************************************************************
 * Module    : FLASH配置
 *****************************************************************************/
#define FLASH_SIZE                      FSIZE_128K                      //芯片内置flash空间大小
#define FLASH_RESERVE_SIZE              CM_SIZE                         //FLASH保留区域大小
#define FLASH_CODE_SIZE                 (FLASH_SIZE-FLASH_RESERVE_SIZE) //程序可使用的空间大小
#define FLASH_ERASE_4K                  1                               //是否支持4K擦除
#define FLASH_DUAL_READ                 0                               //是否支持2线模式
#define FLASH_QUAD_READ                 0                               //是否支持4线模式
#define SPIFLASH_SPEED_UP_EN            1                               //SPI FLASH提速

#define CM_SIZE                         0x2000                          //参数区大小，至少8k(需4K对齐)
#define CM_START                        (FLASH_SIZE - CM_SIZE)          //参数区默认放在flash最后面
#define MAX_CM_PAGE                     2                               //根据需要定义大小,最大15个
#define SYS_CM_PAGE(x)                  PAGE0(x)
#define BLE_CM_PAGE(x)                  PAGE1(x)

/*****************************************************************************
 * Module    : MIC配置
 *****************************************************************************/
#define MIC_ANA_MODE                    xcfg_cb.mic_bias_method            //MIC 模式，底噪大小排序：省RC > 单端 > 差分
#define MIC_VDD_LVMODE                  0                                  //VDDMIC 模式，1:vddmic 1.9v, 0:vddmic 2.6v
#define MIC_IN_TRIM_TARGET              1250                               //省RC MIC IN trim的目标电压，0~vddmic mV
#define MIC_BIAS_RES_LEVEL              xcfg_cb.mic_bias_res               //MIC BIAS电阻=16K/MIC_BIAS_RES_LEVEL，范围：0~63，省RC内部自动trim电阻，单端mic一般配6，差分mic一般配置63
#define MIC_DIG_GAIN                    xcfg_cb.mic_dig_gain               //MIC 数字增益
#define MIC_ANA_GAIN                    xcfg_cb.mic_anl_gain               //MIC 模拟增益，省RC内部固定8.5dB
/*****************************************************************************
 * Module    : 无线mic功能选择配置
 *****************************************************************************/
//公共配置
#define WIRELESS_CON_CODEC_SEL                  CODEC_LC3S                //编解码选择，可选（adpcm/sbc）
#define WIRELESS_CON_VERS                       7                         //传输机制版本（2=v3，7=v8）
#define WIRELESS_CON_FREQ_BAND                  0                         //无线mic工作频段 0:2402M~2480M  2:2320M~2398M   4: 2482M~2560M
#define WIRELESS_CON_BONDING_EN                 0                         //组队绑定
#define WIRELESS_CON_RSSI_THR                   70                        //无线mic rssi阈值，只连接rssi阈值范围内的设备
#define WIRELESS_CON_PWR_CTR                    0                         //是否支持自动调节pwr，近距离降低，远距离增加
#define WIRELESS_MIC_26M_OSCILLATOR             0//xcfg_cb.oscillator_26m_en //是否使用26M晶振/时钟源进行工作,系统主频默认配置120M.烧录使用低速模式
#define WIRELESS_CON_CON_ID_SEL                 0x00                      //无线mic连接id，需要发射端跟接收端id一致才可连接，可以调用ble_set_con_id来动态修改id，实现动态连接
#define WIRELESS_CON_PAIR_MODE                  0                         //是否使能配对码功能(一拖二两个发射端连接id不同,需要配合WIRELESS_CON_CON_ID_SEL使用)
#define WIRELESS_DUMP_EN                        0                         //打印无线PER、RSSI等信息（调试用）

//无线MIC配置（上行音频，DEVICE --> ADAPTER）
#define WIRELESS_CON_LINK_NB                    2               //支持连几路无线麦
#define WIRELESS_MIC_SAMPLE_RATE_SELECT         SAMPLE_RATE_48K //无线mic采样率选择
#define WIRELESS_MIC_SAMPLES_SELECT             120             //无线mic每帧采样样点数选择  16k：20, ADPCM压缩后是samples*2/4+3
#define WIRELESS_MIC_CHANNEL_SELECT             1               //无线mic声道选择，1为单声道，2为双声道（目前只支持单声道）
#define WIRELESS_MIC_FRAME_SIZE                 25              //每帧压缩后大小（FRAME_SIZE*LINK_NB*RETRY_NB暂时不能超过72byte）
#define WIRELESS_MIC_RETRY_NB                   3               //重传次数（暂时支持1/2）
#define WIRELESS_MIC_TX_INTERVAL                4               //传输周期=n*1.25ms（默认值是2，改为4时传输次数可增加到3次）
#define WIRELESS_MIC_ENC_MAX_US                 600             //编码运算时长（单位us）
#define WIRELESS_MIC_DEC_MAX_US                 900             //解码运算时长（单位us，开PLC时约900us）

//设备端私有配置
#define WIRELESS_MIC_PARAM_MEMORY_EN            0               //无线mic音频参数关机保存功能，包括echo，magic，soft_gain，mic_mute等级效果
#define WIRELESS_MAX_POWER_DETECT_EN            0               //麦能量检测接口
#define WIRELESS_MIC_LOWPASS_EN                 0               //发射端是否开启SDADC低通滤波，恢复一些低频信号
#define WIRELESS_MIC_OMIT_XO_EN                 0               //是否使能发射端省晶振方案的配置
#define WIRELESS_MIC_LEDC_HW_EN                 0               //是否使能模块LEDC功能
#define WIRELESS_MIC_KEY_PRESS_MUTE_EN          0               //发射端按键按下抬起时，是否短暂mute掉对应声音，不拾音
#define WIRELESS_MIC_DAC_OUT_EN                 1               //发射端侦听麦采集使能
#define WIRELESS_MIC_CLOSE_EN                   0               //发射端是否关闭，只用于接收端优化对应空间
#define WARNING_MIC_MIX_WAV_PLAY_EN             0               //是否打开PCM资源文件混到MIC数据流中一起播放出来的功能

//设备端算法配置
#define WIRELESS_MIC_SOFT_GAIN_EN               1               //发射端是否开启 带保护的数字增益功能
#define WIRELESS_MIC_EQ_DRC_EN                  1               //发射端是否开启EQ_DRC功能
#define WIRELESS_MIC_EQ_DRC_DELAY               WIRELESS_MIC_EQ_DRC_EN*200//EQ_DRC运算时间
#define WIRELESS_MIC_ECHO_EN                    0               //发射端是否开启 ECHO音效
#define WIRELESS_MIC_ECHO_DELAY                 WIRELESS_MIC_ECHO_EN*200//ECHO运算时间
#define WIRELESS_MIC_MAGIC_EN                   0               //发射端是否开启 MAGIC音效
#define WIRELESS_MIC_MAGIC_DELAY                WIRELESS_MIC_MAGIC_EN*300//MAGIC运算时间
#define WIRELESS_FREQ_SHIFT2_EN                 0               //发射端是否开启移频（防啸叫）
#define WIRELESS_FREQ_SHIFT2_DELAY              WIRELESS_FREQ_SHIFT2_EN*170//移频运算时间
#define WIRELESS_MIC_ROOM_REVERB_EN             0               //发射端是否开启 房间混响
#define WIRELESS_MIC_ROOM_REVERB_DELAY          WIRELESS_MIC_ROOM_REVERB_EN*390//房间混响运算时间
#define WIRELESS_MIC_AGC_EN                     0               //发射端AGC算法使能 (调试中)
#define WIRELESS_MIC_AGC_DELAY                  WIRELESS_MIC_AGC_EN * 360
#define WIRELESS_MIC_ROBOTIZATION_EN            0               //发射端是否开启 机器人音效 (调试中)
#define WIRELESS_ALLPASS_FILTER_CHANGE_EN       0               //发射端是否开启 随机相位(防啸叫) (调试中)
#define WIRELESS_ALLPASS_FILTER_CHANGE_DELAY    WIRELESS_ALLPASS_FILTER_CHANGE_EN*200//随机相位运算时间 (调试中)
#define WIRELESS_MIC_DNR_FRE_EN                 0
#define WIRELESS_MIC_DNR_FRE_DELAY              WIRELESS_MIC_DNR_FRE_EN * 720   //发射端DNR_FRE运算时间
#define WIRELESS_MIC_32K_EN                     0               //是否使能mic 32k采样率
#define WIRELESS_MIC_SRC_DELAY                  WIRELESS_MIC_32K_EN*200//EQ_DRC运算时间
#define WIRELESS_MIC_AINS4_32K_EN               0

//适配器端私有配置
#define ADAPTER_DAC_OUTPUT_EN                   1               //适配器是否支持dac输出MIC音频
#define ADAPTER_USB_SPK_TX_EN                   0               //发射端是否开启usb spk功能（下行音频，不支持）
#define ADAPTER_USB_MIC_RX_EN                   1               //接收端是否开启usb mic功能（上行音频，不支持）
#define ADAPTER_MIX_DRC_EN                      1               //混音DRC功能(用于一拖二混音)
#define ADAPTER_MIX_DRC_DELAY                   ADAPTER_MIX_DRC_EN*250//mix drc运算时间
#define ADAPTER_FREQ_SHIFT_EN                   0               //适配器是否开启 移频（防啸叫）
#define ADAPTER_HUART_AUDIO_OUTPUT_EN           0               //适配器是否支持Huart音频输出
#define ADAPTER_UART_COMMAND_EN                 0               //适配器是否支持跟560x用uart传输私有命令
#define ADAPTER_HUART_UART_ONE_LINE             0               //适配器是否支持huart音频输出及uart传输私有命令统一用huart，单线双向传输
#define ADAPTER_SYNC_PARAM_EN                   0               //适配器是否控制两个mic的echo，magic，soft_gain，mic_mute等级效果，否则各个mic控制各自的音效等级
#define ADAPTER_SYNC_PARAM_MEMORY_EN            0               //适配器是否关机保存功能，需要跟ADAPTER_SYNC_PARAM_EN一起使用，不可与WIRELESS_MIC_PARAM_MEMORY_EN一起使用
#define ADAPTER_CLOSE_EN                        0               //适配器是否关闭
#define ADAPTER_DNR_FRE_EN                      0               //接收端是否打开 DNR_FRE  (调试中)

/*****************************************************************************
 * Module    : 算法参数配置
 *****************************************************************************/
//混响算法
#define ECHO_EN                                 (WIRELESS_MIC_ECHO_EN)
#define ECHO_LEVEL			                    70              //attenuation  range:0-90
#define ECHO_DRY_USER                           32767           //干度 range:0-32767
#define ECHO_WET_USER                           20000           //湿度 range:0-32767
#define ECHO_DELAY_MAX_LEVEL                    9               //echo延迟等级划分，按照固定的间隔划分成这么多等级，用于后续等级加减
#define ECHO_DELAY_DEFAULT_LEVEL                8               //echo第一次上电默认等级 range:1-ECHO_DELAY_MAX_LEVEL
#define ECHO_DELAY_BUF_SIZE                     6000            //echo缓存大小（单位：样点数），根据剩余空间配置大小
#define ECHO_ATTENUATION_MAX_LEVEL              16
//魔音算法
#define MAGIC_EN                                (WIRELESS_MIC_MAGIC_EN)
#define MAGIC_EFFECT_DEFAULT_LEVEL              0               //magic第一次上电默认效果等级，0:原声，1:男神，2:女神，3:娃娃音，4:魔兽

//魔音算法
#define ROBOTIZATION_EN                         (WIRELESS_MIC_ROBOTIZATION_EN)

//房间混响算法
#define ROOM_REVERB_EN                          (WIRELESS_MIC_ROOM_REVERB_EN)
#define ROOM_REVERB_LEVEL			            99              //衰减率range:0-100
#define ROOM_REVERB_DRY			                32767           //干度 range:0-65535 数值越大人声音越突出
#define ROOM_REVERB_WET			                26000           //湿度 range:0-32767 数值越大人混响效果越突出

//软件增益(新)
#define SOFT_GAIN_MAX_LEVEL                     8               // 64/32/16/8级音量表
#define SOFT_GAIN_DEFAULT_LEVEL                 0               //第一次上电默认等级，等级越大音量越小

//AGC算法
#define AGC_EN                                  (WIRELESS_MIC_AGC_EN)

//DNR FRE轻量降噪
#define DNR_FRE_EN                              (WIRELESS_MIC_DNR_FRE_EN || ADAPTER_DNR_FRE_EN)

//AINS4_32K降噪
#define AINS4_32K_EN                            (WIRELESS_MIC_AINS4_32K_EN)

/*****************************************************************************
 * Module    : usb device 功能选择
 *****************************************************************************/
#define UDE_STORAGE_EN                          0               //不支持
#define UDE_SPEAKER_EN                          ADAPTER_USB_SPK_TX_EN
#define UDE_HID_EN                              0
#define UDE_MIC_EN                              ADAPTER_USB_MIC_RX_EN
#define UDE_ENUM_TYPE                           (UDE_STORAGE_EN*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08)

/*****************************************************************************
 * Module    : 调音工具配置
******************************************************************************/
#define EQ_DRC_DBG_IN_UART                      1          //是否使能UART在线调节EQ
#define EFFECT_DBG_ADJUST_EN                    1          //是否使能音效离线调试
#define EFFECT_DBG_ADJUST_IN_UART               1          //是否使能UART在线调试音效,该功能需要打开EFFECT_DBG_ADJUST_EN和EQ_DBG_IN_UART

/*****************************************************************************
 * Module    : User按键配置 (可以同时选择多组按键)
 *****************************************************************************/
//按键通用配置
#define KEY_MAX_NB                              4           //按键数量（不包括IR）
#define DOUBLE_KEY_TIME                         (xcfg_cb.double_key_time)                       //按键双击响应时间（单位100ms）
#define PWRON_PRESS_TIME                        (500*xcfg_cb.pwron_press_time)                  //长按PWRKEY多长时间开机
#define PWROFF_PRESS_TIME                       xcfg_cb.pwroff_press_time                       //长按PWRKEY多长时间关机

/*****************************************************************************
 * Module    : 版本号管理
 *****************************************************************************/
#define SW_VERSION		"V0.0.1"		//只能使用数字0-9,ota需要转码

#include "config_extra.h"

#endif

#ifndef __CONFIG_EXTRA_H__
#define __CONFIG_EXTRA_H__

/*****************************************************************************
 * Module    : SDK版本配置
 *****************************************************************************/
#ifndef SDK_VERSION
    #define SDK_VERSION                 0x0110      //V0110
#endif

/*****************************************************************************
 * Module    : 系统功能配置
 *****************************************************************************/
#if BSP_ADKEY_EN || BSP_IOKEY_EN
    #define BSP_KEY_EN              1
#endif

///需要用到saradc trim mic，默认打开
#if 1//BSP_ADKEY_EN || BSP_VBAT_DETECT_EN || BSP_TSEN_EN
    #define BSP_SARADC_EN           1
#endif

#if (FLASH_RESERVE_SIZE % 4096) != 0
    #error "FLASH_RESERVE_SIZE is not a multiple of 4096!"
#endif

#if (AB_FOT_EN)
    #if (FOT_PACK_START % 4096) != 0
        #error "FOT_PACK_START is not a multiple of 4096!"
    #endif
#endif

#if !AB_FOT_EN
    #undef  AB_FOT_BLE_EN
    #define AB_FOT_BLE_EN   0
#endif

/*****************************************************************************
 * Module    : 线程配置
 *****************************************************************************/
#define OS_HEAP_SIZE    4*1024


/*****************************************************************************
 * Module    : 蓝牙相关配置
 *****************************************************************************/

#if (FUNC_LE_DUT_EN)
    #define LE_DUT_UART_EN          1
#else
    #define LE_DUT_UART_EN          0
#endif

#if LE_FCC_TEST_EN || LE_BQB_RF_EN
    #undef PWRON_PRESS_TIME
    #define PWRON_PRESS_TIME        0

    #undef WK0_10S_RESET
    #define WK0_10S_RESET           0
#endif

#if LE_FCC_TEST_EN || FUNC_LE_FCC_EN
        #if (LE_FCC_TEST_HSUART_IO_SEL == BSP_UART_DEBUG_EN)
        #error "LE FCC TEST: LE_FCC_TEST_HSUART_IO_SEL conflicts with BSP_UART_DEBUG_EN!\n"
        #endif  //(LE_FCC_TEST_HSUART_IO_SEL == BSP_UART_DEBUG_EN)
#endif


#if (FUNC_LE_FCC_EN && LE_FCC_TEST_EN)
#error "LE FCC TEST: The FUNC_LE_FCC_EN and LE_FCC_TEST_EN cannot be enabled at the same time!\n"
#endif

#if (FUNC_LE_BQB_RF_EN && LE_BQB_RF_EN)
#error "LE BQB RF TEST: The FUNC_LE_BQB_RF_EN and LE_BQB_RF_EN cannot be enabled at the same time!\n"
#endif

/*****************************************************************************
 * Module    : 无线mic功能选择配置
 *****************************************************************************/
#if WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_8K
    #define FRAME_SIZE_MIN                      10      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_16K
    #define FRAME_SIZE_MIN                      20      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_24K
    #define FRAME_SIZE_MIN                      30      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_32K
    #define FRAME_SIZE_MIN                      40      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_48K
    #define FRAME_SIZE_MIN                      60      //1.25ms
#else
    #error "WIRELESS_MIC_SAMPLE_RATE_SELECT is not allowed!"
#endif

#if WIRELESS_MIC_SAMPLES_SELECT < FRAME_SIZE_MIN
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif
#if (WIRELESS_MIC_SAMPLES_SELECT % FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif

#define WIRELESS_MIC_DFU_TX_INTERVAL            (WIRELESS_MIC_SAMPLES_SELECT/FRAME_SIZE_MIN)
#ifndef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            WIRELESS_MIC_DFU_TX_INTERVAL
#endif

#if WIRELESS_CON_VERS==0
    #define WIRELESS_CON_INTERVAL               1
    #define WIRELESS_CON_COMB_BUF_EN            1       //组合包
    #define WIRELESS_MIC_COMB_NB                (WIRELESS_CON_LINK_NB*FRAME_SIZE_MIN)/WIRELESS_MIC_SAMPLES_SELECT
#elif WIRELESS_CON_VERS==1
    #define WIRELESS_CON_INTERVAL               60
    #define WIRELESS_CON_COMB_BUF_EN            1       //组合包
    #define WIRELESS_MIC_COMB_NB                (WIRELESS_CON_LINK_NB*FRAME_SIZE_MIN)/WIRELESS_MIC_SAMPLES_SELECT
#elif WIRELESS_CON_VERS==2 || WIRELESS_CON_VERS==7
  #if WIRELESS_MIC_TX_INTERVAL == 12
    #define WIRELESS_CON_INTERVAL               60
  #elif WIRELESS_MIC_TX_INTERVAL == 8
    #define WIRELESS_CON_INTERVAL               64
  #else
    #define WIRELESS_CON_INTERVAL               60
  #endif
    #define WIRELESS_CON_COMB_BUF_EN            0       //单包
    #define WIRELESS_SPK_COMB_NB                1
  #if WIRELESS_MIC_TX_INTERVAL == 0 || (WIRELESS_MIC_TX_INTERVAL%WIRELESS_MIC_DFU_TX_INTERVAL) != 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #elif WIRELESS_MIC_TX_INTERVAL == 1                   //1.25ms传不了几byte，组合成2.5ms
    #undef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            2
    #define WIRELESS_MIC_COMB_NB                2
  #else
    #define WIRELESS_MIC_COMB_NB                (WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_DFU_TX_INTERVAL)
  #endif
  #if (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_COMB_NB)%(WIRELESS_MIC_TX_INTERVAL*FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #endif
  #if WIRELESS_CON_INTERVAL%WIRELESS_MIC_TX_INTERVAL != 0
    #error "WIRELESS_CON_INTERVAL is not allowed!"
  #endif
#endif // WIRELESS_CON_VERS

#if WIRELESS_MIC_COMB_NB == 0
    #undef WIRELESS_MIC_COMB_NB
    #define WIRELESS_MIC_COMB_NB                1
#endif

#if WIRELESS_MIC_ECHO_EN && WIRELESS_MIC_ROOM_REVERB_EN
    #undef WIRELESS_MIC_ECHO_DELAY
    #define WIRELESS_MIC_ECHO_DELAY              0
#endif

#if DUMP_HUART || ADAPTER_HUART_AUDIO_OUTPUT_EN || WIRELESS_MIC_EQ_DBG_EN || LE_DUT_UART_EN
    #define HUART_EN                            1
#else
    #define HUART_EN                            0
#endif

#if ADAPTER_SYNC_PARAM_MEMORY_EN
    #undef ADAPTER_SYNC_PARAM_EN
    #define ADAPTER_SYNC_PARAM_EN               1
#endif

#if ADAPTER_SYNC_PARAM_EN
    #undef WIRELESS_MIC_PARAM_MEMORY_EN
    #define WIRELESS_MIC_PARAM_MEMORY_EN        0
#endif

/*****************************************************************************
 * Module    : UART打印功能配置
 *****************************************************************************/
#if !BSP_UART_DEBUG_EN
    //关闭所以打印信息
    #undef printf
    #undef vprintf
    #undef print_r
    #undef print_r16
    #undef print_r32
    #undef printk
    #undef vprintk
    #undef print_kr
    #undef print_kr16
    #undef print_kr32

    #define printf(...)
    #define vprintf(...)
    #define print_r(...)
    #define print_r16(...)
    #define print_r32(...)
    #define printk(...)
    #define vprintk(...)
    #define print_kr(...)
    #define print_kr16(...)
    #define print_kr32(...)
#endif

/*****************************************************************************
 * Module    : MIC功能配置
 *****************************************************************************/
//同时打开adc 和 dac情况下 ，必须配置高压模式
#if WIRELESS_MIC_DAC_OUT_EN
    #undef MIC_VDD_LVMODE
    #define MIC_VDD_LVMODE              0
#endif


/*****************************************************************************
 * Module    : 无线配对码功能配置
 *****************************************************************************/
#if WIRELESS_CON_PAIR_MODE

#undef ADAPTER_SET_TX1_CON_ID              //此处设置的为RX端连接两个TX的配对码，需要在发射端分别设置对应的WIRELESS_CON_CON_ID_SEL配合使用
#undef ADAPTER_SET_TX2_CON_ID
#define ADAPTER_SET_TX1_CON_ID      0x05
#define ADAPTER_SET_TX2_CON_ID      0x00

#else

#undef ADAPTER_SET_TX1_CON_ID
#undef ADAPTER_SET_TX2_CON_ID
#define ADAPTER_SET_TX1_CON_ID      0x00
#define ADAPTER_SET_TX2_CON_ID      0x00


#endif
#endif // __CONFIG_EXTRA_H__


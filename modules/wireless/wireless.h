#ifndef _WIRELESS_H
#define _WIRELESS_H

#include "wireless_txrx.h"
#include "wireless_proc.h"
#include "wireless_cmd.h"
#include "../../os/os_thread.h"

#if WIRELESS_CON_COMB_BUF_EN
#define MIC_RX_BUFFER_SIZE              (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
#define MIC_TX_BUFFER_SIZE              (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)*WIRELESS_MIC_RETRY_NB
#else
#define MIC_RX_BUFFER_SIZE              (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
#define MIC_TX_BUFFER_SIZE              (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
#endif



void ble_con_var_init(void);
void bsp_ble_init(void);
void mic_enc_mute_en_set(uint mute_en);
u8 mic_enc_mute_en_get(void);
uint wireless_mic_init_get(void);
uint wireless_mic_connected_sta_get(void);
void bsp_26m_oscillator_init(void);

void mic_enc_key_press_mute_en_set(uint mute_en);
u8 mic_enc_key_press_mute_en_get(void);

void wireless_host_ch_class_set(void);

void wireless_mic_unbonding(void);      //断开连接和清除组队绑定接口

#if WIRELESS_DUMP_EN
void wireless_dump_init(void);
void wireless_dump_proc(void);
void wireless_dump_reset(u8 idx);
void wireless_dump_set_rx_status(u8 idx, u8 chidx, s8 rssi, u8 bfi);
void wireless_dump_set_chmap_cb(u8 idx, const u8 *chmap);
#else
    #define wireless_dump_init()
    #define wireless_dump_proc()
    #define wireless_dump_reset(idx)
    #define wireless_dump_set_rx_status(idx, chidx, rssi, bfi)
    #define wireless_dump_set_chmap_cb(idx, chmap)
#endif
#endif

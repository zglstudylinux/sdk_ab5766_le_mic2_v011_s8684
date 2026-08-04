/**********************************************************************
*
*   strong_ble.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

bool cfg_bt_hsec_wakeup_en       = false;
//uint8_t adpll_div_adjtype        = 0;//CHIP_VERS;

void ble_get_local_bd_addr(u8 *addr)
{
    memcpy(addr, xcfg_cb.le_addr, 6);
}

const uint8_t *bt_rf_get_param(void)
{
//    cfg_bt_rf_def_txpwr = xcfg_cb.bt_rf_pwrdec;

    if(xcfg_cb.bt_rf_param_en) {
//        cfg_bt_rf_def_txpwr = 0;
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    } else {
        if (bt_get_ft_trim_value(&xcfg_cb.rf_pa_gain)) {
            return (const uint8_t *)&xcfg_cb.rf_pa_gain;
        }
    }
    return NULL;
}

#if (!LE_DUT_UART_EN)
void hci_fcc_init(void){}

void ble_msg_send(void *param){}
void ble_dut_cmd_do(uint param){}
bool ble_hci_cmd(void *pdu) {return 0;}
uint8_t ble_test_start(void *pdu) {return 0;}
uint8_t ble_test_stop(void) {return 0;}
void ble_test_init(uint8_t init_type) {}
uint8_t vs_fcc_test_cmd(void *pdu) {return 0;}
int hci_le_rx_test_v1_cmd_func(void *pdu, uint16_t opcode) {return 0;}
int hci_le_tx_test_v1_cmd_func(void *pdu, uint16_t opcode) {return 0;}
int hci_le_rx_test_v2_cmd_func(void *pdu, uint16_t opcode) {return 0;}
int hci_le_tx_test_v2_cmd_func(void *pdu, uint16_t opcode) {return 0;}
int hci_le_test_end_cmd_func(void *pdu, uint16_t opcode) {return 0;}
int ble_test_end_ind_func(u16 mid, void *pdu, u16 did, u16 sid) {return 0;}
#endif

#if WIRELESS_CON_VERS == 2
void ble_con_acl_tx_done(uint8_t link, uint8_t tx_idx);
void ble_con_acl_rx_done(uint8_t link, uint8_t rx_idx);
void ble_con_evt_start_do(uint8_t link);
void ble_con_end_cb_v3(uint8_t link);
void ble_t2r_v3_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
void ble_t2r_v3_init(uint8_t init_type);
uint8_t ble_t2r_v3_start(uint16_t con_handle, uint8_t test_rx);
uint8_t ble_t2r_v3_stop(uint16_t con_handle);
void ble_t2r_v3_pwr_ctr(uint8_t index, uint8_t pwr_level);
uint16_t ble_t2r_v3_link_duration_get(uint8_t con_link_index);

AT(.com_text.ble.isr.con)
void ble_con_acl_tx_done_cb(uint8_t link, uint8_t rx_idx)
{
    ble_con_acl_tx_done(link, rx_idx);
}

AT(.com_text.ble.isr.con)
void ble_con_acl_rx_done_cb(uint8_t link, uint8_t rx_idx)
{
    ble_con_acl_rx_done(link, rx_idx);
}

AT(.com_text.ble.isr.con)
void ble_con_evt_start_do_cb(uint8_t link)
{
    ble_con_evt_start_do(link);
}

void ble_con_established_cb(uint8_t link)
{
//    ble_con_established_cb_v3(link);
}

AT(.com_text.ble.isr.con)
void ble_con_end_cb(uint8_t link)
{
    ble_con_end_cb_v3(link);
}

AT(.com_text.ble.isr.con)
void ble_con_anchor_ts_update_cb(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt)
{
    ble_t2r_v3_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
}

void ble_audio_init_cb(uint8_t init_type)
{
    return ble_t2r_v3_init(init_type);
}

uint8_t ble_audio_start_cb(uint16_t con_handle, uint8_t test_rx)
{
    return ble_t2r_v3_start(con_handle, test_rx);
}

uint8_t ble_audio_stop_cb(uint16_t con_handle)
{
    return ble_t2r_v3_stop(con_handle);
}

uint16_t ble_single_link_duration_get(void)
{
    return ble_t2r_v3_link_duration_get(0);
}

void ble_audio_pwr_ctr_set(uint8_t idx, uint8_t pwr_level)
{
    printf("ble_audio_pwr_ctr_set, %d, %d\n", idx, pwr_level);
    ble_t2r_v3_pwr_ctr(idx, pwr_level);
}
#else
void ble_t2r_v3_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v3_init(uint8_t init_type) {}
uint8_t ble_t2r_v3_start(uint16_t con_handle, uint8_t test_rx) {return 0;}
uint8_t ble_t2r_v3_stop(uint16_t con_handle) {return 0;}
#endif

#if WIRELESS_CON_VERS == 7
void ble_con_acl_tx_done(uint8_t link, uint8_t tx_idx);
void ble_con_acl_rx_done(uint8_t link, uint8_t rx_idx);
void ble_con_evt_start_do(uint8_t link);
void ble_con_end_cb_v8(uint8_t link);
void ble_t2r_v8_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
void ble_t2r_v8_init(uint8_t init_type);
uint8_t ble_t2r_v8_start(uint16_t con_handle, uint8_t test_rx);
uint8_t ble_t2r_v8_stop(uint16_t con_handle);
uint16_t ble_t2r_v8_link_duration_get(void);
void ble_t2r_v8_skip_first_set_enable(uint8_t index, bool enable);

AT(.com_text.ble.isr.con)
void ble_con_acl_tx_done_cb(uint8_t link, uint8_t rx_idx)
{
    ble_con_acl_tx_done(link, rx_idx);
}

AT(.com_text.ble.isr.con)
void ble_con_acl_rx_done_cb(uint8_t link, uint8_t rx_idx)
{
    ble_con_acl_rx_done(link, rx_idx);
}

AT(.com_text.ble.isr.con)
void ble_con_evt_start_do_cb(uint8_t link)
{
    ble_con_evt_start_do(link);
}

AT(.com_text.ble.isr.con)
void ble_con_end_cb(uint8_t link)
{
    ble_con_end_cb_v8(link);
}

void ble_con_established_cb(uint8_t link)
{
//    ble_con_established_cb_v8(link);
}

AT(.com_text.ble.isr.con)
void ble_con_anchor_ts_update_cb(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt)
{
    ble_t2r_v8_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
}

void ble_audio_init_cb(uint8_t init_type)
{
    return ble_t2r_v8_init(init_type);
}

uint8_t ble_audio_start_cb(uint16_t con_handle, uint8_t test_rx, void *param)
{
    return ble_t2r_v8_start(con_handle, test_rx);
}

uint8_t ble_audio_stop_cb(uint16_t con_handle)
{
    return ble_t2r_v8_stop(con_handle);
}

uint16_t ble_single_link_duration_get(void)
{
    return ble_t2r_v8_link_duration_get();
}

void ble_skip_first_set(uint8_t index, uint8_t enable)
{
    ble_t2r_v8_skip_first_set_enable(index, enable);
}

void ble_audio_pwr_ctr_set(uint8_t idx, uint8_t pwr_level)
{
    printf("ble_audio_pwr_ctr_set, %d, %d\n", idx, pwr_level);
}
#else
void ble_t2r_v8_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v8_init(uint8_t init_type) {}
uint8_t ble_t2r_v8_start(uint16_t con_handle, uint8_t test_rx) {return 0;}
uint8_t ble_t2r_v8_stop(uint16_t con_handle) {return 0;}
#endif

#if !ADAPTER_USB_MIC_RX_EN
void usb_init(void) {}
void ude_control_flow(void) {}
void ude_ep_reset(void) {}
void ude_isoc_tx_process(void) {}
void usb_isr(void) {}
void usb_dev_isr(void) {}
#endif

#if !ECHO_EN
void echo_delay_level_set(u8 delay_level){}
void echo_audio_mute_set(uint8_t mute){}
void echo_delay_level_down(void){}
void echo_delay_level_up(void){}
u8 echo_delay_level_get(void){ return 0; }
bool echo_delay_level_is_max_min(void){ return false; }
#endif

#if !ROOM_REVERB_EN
void room_reverb_audio_mute_set(uint8_t mute){}
#endif

#if !MAGIC_EN
void magic_effect_level_set(u8 effect_level){}
void magic_audio_mute_set(uint8_t mute){}
void magic_effect_level_change(void){}
u8 magic_effect_level_get(void){ return 0; }
#endif

#if WIRELESS_MIC_CLOSE_EN
void ble_page_init(uint8_t init_type) {}
void ble_scan0_set_param(uint intv, uint window) {}
void ble_scan0_set_enable(bool enable) {}
void ble_scan0_init_hook(void *params, uint8_t act_id) {}
uint ble_create_con(uint8_t *addr, uint8_t addr_type, uint16_t timeout) {return 0;}
uint ble_disconnect(uint8_t index) {return 0;}
bool ble_pscan_adv_unpack(void *pdu) {return 0;}
AT(.com_text.ble.isr.con)
bool ble_scan0_rx_ind(uint8_t scan_id, uint8_t rxdesc_idx) {return 0;}
void ble_scan_init(uint8_t init_type) {}
uint8_t ble_scan_stop(void) {return 0;}
uint8_t ble_scan_start(uint8_t act_id, void *pdu) {return 0;}
AT(.com_text.ble.isr.con)
uint8_t ble_scan_restart(void) {return 0;}
void *ble_get_scan_var(void) {return NULL;}
void *ble_get_ssync_var(void) {return NULL;}
void ble_init_init(uint8_t init_type) {}
uint8_t ble_init_start(void *params) {return 0;}
uint8_t ble_init_stop(void) {return 0;}
void *ble_get_init_var(void) {return NULL;}
AT(.com_text.ble.isr.con)
void ble_init_pkt_rx(uint8_t env_idx) {}
AT(.com_text.ble.isr.con)
void ble_init_pkt_tx(uint8_t env_idx) {}
#endif

#if ADAPTER_CLOSE_EN
void ble_pscan_init(uint8_t init_type) {}
void ble_pscan_set_interval(uint32_t interval) {}
void ble_w_pscan_ctrl(uint8_t scan_en) {}
void ble_con_adv_restore(uint8_t index) {}
bool ble_pscan_rx_ind(uint8_t act_id, uint8_t rxdesc_idx, uint *ret){return 0;}
bool ble_pscan_con_end(uint8_t link_id) {return 0;}
bool ble_pscan_end_ind(void *param) {return 0;}
void ble_t2r_v3_d2a_rx_instant_cb(void* elt){}
void ble_t2r_v3_d2a_dac_dma_instant_cb(void* elt){}
void ble_t2r_v3_rx_isr(uint8_t link_id, uint32_t timestamp) {}
void ble_d2a_rx_tick_alloc(uint8_t index, uint8_t link_id, bool con_sta) {}
void ble_d2a_rx_tick_set_sta(uint8_t index, bool con_sta) {}
void ble_d2a_rx_tick_free(uint8_t index) {}
bool ble_d2a_rx_tick_set(uint8_t index, uint tmr_hus) {return 0;}
uint8_t ble_adv_start(uint8_t act_id, void *pdu) {return 0;}
uint8_t ble_adv_stop(uint8_t act_id) {return 0;}
void ble_adv_adv_data_update(uint8_t act_id, uint8_t len, uint16_t buffer) {}
void ble_adv_scan_rsp_data_update(uint8_t act_id, uint8_t len, uint16_t buffer) {}
void ble_adv_rand_addr_update(uint8_t act_id, void *pdu) {}
void ble_adv_duration_update(uint8_t act_id, uint16_t duration) {}
void ble_adv_restart(uint8_t act_id, uint16_t duration, uint8_t max_ext_adv_evt) {}
void *ble_get_adv_var(void) {return NULL;}
void ble_adv_init(uint8_t init_type) {}
#endif // ADAPTER_CLOSE_EN


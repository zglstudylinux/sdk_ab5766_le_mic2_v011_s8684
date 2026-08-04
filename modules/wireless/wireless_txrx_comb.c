/*
 * 组合包发射BUF管理，编码的多个帧组合成一个大数据包发射，通过修改数据包内容实现重传
 *
 *                               -->|-------------|<--interval
 *                               -->|-------------|<--frame
 * LINK_NB=1, RETRY=1, COMB_NB=1时, a0,           a1,           a2,           a3,
 * LINK_NB=1, RETRY=2, COMB_NB=1时, a0a1,         a1a2,         a2a3,         a3a4,
 * LINK_NB=1, RETRY=3, COMB_NB=1时, a0a1a2,       a1a2a3,       a2a3a4,       a3a4a5,
 *
 *                               -->|---------------------------|<--interval
 *                               -->|---------------------------|<--frame
 * LINK_NB=2, RETRY=1, COMB_NB=1时, a0,           b0,           a1,           b1,
 * LINK_NB=2, RETRY=2, COMB_NB=1时, a0a1,         b0b1,         a1a2,         b1b2,
 * LINK_NB=2, RETRY=3, COMB_NB=1时, a0a1a2,       b0b1b2,       a1a2a3,       b1b2b3,
 *
 *                               -->|---------------------------|<--interval
 *                               -->|-------------|<--frame
 * LINK_NB=2, RETRY=1, COMB_NB=2时, a0a1,         b0b1,         a2a3,         b2b3,
 * LINK_NB=2, RETRY=2, COMB_NB=2时, a0a1a2a3,     b0b1b2b3,     a2a3a4a5,     b2b3b4b5,
 * LINK_NB=2, RETRY=3, COMB_NB=2时, a0a1a2a3a4a5, b0b1b2b3b4b5, a2a3a4a5a6a7, b2b3b4b5b6b7, a4a5a6a7a8a9
 ****************************************************************************************
 */
#include "include.h"
#include "wireless_txrx.h"

#if WIRELESS_CON_COMB_BUF_EN

typedef struct {
    u8 buf[MIC_RX_BUFFER_SIZE];
    u8 buf_ofs;
    u8 bfi;
//    u8 chidx;
//    s8 rssi;
//    u8 chstatus;
} ble_rxch_t;

typedef struct {
    u8 *txbuf;
    u8 buf_ofs;
    u8 first_pkt;
} ble_txch_t;

AT(.buf.mic_emit.proc)
static ble_txch_t ble_txch;
AT(.buf.adapter.proc)
static ble_rxch_t ble_rxch[WIRELESS_CON_LINK_NB][WIRELESS_MIC_RETRY_NB];


void mic_dec_dac_dma_kick(uint tick_cnt);
void mic_enc_adc_dma_kick(uint tick_cnt);

//------------------------------------------------------------------------------------------
AT(.com_text.ble.rx)
void adapter_rxpkt_init(u8 idx)
{
    for(uint i=0; i<WIRELESS_MIC_RETRY_NB; i++) {
        ble_rxch[idx][i].buf_ofs = 0;
        ble_rxch[idx][i].bfi    = 1;
    }
}

AT(.com_text.ble.tx)
void device_txpkt_init(void)
{
    ble_txch.txbuf = NULL;
    ble_txch.buf_ofs = 0;
    ble_txch.first_pkt = true;
}

//------------------------------------------------------------------------------------------
// DEVICE -> ADAPTER
//------------------------------------------------------------------------------------------

//RX回调函数，每次RX回调一次，bfi=1表示错帧
AT(.com_text.mic_dec)
void wireless_d2a_set_rxpkt_cb(u8 idx, u8 *ptr, u16 len, bool bfi, u8 chidx, s8 rssi)
{
#if WIRELESS_MIC_RETRY_NB > 1
    for(uint i=0; i<WIRELESS_MIC_RETRY_NB-1; i++) {
        ble_rxch_t *curr = &ble_rxch[idx][i];
        ble_rxch_t *next = &ble_rxch[idx][i+1];

        if(bfi == 0) {
            memcpy(curr->buf, ptr, MIC_RX_BUFFER_SIZE);
            curr->buf_ofs   = 0;
            curr->bfi       = bfi;
        } else if(next->bfi == 0) {
            memcpy(curr, next, sizeof(ble_rxch_t));
        } else {
            curr->buf_ofs   = 0;
            curr->bfi       = 1;
        }

        if((WIRELESS_MIC_RETRY_NB - i) == 2) {
            if(!bfi) {
                memcpy(next->buf, ptr+MIC_RX_BUFFER_SIZE, MIC_RX_BUFFER_SIZE);
            }
            next->buf_ofs   = 0;
            next->bfi       = bfi;
        } else {
            next->bfi       = 1;
        }

        ptr += MIC_RX_BUFFER_SIZE;
    }
#else
    ble_rxch_t *rxch = &ble_rxch[idx][0];

    if(!bfi) {
        memcpy(rxch->buf, ptr, len);
    }
    rxch->buf_ofs   = 0;
    rxch->bfi       = bfi;
#endif

    wireless_channel_status(idx,chidx, rssi, bfi, 0);
}

//从rxbuf获取一个数据帧
AT(.com_text.ble.rx)
u8 wireless_d2a_get_rx_frame(u8 idx, u8 *buf, uint size)
{
    ble_rxch_t *rxch = &ble_rxch[idx][0];
    u8 *ptr = rxch->buf + rxch->buf_ofs;
    u8 bfi = rxch->bfi;

#if WIRELESS_MIC_COMB_NB > 1
    if(rxch->buf_ofs <= size) {
        rxch->buf_ofs += size;
    }
#endif
    if(!bfi) {
        memcpy(buf, ptr, size);
    }
    return bfi;
}

//RX回调解码，每1.25ms触发一次
AT(.com_text.mic_dec)
void wireless_d2a_set_rxdec_cb(u8 idx, bool rxdone)
{
    kick_dec_prio_trans(idx);
}

//连接断开时，清除TX/RX状态标记
AT(.com_text.mic_dec)
void wireless_d2a_end_ind_cb(u8 idx)
{
    if(wireless_role_is_adapter()) {
        adapter_rxpkt_init(idx);
    } else {
        device_txpkt_init();
    }
}

//RX解码完，启动DAC_DMA
AT(.com_text.mic_dec)
void wireless_d2a_dac_dma_cb(uint tick_cnt)
{
    mic_dec_dac_dma_kick(tick_cnt);
}

//------------------------------------------------------------------------------------------
//TX前启动ADC采集
AT(.com_text.mic_enc)
void wireless_d2a_adc_dma_cb(uint tick_cnt)
{
    mic_enc_adc_dma_kick(tick_cnt);
}

//TX回调函数，每次TX前回调一次设置当前TX_BUF（实际上TX_BUF一直用同一个）
AT(.com_text.mic_enc)
void wireless_d2a_set_txbuf_cb(u8 *ptr, u16 len)
{
    if(ble_txch.txbuf == NULL) {
        ble_txch.first_pkt = true;
    }

    ble_txch.txbuf = ptr;

#if WIRELESS_MIC_COMB_NB > 1
    ble_txch.buf_ofs = WIRELESS_MIC_FRAME_SIZE * (WIRELESS_MIC_COMB_NB-1);
#endif
}

//保存一个数据帧到txbuf
AT(.com_text.ble.tx)
void wireless_d2a_put_tx_frame(u8 *ptr, u16 size)
{
    u8 *buf = ble_txch.txbuf;
    if(buf != NULL) {
        u8 *curr;

        u8 *retry = buf + size;
        uint remain = MIC_TX_BUFFER_SIZE - (size);

        if(ble_txch.first_pkt) {
            memset(buf, 0x00, MIC_TX_BUFFER_SIZE);
        }

        if(ble_txch.buf_ofs == 0) {
            //buf_ofs==0, 表示TX已经取完上一包，将后半段数据copy到前面去TX
            memcpy(buf, retry, remain);
        }

        //新的数据copy到后半段去TX
        curr = buf + remain + ble_txch.buf_ofs;

        ble_txch.first_pkt = false;

#if WIRELESS_MIC_COMB_NB > 1
        ble_txch.buf_ofs += size;
        if(ble_txch.buf_ofs >= size*WIRELESS_MIC_COMB_NB) {
            ble_txch.buf_ofs = 0;
        }
#endif

        memcpy(curr, ptr, size);
    }
}

//------------------------------------------------------------------------------------------
void wireless_con_adapter_init(void)
{
    if(wireless_role_is_adapter()) {
        for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
            adapter_rxpkt_init(i);
        }
    }
}

void wireless_con_device_init(void)
{
    if(!wireless_role_is_adapter()) {
        device_txpkt_init();
    }
}
#endif // WIRELESS_CON_COMB_BUF_EN

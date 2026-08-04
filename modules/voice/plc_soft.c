#include "include.h"
#include "plc_soft_api.h"


#define CH_NUM			2    //支持的通道数
static plc_proc_t m_st[CH_NUM] AT(.buf.plc_buf);
static u8 framesz;


AT(.text.plc_proc.pro)
void plc_soft_process(s16 *int_data, u32 samples, u8 bfi, u8 idx)//samples长度必须与FRMESIZE匹配
{
	plc_proc_t *st = &m_st[idx];

	plc_soft_process_do(int_data, samples, bfi, st);
}

AT(.text.plc_init)
void plc_soft_init(u8 idx, uint pkt_len, u8 sample_rate)
{
	plc_proc_t *st = &m_st[idx];
	memset((u8 *)st, 0, sizeof(plc_proc_t));

	if (sample_rate >= SAMPLE_RATE_16K) {
        st->pitch_min   = PITCH_MIN_8K;
        st->pitch_max   = PITCH_MAX_8K;
        st->pitchdiff   = PITCHDIFF_8K;
        st->poverlapmax = POVERLAPMAX_8K;
        st->historylen  = HISTORYLEN_8K;
        st->ndec        = NDEC_8K;
        st->corrlen     = CORRLEN_8K;
        st->corrbuflen  = CORRBUFLEN_8K;
        st->corrminpower = CORRMINPOWER_8K;
        st->eoverlapincr = EOVERLAPINCR_8K;
	} else {
        st->pitch_min   = PITCH_MIN;
        st->pitch_max   = PITCH_MAX;
        st->pitchdiff   = PITCHDIFF;
        st->poverlapmax = POVERLAPMAX;
        st->historylen  = HISTORYLEN;
        st->ndec        = NDEC;
        st->corrlen     = CORRLEN;
        st->corrbuflen  = CORRBUFLEN;
        st->corrminpower = CORRMINPOWER;
        st->eoverlapincr = EOVERLAPINCR;
	}

	st->history_ptr = st->history;
	st->pitchbuf_ptr = st->pitchbuf;
	st->lastq_ptr = st->lastq;
	framesz = pkt_len;
    plc_soft_init_do(st, pkt_len);
}

AT(.text.plc_exit)
void plc_soft_exit(u8 idx)
{
	plc_proc_t *st = &m_st[idx];

    st->enable = false;
    delay_5ms(2);       //delay要超过一帧的时间，确保disable时上一帧已经做完

	plc_soft_init_do(st, framesz);
}

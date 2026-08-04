#ifndef __OS_THREAD_
#define __OS_THREAD_

//------------------------------------------------------------------------------------------
//低优先级
enum {
    MSG_MIC_DEC0 = 0,
    MSG_MIC_DEC1,
    MSG_MIC_AINS4_32K,
};

void mic_dec_kick_cb(u8 msg);
void mic_alg_kick_cb(u8 msg);
#define kick_dec_prio_trans(idx)                   mic_dec_kick_cb(MSG_MIC_DEC0 + (idx))

#define robotization_mic_proc_kick_start()         mic_alg_kick_cb(MSG_MIC_ROBOTIZATION)
#define ains4_32k_mic_proc_kick_start()            mic_alg_kick_cb(MSG_MIC_AINS4_32K)

#endif // __THREAD_ALG_

#include "include.h"
#include "os_thread.h"

AT(.com_text.thread)
void thread_alg_proc_msg_cb(u32 msg)
{
    switch (msg) {
//        case MSG_MIC_DEC0:
//            mic_dec_prco_cb(0);
//            break;
//        case MSG_MIC_DEC1:
//            mic_dec_prco_cb(1);
//            break;
//
//        case MSG_MIC_AINS4:
//            ains4_mic_proc_cb();
//            break;

        case MSG_MIC_AINS4_32K:
            ains4_32k_mic_proc_cb();
            break;

#if ROBOTIZATION_EN
        case MSG_MIC_ROBOTIZATION:
            robotization_mic_proc_cb();
            break;
#endif
        default:
            break;
    }
}

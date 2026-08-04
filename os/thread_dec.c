#include "include.h"
#include "os_thread.h"

AT(.com_text.adapter.proc.thread)
void thread_dec_proc_msg_cb(u32 msg)
{
#if !ADAPTER_CLOSE_EN
    switch (msg) {
        case MSG_MIC_DEC0:
            mic_dec_prco_cb(0);
            break;
        case MSG_MIC_DEC1:
            mic_dec_prco_cb(1);
            break;

        default:
            break;
    }
#endif
}

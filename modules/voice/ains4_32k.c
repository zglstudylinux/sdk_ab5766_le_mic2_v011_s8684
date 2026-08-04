#include "include.h"
#include "api_alg.h"
#include "ains4_32k.h"

/*
 * 文件名称: ains4_32k.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.ains4_32k);
    AT(.rodata.ains4_32k)
    AT(.text.ains4_32k_proc)
    AT(.text.ains4_32k_init)

    注意mic_pcm_t 实际配置类型
    需要输入采样率为32000
 ****************************************************************************************
    code + rodata : 12k
    buf           : 11k
    time          :
 */

#if AINS4_32K_EN
typedef s16 mic_pcm_t;
#define TOG_BUF_EN                   1
#define AINS4_32K_INFO_PRINT         0
#define FRAME_LEN                    240                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          80                         //每次存取帧长

#if TOG_BUF_EN
static struct tog_bug_tag ains4_32k_tbuf AT(.buf.ains4_32k);                    //乒乓buf控制
static mic_pcm_t ains4_32k_cache_buf[FRAME_LEN*2] AT(.buf.ains4_32k);           //乒乓buf缓存
static mic_pcm_t ains4_32k_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ains4_32k);     //输出buf中转缓存
static void *ains4_32k_proc_ptr = ains4_32k_cache_buf;
#endif

static mic_pcm_t ains4_32k_cache_buf[FRAME_LEN*2] AT(.buf.ains4_32k);

static ains4_cb_t ains4_cb AT(.buf.ains4_32k);
static ains4_32k_mic_cfg_t ains4_32k_mic_cfg AT(.buf.ains4_32k);

#if AINS4_32K_INFO_PRINT
AT(.com_text.ains4_32k)
const char ains4_32k_info[] = "AINS4_32K_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ains4_32k_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(ains4_32k_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.ains4_32k_proc)
void ains4_32k_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{

    if (!ains4_32k_mic_cfg.mute /*&& wireless_cb.alg_en*/) {
#if TOG_BUF_EN
        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ains4_32k_tbuf, (u8 *)ains4_32k_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ains4_32k_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ains4_32k_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ains4_32k_proc_ptr = tog_bug_get_w_block(&ains4_32k_tbuf);
                tog_buf_wr_toggle(&ains4_32k_tbuf);
                ains4_32k_mic_cfg.kick_proc_done++;
                ains4_32k_mic_proc_kick_start();
            }

            memcpy(ptr, ains4_32k_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ains4_32k_mic_cfg.callback) {
                ains4_32k_mic_cfg.callback((void *)ptr, rlen, ch_mode, params);
            }

            samples -= rlen;
        }
#else

        if (samples == FRAME_LEN) {
        #if AINS4_32K_INFO_PRINT
            info_printf();
        #endif
//           ains4_process(rptr);

        }
#endif
    } else {
        if(ains4_32k_mic_cfg.callback) {
            ains4_32k_mic_cfg.callback((void *)ptr, samples, ch_mode, params);
        }
    }

}

//ains4_32k算法启动计算 放在低优先级现场处理
AT(.text.ains4_32k_proc)
void ains4_32k_mic_proc_cb(void)
{
#if TOG_BUF_EN
    mic_pcm_t *rptr = ains4_32k_proc_ptr;
#if AINS4_32K_INFO_PRINT
    info_printf();
#endif

    ains4_process(rptr);

    ains4_32k_mic_cfg.kick_proc_done--;
#endif
}

AT(.text.ains4_32k_set)
void ains4_32k_mic_output_callback_set(audio_callback_t callback)
{
    ains4_32k_mic_cfg.callback = callback;
}

AT(.text.ains4_32k_init)
void ains4_32k_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ains4_32k_mic_cfg, 0, sizeof(ains4_32k_mic_cfg));
#if TOG_BUF_EN
    tog_buf_init(&ains4_32k_tbuf, ains4_32k_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    ains4_32k_mic_param_set(0);

//    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
//    ains4_32k_mic_mute_set(1);

}

AT(.text.ains4_32k_exit)
void ains4_32k_mic_exit(void)
{

}

AT(.text.ains4_32k_set.param)
void ains4_32k_mic_param_set(s16 ains4_32k_nt)
{
    memset((uint8_t *)&ains4_cb, 0, sizeof(ains4_cb_t));

	//ains4_cb.nr_suppress			= 7;
	ains4_cb.denoiseBound		    = 1500;
	ains4_cb.overdrive				= 40960;
	ains4_cb.smooth_en				= 1;
	ains4_cb.modelUpdatePars0	    = 2;//0:no use HIST 1:only update one time first 2：always update
	//ains4_cb.gainHB_rd				= 32767;//0-32767
	ains4_cb.delta_k_up		        = 0;
#if 1
	ains4_cb.delta_k_down		    = 1310720;
#else
	ains4_cb.delta_k_down		    = 1310720;
#endif
	ains4_cb.prior_opt_idx           = 0;
	ains4_cb.low_fre_range           = 16;
	//ains4_cb.denoiseBound_fix		= 5;//
	//ains4_cb.yuan_en				= 0;
	ains4_cb.enr_thres				= 0;
	ains4_cb.spp_en					= 1;
#if 1
	ains4_cb.high_gain_len			= 64;
#endif
#if 0
	ains4_cb.high_gain_len			= 128;
#endif

    ains4_init(&ains4_cb);
}

AT(.text.ains4_32k_set.mute)
void ains4_32k_mic_mute_set(uint8_t mute)
{
    ains4_32k_mic_cfg.mute = mute;
    if (mute) {
        while(ains4_32k_mic_cfg.kick_proc_done){
            printf("#");
        }
#if TOG_BUF_EN
        tog_buf_init(&ains4_32k_tbuf, ains4_32k_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    }
}

AT(.text.ains4_32k_get.mute)
uint8_t ains4_32k_mic_mute_get(void)
{
    return ains4_32k_mic_cfg.mute;
}
#else
void ains4_32k_mic_proc_cb(void){}
#endif

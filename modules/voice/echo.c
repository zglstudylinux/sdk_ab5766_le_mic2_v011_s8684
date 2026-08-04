#include "include.h"
#include "echo.h"
#include "api_alg.h"
/*
 * 文件名称: echo.c
 * 功能描述: 本文件为软件ECHO处理模块
 * 暂时只支持48k采样率
 ****************************************************************************************
    code :
    buf  : 10000*2 Bytes
    time : 160M 下 120 个点处理 147us
 */
#if ECHO_EN

#if WIRELESS_MIC_32K_EN
#define ECHO_SAMPLE_RATE             32000
#define FRAME_LEN		             80
#else
#define ECHO_SAMPLE_RATE             48000
#define FRAME_LEN		             120
#endif // WIRELESS_MIC_32K_EN

#define MAX_DELAY_LENGTH             (ECHO_DELAY_BUF_SIZE/((ECHO_SAMPLE_RATE >> 1) * 0.001f))  //最大delay时间

#define UARTDUMP_ECHO_EN             0             //DUMP数据一键配置，需要使用定制上位机

#if UARTDUMP_ECHO_EN
static u8 dump_head_buf[14*2] AT(.buf.echo);
static u8 UartDump_buf[2][14 + FRAME_LEN*2] AT(.buf.echo);
static u8 UartDump_buf_temp[14 + FRAME_LEN*2] AT(.buf.echo);
static u8 huart_buffer_rx[128] AT(.buf.echo);
#endif

static echo_init_t echo_init_cfg AT(.buf.echo);        //初始化结构体
static echo_cfg_t echo_cfg AT(.buf.echo);          //管理模块结构体
s16 delay_lbuf[ECHO_DELAY_BUF_SIZE] AT(.buf.echo.delay);

AT(.text.echo_proc.input)
void echo_audio_process(s16 *ptr, u32 samples)
{
    s16 *rptr = ptr;
    if(!echo_cfg.mute){
        for(u8 i=0;i<samples;i++){
            echo_process_16bit(rptr+i);
        }
    }
}

//只有本地麦在线，没有无线麦链接时在此接口处理echo音效
AT(.text.echo_proc.input)
void alg_echo_audio_input(u8 *ptr, u32 samples, int ch_mode, void *param)
{
#if UARTDUMP_ECHO_EN
    s16 *ptr_temp = (s16 *) UartDump_buf_temp;
    dump_dma_wait();
    dump_put2ram(&UartDump_buf[0][0],ptr,FRAME_LEN*2,0); //算法前数据组包

    dump_put2ram(&UartDump_buf[1][0],ptr_temp,FRAME_LEN*2,1);//算法后数据组包
    dump_dma_kick(&UartDump_buf[0][0], (FRAME_LEN*2 + 14) * 2); //最后再一起吐数据，避免前后kick数据打架
#endif

    s16 *rptr = (s16 *)ptr;
    if (!echo_cfg.mute) {
        if (samples) {
            for (u8 i=0;i<samples;i++) {
                echo_process_16bit(rptr+i);
            }
        }
    }

#if UARTDUMP_ECHO_EN
    for (int i = 0; i < FRAME_LEN; i++) {
        ptr_temp[i]  = rptr[i];
    }
#endif // UARTDUMP_ECHO_EN
}

AT(.text.echo_proc.input)
void echo_audio_input(u8 *ptr, u32 samples, int ch_mode, void *param)
{
    s16 *rptr = (s16 *)ptr;
    if (!echo_cfg.mute) {
        if (samples) {
            for (u8 i=0;i<samples;i++) {
                echo_process_16bit(rptr+i);
            }
        }
    }

    if (echo_cfg.callback) {
        echo_cfg.callback((u8 *)rptr, samples, ch_mode, param);
    }
}

AT(.text.echo_set.callback)
void echo_audio_output_callback_set(audio_callback_t callback)
{
    echo_cfg.callback = callback;
}

AT(.text.echo_set.mute)
void echo_audio_mute_set(uint8_t mute)
{
    echo_cfg.mute = mute;

    if(echo_cfg.mute) {
        memset(delay_lbuf, 0, sizeof(delay_lbuf));
    }
}



AT(.text.echo_set)
void echo_delay_len_set(u16 delay_len)
{
    echo_audio_param_set(ECHO_LEVEL, delay_len, 1000, 0, ECHO_DRY_USER, ECHO_WET_USER);
}

AT(.text.echo_set)
void echo_delay_level_change(void)
{
    echo_cfg.delay_level += 1;
    if (echo_cfg.delay_level >= ECHO_DELAY_MAX_LEVEL) {
        echo_cfg.delay_level = 0;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_set)
void echo_delay_level_up(void)
{
    if (echo_cfg.delay_level < (ECHO_DELAY_MAX_LEVEL - 1)) {
        echo_cfg.delay_level += 1;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_set)
void echo_delay_level_down(void)
{
    if (echo_cfg.delay_level) {
        echo_cfg.delay_level -= 1;
    }
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_get)
u8 echo_delay_level_get(void)
{
    return echo_cfg.delay_level;
}

AT(.text.echo_get)
bool echo_delay_level_is_max_min(void)
{
    if (echo_cfg.delay_level == (ECHO_DELAY_MAX_LEVEL - 1) || echo_cfg.delay_level == 0) {
        return true;
    }
    return false;
}

AT(.text.echo_set)
void echo_delay_level_set(u8 delay_level)
{
    if (delay_level > (ECHO_DELAY_MAX_LEVEL - 1)) {
        delay_level = ECHO_DELAY_MAX_LEVEL - 1;
    }
    echo_cfg.delay_level = delay_level;
    echo_delay_len_set(echo_cfg.delay_level*echo_cfg.delay_len_step);
}

AT(.text.echo_set)
void echo_attenuation_set(u16 attenuation)
{
    echo_audio_param_set(attenuation, (ECHO_DELAY_DEFAULT_LEVEL - 1)*echo_cfg.delay_len_step, 1000, 0, ECHO_DRY_USER, ECHO_WET_USER);
}

AT(.text.echo_set)
void echo_attenuation_level_set(u8 attenuation_level)
{
    if (attenuation_level > (ECHO_ATTENUATION_MAX_LEVEL - 1)) {
        attenuation_level = ECHO_ATTENUATION_MAX_LEVEL - 1;
    }
    echo_cfg.attenuation_level = attenuation_level;
    //可以改成用表的形式听感会好一些
    echo_attenuation_set(echo_cfg.attenuation_level * (90 / (ECHO_ATTENUATION_MAX_LEVEL - 1)) );
}

AT(.text.echo_set)
void echo_attenuation_level_change(void)
{
    echo_cfg.attenuation_level += 1;
    if (echo_cfg.attenuation_level >= ECHO_ATTENUATION_MAX_LEVEL) {
        echo_cfg.attenuation_level = 0;
    }
    echo_attenuation_level_set(echo_cfg.attenuation_level);
}

/// attenuation:    混响次数  0~90
/// delay_length:   每一声混响之间的间隔  0~400
/// cutoffFreq_set: 低通滤波器的截止频率  1~4000
/// lp_filter_en:   低通滤波器的开关      0：关 1：开
/// dry_set:        原始声音               0~49152（Q15即0~1.5）
/// wet_set:        回声湿度，越大所占比例越大  0~32768（Q15 即0~1.0）
/// 默认配置 echo_audio_param_set(55,250,1000,1,32768,15000)
AT(.text.echo_set.param)
void echo_audio_param_set(u16 attenuation ,s32 delay_length ,u32 cutoffFreq_set ,u32 lp_filter_en , u16 dry_set ,u16 wet_set)
{
    if (attenuation >= 90) {
        attenuation = 90;
    }

    if (delay_length >= echo_cfg.max_delay_len) {
        delay_length = echo_cfg.max_delay_len;
    }

    echo_init_cfg.skip_flag         = 1;
    echo_init_cfg.lp_filter_en      = lp_filter_en;
    echo_init_cfg.attenuation_set   = attenuation;
    echo_init_cfg.delay_set         = delay_length;
    echo_init_cfg.dry               = dry_set;
    echo_init_cfg.wet               = wet_set;
    echo_init_cfg.cutoffFreq_set    = cutoffFreq_set;
#if WIRELESS_MIC_32K_EN
    echo_init_cfg.sample_rate       = SAMPLE_RATE_32K;
#else
    echo_init_cfg.sample_rate       = SAMPLE_RATE_48K;
#endif // WIRELESS_MIC_32K_EN
//    echo_init_cfg.delay_lbuf_set    = delay_lbuf;

//    echo_update_param_16bit_32k(&echo_init_cfg, 0);

    echo_update_param_16bit(&echo_init_cfg, 0);


}

AT(.text.echo_set)
void echo_audio_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&echo_cfg, 0, sizeof(echo_cfg_t));
    memset(&echo_init_cfg, 0, sizeof(echo_init_t));
    memset(delay_lbuf, 0, sizeof(delay_lbuf));

    //管理管理模块初始化设置
    echo_cfg.sample_rate            = sample_rate;
    if (echo_cfg.sample_rate != SAMPLE_RATE_48K && echo_cfg.sample_rate != SAMPLE_RATE_32K ) {
        printf("Notice Samples config err! ");
    }
    echo_cfg.max_delay_len          = MAX_DELAY_LENGTH;
    echo_cfg.delay_len_step         = echo_cfg.max_delay_len/(ECHO_DELAY_MAX_LEVEL - 1); //delay挡位步进
    echo_cfg.delay_level            = ECHO_DELAY_DEFAULT_LEVEL - 1; //初始化挡位设置

    //音效参数初始化设置
    echo_init_cfg.skip_flag         = 1;
    echo_init_cfg.lp_filter_en      = 0;
    echo_init_cfg.attenuation_set   = ECHO_LEVEL;
    echo_init_cfg.delay_set         = echo_cfg.delay_level * echo_cfg.delay_len_step;
    echo_init_cfg.dry               = ECHO_DRY_USER;
    echo_init_cfg.wet               = ECHO_WET_USER;
    echo_init_cfg.cutoffFreq_set    = 1000;
    echo_init_cfg.delay_lbuf_set    = delay_lbuf;
#if WIRELESS_MIC_32K_EN
    echo_init_cfg.sample_rate       = SAMPLE_RATE_32K;
#else
    echo_init_cfg.sample_rate       = SAMPLE_RATE_48K;
#endif // WIRELESS_MIC_32K_EN
    echo_init_16bit(&echo_init_cfg);

#if UARTDUMP_ECHO_EN
    memset(dump_head_buf, 0, 14*2);
    memset(UartDump_buf, 0, 2*(14 + FRAME_LEN*2));

    huart_t huart0;
    memset(&huart0, 0x00, sizeof(huart0));
    huart0.rx_port = HUART_TR_PB3;        //HUART串口选择: PA15: 0, PB2: 1, PB3: 2, PB4: 3, PB15: 4, PE10: 5, PE13: 6, VUSB: 7
    huart0.tx_port = HUART_TR_PB3;
    huart0.rxbuf_loop = 1;
    huart0.rxbuf      = huart_buffer_rx;
    huart0.rxbuf_size = 128;
    huart0.txisr_en = 1;
    huart0.rxisr_en = 0;
    huart_init(&huart0, 8000000);        //波特率设置默认是1.5MHz

    dump_buf_init(dump_head_buf,2, huart_tx, huart_wait_txdone);
#endif
}

AT(.text.echo_exit)
void echo_audio_exit(void)
{
}



#endif //ECHO_EN

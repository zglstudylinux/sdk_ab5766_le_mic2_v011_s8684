#ifndef __API_ALG_H
#define __API_ALG_H

//乒乓buffer
struct rw_tog {
    u8 *buf;
    u16 w_cnt;
    u16 r_cnt;
};

struct tog_bug_tag {
    u16 block_size;
    bool w_idx;
    bool r_idx;
    struct rw_tog tog[2];
};

void tog_buf_init(struct tog_bug_tag *tbuf, void *buffer, u16 block_size);  //初始化tog_buf，buffer size = 2*block_size
bool tog_buf_put(struct tog_bug_tag *tbuf, u8 *ptr, u16 size);              //写入到w_idx指向的buffer，写完w_cnt计数，w_cnt>=block_size时返回true
bool tog_buf_get(struct tog_bug_tag *tbuf, u8 *ptr, u16 size);              //获取r_idx指向的buffer，读完r_cnt计数，r_cnt>=block_size时返回true
void tog_buf_wr_toggle(struct tog_bug_tag *tbuf);                           //手动翻转w_idx
void tog_buf_rd_toggle(struct tog_bug_tag *tbuf);                           //手动翻转r_idx
void *tog_buf_get_wr_ptr(struct tog_bug_tag *tbuf);                         //获取当前写指针位置
void *tog_buf_get_rd_ptr(struct tog_bug_tag *tbuf);                         //获取当前读指针位置
#define tog_bug_get_w_block(tbuf)   (tbuf)->tog[(tbuf)->w_idx].buf          //获取当前写block指针
#define tog_bug_get_w_cnt(tbuf)     (tbuf)->tog[(tbuf)->w_idx].w_cnt        //获取当前写计数
#define tog_bug_get_r_cnt(tbuf)     (tbuf)->tog[(tbuf)->r_idx].r_cnt        //获取当前读计数

///------------------------------------------------------------------------------------------
///magic 算法结构体以及相关API声明
#define CircleBufo_Len		1536//0.032*Fs

typedef struct {
//Common use
	s32 fre_shift;
	u32 Fs;
	u32 pitch_levels;
	s32 ph1;
	s32 ph2;
	s16 pWtite1;
	s16 pRead1;
	s16 pRead2;
	s16 CircleBuf[CircleBufo_Len];
} pitch_shift2_cb_t;


void pitch_shift2_init(s32 SampleRate, s32 shift, pitch_shift2_cb_t *st);
void pitch_shift2_process(s16 *data, pitch_shift2_cb_t *st);

///------------------------------------------------------------------------------------------
///ECHO 算法结构体以及相关API声明
/*初始化参数结构体*/
typedef struct {
    u8   skip_flag;			/*开启后相同buf最大delay可多一倍*/
	u8   lp_filter_en;       /*滤波器使能*/
	u8   attenuation_set;    /*1-9 ->10percent-90percent*/
	u16  delay_set;          /*1-500ms*/
    u16  dry;
    u16  wet;
    s16 *delay_lbuf_set;
    u32  cutoffFreq_set;     /*截至频率选择*/
	u8 sample_rate;   //采样率
} echo_init_t;

void echo_init_16bit(echo_init_t *p);
void echo_update_param_16bit(echo_init_t *p, u8 fade_en);
void echo_process_16bit(s16 *ldata);

///------------------------------------------------------------------------------------------
///ROOM_REVERB 算法结构体以及相关API声明
typedef struct
{
	u16 damping_set;        /*高频阻尼，去除高频部分的齿音，擦类的刺耳声*/
	u16 decay_set;          /*衰减率*/
	u16 wet;
	u16 dry;
	u32 samples;
	u8 hpl;                 /*高通滤波器等级0表示关闭,1-10:50hz-500hz,步进50hz*/
}rvb_init_sb;

void room_reverb_process(s16 *sample, u8 idx);
void reverb_buf_init(rvb_init_sb *p, u8 idx);

///------------------------------------------------------------------------------------------
///allpass_filter_change算法结构体以及相关API声明
void allpass_filter_change_init(u8 idx, s32 change_time);
s32 allpass_filter_change(s32 input, u8 idx);

///---------------------------------------------------------------------------------------------
///AGC 算法结构体以及相关API声明
typedef struct {
	u32 sampleHz;           //语音采样率
	u8  frameMs;             //一帧语音长度，以毫秒为单位
	u16 frameSh;            //一帧short的个数， 默认一帧10ms
	u16 frameSh_true;
	s16 CompressiondB;
	s16 TargetdBfs;
	u8  limiterEnable;		//是否打开限压器
	u8  frame_2p5ms_en;
	u8	inbits24_en;
	u8 	smooth_en;
	u8  agcMode;			//AGC模式
} agc_cb_t;
void agcInit_do(agc_cb_t* agc_cb);
void AgcProcess_do(s16 *input, s32 pcmLen);

///---------------------------------------------------------------------------------------------
///dnr fre 算法结构体以及相关API声明
typedef struct {

	//s32 noise_db2;
	//s32 noise_db3;
	s16 denoiseBound;
	s32 overdrive;
	u8  smooth_en;
	s64 enr_thres;
	s16 smooth_v;
	u8  enr_mean_max_en;
	s16 music_lev;
	s32 enr_nr_thr;
	s16 low_fre_range;
	s16 prior_opt_idx;
	s32 in_attack;
	s32 in_release;
	s32 fs;
	s32 noise_init;
	//u16 pitch_filter_range;
} dnr_fre_cb_t;

void dnr_fre_init(dnr_fre_cb_t *dnr_fre_cb);
int dnr_fre_process(s16 *data, s32 *data_32bits, u8 in_24bits_en);

///---------------------------------------------------------------------------------------------
///32k ains4 算法结构体以及相关API声明
typedef struct {
	//s32 nr_suppress;
	s16 denoiseBound;
	s32 overdrive;
	u8  smooth_en;
	u8  modelUpdatePars0;
	u8  delta_k_up;
	s32 delta_k_down;
	s16 prior_opt_idx;
	u8  low_fre_range;
	s32 enr_thres;
	u8  spp_en;
	u16 high_gain_len;
} ains4_cb_t;
void ains4_init(ains4_cb_t *ains4_cb);
void ains4_process(s16 *data);

///------------------------------------------------------------------------------------------
///软件src算法结构体以及相关API声明
typedef struct{
	u8  src_en;
	u8  resv_en;
	int samplerate_in;
	int samplerate_out;
	int phase_comp;
	int phase_i;
	int phase_o;
	int phase_sum;
} srccon_t;

extern s16 pcm_48k[130];
void src_init(u32 ch_index, u32 spr_in, u32 spr_out);
int src_frame_resample(u32 ch_index, short *src_in, short *src_out, int in_cnt);
int src_frame_resample_24bit(u32 ch_index, int *src_in, int *src_out, int in_cnt);
void src_phase_comp_set(u32 ch_index, int phase);
#endif // __API_CODEC_H


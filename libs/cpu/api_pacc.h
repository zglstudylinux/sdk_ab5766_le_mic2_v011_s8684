#ifndef _API_PACC_H
#define _API_PACC_H

#define MDRC_LP_BAND            2
#define MDRC_BP_BAND            4
#define MDRC_HP_BAND            2

#define EQ_COEF_LEN(band)       (20*band+4)
#define EQ_ZX_LEN(band)         (8*band+8)
#define DYEQ_COEF_LEN           96
#define DYEQ_ZX_LEN             144
#define DRC_COEF_LEN            80
#define FIR_COEF_LEN(order)     (4*order)
#define FIR_ZX_LEN(order)       (4*order)
#define MBDRC_ZX_LEN            (3*DRC_COEF_LEN + 4*EQ_ZX_LEN(2) + 2*EQ_ZX_LEN(4))
#define MBDRC_COEF_LEN          (4*EQ_COEF_LEN(2) + 2*EQ_COEF_LEN(4))


//pacc
typedef enum{
    S16_Q15_SAT = 0,
    S32_Q23_SAT,
    S32_Q15,
    S32_Q23,
} pacc_fmt_t;

typedef struct pacc_cs_tag {
    u8 buf[36];
} pacc_cs_t;

//-----------------------------------------------------------------------------------
//pacc eq/drc
typedef struct {
    int threshold;
    int gain;
    int slope;
} drc_coeff_t;

typedef struct __attribute__((packed)){
    int in_attack;
    int in_release;
    int out_attack;
    int out_release;
    int in_level;
    int out_gain;
    int tav;
    int in_rms;
    drc_coeff_t coeffs[4];
} drc_coef_t;

//常用EQ数据格式
#define EQ_BAND_MAX_NB          10           //最大支持的band数
#define EQ_MAX_CH               1           //使用change时需要2个
#define EQ_ZX_BUF_SIZE          EQ_ZX_LEN(EQ_BAND_MAX_NB)*(EQ_MAX_CH)
#define EQ_COEF_BUF_SIZE        EQ_COEF_LEN(EQ_BAND_MAX_NB)*(EQ_MAX_CH)
typedef struct {
    struct {
        s32 buf[EQ_MAX_CH][2*(EQ_BAND_MAX_NB+1)];
    } zx;   //zx_buf
    struct {
        s32 buf[EQ_MAX_CH][5*EQ_BAND_MAX_NB+1];
    } coef; //coef_buf
} pacc_eq_bd_t;

//常用DRC数据格式
#define DRC_ZX_BUF_SIZE         DRC_COEF_LEN
#define DRC_COEF_BUF_SIZE       0
typedef struct {
    drc_coef_t drc_coef;
} pacc_drc_bd_t;

//常用MDRC数据格式
typedef struct {
    struct {
        drc_coef_t drc_coef[3];
        struct {
            s32 bq0[2*(MDRC_LP_BAND+1)];
            s32 bq1[2*(MDRC_LP_BAND+1)];
        } lp;
        struct {
            s32 bq0[2*(MDRC_BP_BAND+1)];
            s32 bq1[2*(MDRC_BP_BAND+1)];
        } bp;
        struct {
            s32 bq0[2*(MDRC_HP_BAND+1)];
            s32 bq1[2*(MDRC_HP_BAND+1)];
        } hp;
    } zx;   //zx_buf
    struct {
        struct {
            s32 bq0[5*MDRC_LP_BAND+1];
            s32 bq1[5*MDRC_LP_BAND+1];
        } lp;
        struct {
            s32 bq0[5*MDRC_BP_BAND+1];
            s32 bq1[5*MDRC_BP_BAND+1];
        } bp;
        struct {
            s32 bq0[5*MDRC_HP_BAND+1];
            s32 bq1[5*MDRC_HP_BAND+1];
        } hp;
    } coef; //coef_buf
} pacc_mdrc_bd_t;

//常用DYEQ数据格式
#define DYEQ_BAND_MAX_NB    1
typedef struct {
    struct {
        drc_coef_t drc_coef;
        s32 buf[2*(DYEQ_BAND_MAX_NB + 1)][4];
    } zx;       //zx_buf
    struct {
        s32 buf[4][5*DYEQ_BAND_MAX_NB+1];
    } coef;     //coef_buf
} pacc_dyeq_bd_t;

//-----------------------------------------------------------------------------------
//freq_shift
#define HIBERT_ORDER                                  160
typedef struct {
    s32 zx[HIBERT_ORDER];
    s32 Hilbert_hcoef[HIBERT_ORDER];
    s32 phase_step;
} freq_shift_bd_t;

//pacc api
typedef void (*pacc_done_callback_t)(void);
void pacc_ctl_set_done_callback(void *p_ctl, pacc_done_callback_t callback); //设置PACC完成中断的回调函数
void *pacc_ctl_init(uint link);                                     //初始化pacc外设，link=0/1
void pacc_ctl_exit(uint link);                                      //关闭pacc外设，link=0/1
bool pacc_ctl_proc(void *ctl, void *start_cs, uint samples);        //将start_cs注册到ctl，按顺序处理cs链路，并等待处理完成（公共区，配合set_io_addr_fsize/set_all_addr_fsize可修改输入输出buf地址）
bool pacc_ctl_proc_kick(void *ctl, void *start_cs, uint samples);   //将start_cs注册到ctl，按顺序处理cs链路，不等待（公共区）
bool pacc_ctl_proc_wait(void *ctl, uint timeout);                   //等待处理完成
void pacc_dump_cs(void *dump_cs, uint id);
void pacc_dump_cs_list(pacc_cs_t *start_cs);

//cs api
void pacc_cs_set_next(pacc_cs_t *cs, pacc_cs_t *next_cs);                               //将next_cs链接到cs->next_ptr
void pacc_cs_set_buf_addr(pacc_cs_t *cs, void *buf_addr, void *coef_addr);              //设置cs的buf_addr和coef_addr
void pacc_cs_set_io_fmt(pacc_cs_t *cs, bool stereo, u8 in_fmt, u8 out_fmt);             //设置cs的输入输出格式
void pacc_cs_set_io_addr_fsize(pacc_cs_t *cs, void *out_addr, void *in_addr0, void *in_addr1, uint samples);//设置cs的输入输出buffer地址
//cs_list
void pacc_cs_list_set_io_fmt(pacc_cs_t *start_cs, bool stereo, u8 in_fmt, u8 out_fmt);  //设置cs链路输入输出格式
void pacc_cs_list_set_io_fsize(pacc_cs_t *start_cs, uint samples);                      //设置cs链路的处理帧长
void pacc_cs_list_set_io_addr_fsize(pacc_cs_t *start_cs, void *dst, void *src, uint samples); //配置链路头和尾cs的输入输出buf地址，以及整个链路的帧长。输入输出16bit时，需要24bit的中间buffer（公共区）
void pacc_cs_list_set_all_addr_fsize(pacc_cs_t *start_cs, void *dst, void *src, uint samples);//配置整个链路所有cs的buf地址和帧长。输入输出都是24bit时，不需要中间buffer（公共区）

//eq api
//uint pacc_eq_init(pacc_cs_t *cs, eq_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);      //初始化bq_cs
//uint pacc_eq_off(pacc_cs_t *cs);                                                        //关闭bq_cs
void pacc_eq_set_by_param(pacc_cs_t *cs, u8 band_cnt, const u32 *eq_param);
bool pacc_eq_set_by_res(pacc_cs_t *cs, u32 addr, u32 len);
void pacc_eq_set_post_gain(pacc_cs_t *cs, u32 gain);

//dyeq api

//drc api
//uint pacc_drc_init(pacc_cs_t *cs, drc_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);    //初始化drc_cs
//uint pacc_drc_off(pacc_cs_t *cs);                                                       //关闭drc_cs
uint pacc_drc_set_by_res(pacc_cs_t *cs, u32 addr, u32 len);
uint pacc_drc_set_by_param(pacc_cs_t *cs, u8 band_cnt, const u32 *drc_param);

//freq_shift api
uint pacc_fsh_init(pacc_cs_t *cs, freq_shift_bd_t *bd, bool stereo, u8 in_fmt, u8 out_fmt);
uint pacc_fsh_set_param(pacc_cs_t *cs, s32 phase_step);
uint pacc_fsh_off(pacc_cs_t *cs);


//pacc_effect处理类型
enum {
    PACC_EQ,            //mode0, bq
    PACC_EQ_DRC,        //mode1, bq->drc
    PACC_DRC,           //mode0, drc
    PACC_DRC_EQ,        //mode1, drc -> eq
    PACC_MDRC,          //mode2, mdrc
    PACC_DYEQ,          //dybq
    PACC_FSH,           //freq_shift
    PACC_FIR,           //fir filter
};

typedef struct pacc_effect_cb_tag {
    u8 acc_id;
    u8 acc_mode;
    bool stereo;
    bool right_channel;
    void *data_in;
    pacc_fmt_t fmt_in;
    void *data_out;
    pacc_fmt_t fmt_out;
    void *pacc_cs;
    void *pacc_sb;
} pacc_effect_cb;

//eq参数配置
typedef struct {
    struct pacc_cs_tag* cs;
    void *drc_coef;
    void *eq_zx;
    void *eq_coef;
    u8 max_band_cnt;                    //max_band_cnt
    u8 band_cnt;                        //eq_band, eq_band <= max_band_cnt
    u8 mode;                            //0: eq    1: eq->drc
    bool change_en;                     //fade_in, change, fade_out enable
} pacc_eq_t;

//drc参数配置
typedef struct {
    struct pacc_cs_tag *cs;
    void *drc_coef;
    void *eq_zx;
    void *eq_coef;
    u8 eq_band;                         //eq_band
    u8 mode;                            //0: drc    1: drc->eq    2: mdrc, bq0->drc->bq1
} pacc_drc_t;

//dyeq参数配置
typedef struct {
    struct pacc_cs_tag* cs;
    void *drc_coef;
    void *eq_zx;
    void *eq_coef;
} pacc_dyeq_t;

//fir参数配置
typedef struct {
    struct pacc_cs_tag* cs;
    u32 order       : 8;        //fir order
    u32 shift_bits  : 6;
    u8* zx;
    u8* coef;
} pacc_fir_t;

//音乐使用PACC1 麦使用PACC0
void *pacc_effect_init(uint link);  //初始化pacc_effect，返回：ctl控制句柄
pacc_cs_t *pacc_effect_link(const pacc_effect_cb *cb_tbl, u8 max_cs, u16 frame_len);  //链表注册，返回cs链表的表头
pacc_cs_t *pacc_effect_relink(pacc_cs_t *cs_tbl, u8 *pacc_en, u8 max_cs);       //链表动态调整，返回cs链表的链头
void pacc_effect_process(void *ctl, pacc_cs_t *start_cs);                       //按链表顺序处理音频链路，每次处理frame_len个样点
void pacc_effect_exit(uint link);   //关闭pacc_effect

bool pacc_effect_eq_init(pacc_eq_t *eq, pacc_cs_t *cs, u8 mode, u8 eq_band, bool change_en, void *coef_buf, u16 coef_len, void *zx_buf, u16 zx_len);
bool pacc_effect_set_eq_param(pacc_eq_t *eq, u8 *buf, u8 params);               //设置EQ参数，change_en=0时立即生效，change_en=1时需要update
bool pacc_effect_set_eq_update(pacc_eq_t *eq0, pacc_eq_t *eq1);                 //change_en=1时，调用后参数才生效（左右声道同步）

bool pacc_effect_drc_init(pacc_drc_t *drc, pacc_cs_t *cs, u8 mode, u8 eq_band, void *coef_buf, u16 coef_len, void *zx_buf, u16 zx_len);
bool pacc_effect_set_drc_param(pacc_drc_t *drc, u8 *buf, u8 params);
bool pacc_effect_set_mdrc_param(pacc_drc_t *mdrc, u8* buf, u8 params);

bool pacc_effect_dyeq_init(pacc_dyeq_t *dyeq, pacc_cs_t *cs, void *coef_buf, u16 coef_len, void *zx_buf, u16 zx_len);
bool pacc_effect_set_dyeq_param(pacc_dyeq_t *dyeq, u8 *buf, u8 params);
//fir
void pacc_effect_fir_init(pacc_fir_t* pacc_fir, pacc_cs_t *cs, u16 order, u8 shift_bits, u8* coef_buf, u16 coef_len, u8* zx_buf, u16 zx_len);
void pacc_effect_set_fir_param(pacc_fir_t* fir, const s32* fir_coef);

#endif


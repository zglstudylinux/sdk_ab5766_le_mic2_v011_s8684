#ifndef _EFFECT_H
#define _EFFECT_H


#define BUF_OFS_CFG_MIC_EQ           		(0x00000016)
#define BUF_LEN_OFS_CFG_MIC_EQ       		(0x00000012)
#define SHOWBUF_OFS_CFG_MIC_EQ       		(0x00000016)
#define SHOWBUF_LEN_OFS_CFG_MIC_EQ   		(0x0000007E)

#define BUF_OFS_CFG_MIC_DRC          		(0x0000002A)
#define BUF_LEN_OFS_CFG_MIC_DRC      		(0x00000026)
#define SHOWBUF_OFS_CFG_MIC_DRC      		(0x0000002A)
#define SHOWBUF_LEN_OFS_CFG_MIC_DRC  		(0x00000082)

#define BUF_OFS_CFG_ECHO             		(0x0000003E)
#define BUF_LEN_OFS_CFG_ECHO         		(0x0000003A)
#define SHOWBUF_OFS_CFG_ECHO         		(0x0000003E)
#define SHOWBUF_LEN_OFS_CFG_ECHO     		(0x00000086)

#define BUF_OFS_CFG_MAGIC            		(0x00000052)
#define BUF_LEN_OFS_CFG_MAGIC        		(0x0000004E)
#define SHOWBUF_OFS_CFG_MAGIC        		(0x00000052)
#define SHOWBUF_LEN_OFS_CFG_MAGIC    		(0x0000008A)

#define BUF_OFS_CFG_MIX_EQ           		(0x00000066)
#define BUF_LEN_OFS_CFG_MIX_EQ       		(0x00000062)
#define SHOWBUF_OFS_CFG_MIX_EQ       		(0x00000066)
#define SHOWBUF_LEN_OFS_CFG_MIX_EQ   		(0x0000008E)

#define BUF_OFS_CFG_MIX_DRC          		(0x0000007A)
#define BUF_LEN_OFS_CFG_MIX_DRC      		(0x00000076)
#define SHOWBUF_OFS_CFG_MIX_DRC      		(0x0000007A)
#define SHOWBUF_LEN_OFS_CFG_MIX_DRC  		(0x00000092)



#define ALL_MODE_NAME_SUM            		 0x00000A68


//EFFECT_MODE_IDX
enum {
	CFG_MIC_EQ = 0,
	CFG_MIC_DRC,
	CFG_ECHO,
	CFG_MAGIC,
	CFG_MIX_EQ,
	CFG_MIX_DRC,
	CFG_MAX,
};


typedef struct {
	char effect_cfg_name[12];
	u32  effect_res_offset;
	u32  effect_len_offset;
} effect_info_cfg_t;

extern const effect_info_cfg_t effect_info[CFG_MAX];

typedef struct {
	void (*effect_update_callback)(u8 *buf, u32 len, u8 params);//0:初始化 1:更新
} effect_update_callback_t;

extern const effect_update_callback_t effect_update_callback_tbl[CFG_MAX];

/* 在线调试->芯片回复中的Reply枚举类型
enum effect_online_debug_reply
{
    OK,                   //0：成功
    FAIL,                 //1：失败
    NOT_SUPPORTED,        //2：不支持
    SAMPLE_RATE_ERROR,    //3：采样率错误
    PARAM_ERROR,          //4：参数错误
};
*/

#endif
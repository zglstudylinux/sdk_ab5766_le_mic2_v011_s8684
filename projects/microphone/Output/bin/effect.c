#include "include.h"
#include "effect.h"


const effect_info_cfg_t effect_info[CFG_MAX] = {
	[CFG_MIC_EQ]   = {"CFG_MIC_EQ##", BUF_OFS_CFG_MIC_EQ,    BUF_LEN_OFS_CFG_MIC_EQ},
	[CFG_MIC_DRC]  = {"CFG_MIC_DRC#", BUF_OFS_CFG_MIC_DRC,   BUF_LEN_OFS_CFG_MIC_DRC},
	[CFG_ECHO]     = {"CFG_ECHO####", BUF_OFS_CFG_ECHO,      BUF_LEN_OFS_CFG_ECHO},
	[CFG_MAGIC]    = {"CFG_MAGIC###", BUF_OFS_CFG_MAGIC,     BUF_LEN_OFS_CFG_MAGIC},
	[CFG_MIX_EQ]   = {"CFG_MIX_EQ##", BUF_OFS_CFG_MIX_EQ,    BUF_LEN_OFS_CFG_MIX_EQ},
	[CFG_MIX_DRC]  = {"CFG_MIX_DRC#", BUF_OFS_CFG_MIX_DRC,   BUF_LEN_OFS_CFG_MIX_DRC},
};


/*模块初始化更新回调注册表 cv到应用层使用
const effect_update_callback_t effect_update_callback_tbl[CFG_MAX] = {
	[CFG_MIC_EQ]   = {NULL},
	[CFG_MIC_DRC]  = {NULL},
	[CFG_ECHO]     = {NULL},
	[CFG_MAGIC]    = {NULL},
	[CFG_MIX_EQ]   = {NULL},
	[CFG_MIX_DRC]  = {NULL},
};
*/
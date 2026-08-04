#ifndef _BSP_SARADC_VBAT_H
#define _BSP_SARADC_VBAT_H

#define LPWR_DETECT_CNT                 4
#define LPWR_WARNING_VBAT               (xcfg_cb.lpwr_warning_vbat_level*100+2700)
#define LPWR_OFF_VBAT                   (xcfg_cb.lpwr_off_vbat_level*100+2700)

typedef struct {
    u16 vbat_min_power;
    u16 vbat_max_power;
    u8  vbat_max_level;
    u8  res[4];
} vbat_param_struct;

typedef struct {
    u8 lpwr_warning_vbat_cnt;
    u8 lpwr_warning_vbat_exit_cnt;
    u8 lpwr_warning_vbat_flag;
    u8 lpwr_off_vbat_cnt;
} lpwr_param_t;

void bsp_vbat_detect_init(void);
uint16_t bsp_get_vbat_level(u8 mode);
bool bsp_vbat_proc(void);

#endif // _BSP_SARADC_VBAT_H

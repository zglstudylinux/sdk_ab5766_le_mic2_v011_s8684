#ifndef _BSP_SDADC_H
#define _BSP_SDADC_H

void bsp_sdadc_init(void);
void bsp_sdadc_kick(void);
void bsp_sdadc_exit(void);

///extern
void wl_save_tick1_time(void);
void wl_tick_time_init(void);

void sdadc_rmdc_restore(void);
bool bsp_sdadc_discard_proc(void);
#endif

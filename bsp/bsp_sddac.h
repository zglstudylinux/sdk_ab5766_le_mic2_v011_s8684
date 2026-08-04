#ifndef _BSP_SDDAC_H_
#define _BSP_SDDAC_H_
#include "include.h"
#include "res.h"
#include "dec.h"

void bsp_sddac_aubuf_kick(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf_addr, uint32_t samples);
void bsp_sddac_init(void);
void bsp_sddac_deinit(void);
#endif  //_BSP_TONE_H_

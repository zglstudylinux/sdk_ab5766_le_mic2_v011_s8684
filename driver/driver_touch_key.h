/**
******************************************************************************************************************************
*
*@file driver_touch_key.h
*
*@brief Header file for Touch Key Driver
*
*@Create date: 2023-03-23
*
*
*Copyright (c) 2023, BLUETRUM
******************************************************************************************************************************
**/
#ifndef _TOUCH_KEY_H_
#define _TOUCH_KEY_H_

#ifdef __cpluscplus
extern "C" {
#endif

/*
 * INCLUDE FILES
 ***************************************************************************
 */
#include "driver_com.h"

/*
 * VARIABLE DEFINITION
 ***************************************************************************
 */

#define SUPPORT_TOUCH_KEY_NUM           4

typedef struct {
    bool en;
    u16 cdpr;
    u8  ctrim;                          //cur channel ctrim select
    u8  itrim;                          //cur channel itrim select
} tkey_ch_t;

typedef struct {
    bool auto_calib;
    u8 bcnt_dig_ch;
    u8 bcnt_ana_ch;
    const tkey_ch_t *key[SUPPORT_TOUCH_KEY_NUM];
    union {
        struct {
            u32 fil_low     :  4;       //touch key state low state filter length, fil_low + 1
            u32 fil_high    :  4;       //touch key state high state filter length, fil_high + 1
            u32 fil_except  :  8;       //touch key state exception filter length, fil_except + 1
            u32 fil_val     :  4;       //touch key base_cnt valid filter length, fil_val + 1
            u32 to_except   :  12;      //touch key state high timeout length
        };
        u32 reg_tktmr;
    };
    union {
        struct {
            u32 tkpthd      :   12;     //touch key press threshold
            u32 resv0       :   4;
            u32 tkrthd      :   12;     //touch key release threshold
            u32 pto_except  :   4;      //touch key press time out length. (pto_except << 8)
        };
        u32 reg_tkpthd;
    };
    union {
        struct {
            u32 tksthd      :   12;     //touch key smaller threshold
            u32 resv1       :   4;
            u32 tklthd      :   12;     //touch key larger threshold
            u32 resv2       :   4;
        };
        u32 reg_tkethd;
    };

    union {
        struct {
            u32 tkvthd      :   16;     //touch key variance threshold
            u32 val         :   16;     //touch key variance
        };
        u32 reg_tkvari;
    };

    union {
        struct {
            u32 tkarthd     :   12;     //touch key average range threshold
            u32 tkaethd     :   4;      //touch key average equal threshold
            u32 tkvfil      :   8;      //touch key variance filter count
            u32 tkbadd      :   8;      //touch key base counter adder value
        };
        u32 reg_tkvarithd;
    };

    union {
        struct {
            u32 press_vari_en   :   1;  //press variance enable bit
            u32 rels_vari_en    :   1;  //release variance enable bit
            u32 press_fil_sel   :   1;  //press variance after filter
            u32 rels_fil_sel    :   1;  //release variance after filter
            u32 press_vari_thd  :   12; //press variance threshold
            u32 rels_vari_thd   :   12; //release variance threshold
            u32 to_bcnt_thd     :   4;  //time out base counter threshold
        };
        u32 reg_tkcon2;
    };


    //in ear
    union {
        struct {
            u32 ear_fil_low     :  4;
            u32 ear_fil_high    :  4;
            u32 ear_fil_except  :  8;
            u32 ear_fil_val     :  4;
            u32 tkpwup          :  6;
        };
        u32 reg_tetmr;
    };
    union {
        struct {
            u32 tepthd      :   12;     //touch ear press threshold
            u32 resv3       :   4;
            u32 terthd      :   12;     //touch ear release threshold
            u32 tefathd     :   4;
        };
        u32 reg_tepthd;
    };
    union {
        struct {
            u32 testhd      :   12;     //touch ear smaller threshold
            u32 resv5       :   4;
            u32 telthd      :   12;     //touch ear larger threshold
            u32 resv6       :   4;
        };
        u32 reg_teethd;
    };
} tkey_cfg_t;

typedef struct {
    u8 ch;
    u8 touch_key_sta[SUPPORT_TOUCH_KEY_NUM];
    u16 tkcnt_tmr[4];       //每个通路的实时TKCNT
} tk_cb_t;


/*
 * FUNCTION DECLARATION
 ***************************************************************************
 */
int touch_key_init(touch_key_typedef *touch_key_reg, void *tkey_cfg, CLK_TOUCH_KEY_TYPEDEF clk_sel, uint8_t tkclk_div2_enable);
void touch_key_tkcnt_isr(u32 tk_ch, u16 tkcnt);
u8 touch_key_get_key(touch_key_typedef *touch_key_regid);
void touch_key_sw_reset(void);

#ifdef __cpluscplus
}
#endif

#endif

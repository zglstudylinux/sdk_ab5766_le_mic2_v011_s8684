/*
 * @File name    : driver_iis.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-28
 * @Description  : This file contains all the functions prototypes for the IIS library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_IIS_H
#define _DRIVER_IIS_H
#include "include.h"
#include "driver_com.h"

typedef enum {
    IIS_FLAG_RX_PND          = 1 << 0,
    IIS_FLAG_TX_PND          = 1 << 1,
} IIS_FLAG_TYPEDEF;

typedef enum {
    IIS_ROLE_MASTER          = 0 << 3,
    IIS_ROLE_SLAVE           = 1 << 3,
} IIS_ROLE_TYPEDEF;

typedef enum {
    IIS_IOMAP_0              = 0,  //MCLK:PA8, LRCLK:PA0, BCLK:PA1, DO:PA2, PI:PA3.
    IIS_IOMAP_1              = 1,  //MCLK:PA8, LRCLK:PA4, BCLK:PA5, DO:PA6, PI:PA7.
} IIS_IOMAP_TYPEDEF;

typedef enum {
    IIS_MODE_LEFT_JUSTIFIED  = 0 << 4,
    IIS_MODE_NORMAL          = 1 << 4,  //标准IIS模式
} IIS_DAT_MODE_TYPEDEF;

typedef enum {
    IIS_DAT_SRC_DAC          = 0 << 5,  //仅TX可配，需要先初始化DAC
    IIS_DAT_SRC_DMA          = 1 << 5,  //TX、RX都可配
} IIS_DAT_SRC_TYPEDEF;

typedef enum {
    IIS_DAC_OUT_SRC0         = 0,  
    IIS_DAC_OUT_SRC1         = 1,  
    IIS_DAC_OUT_US0          = 2,  
    IIS_DAC_OUT_US1          = 3,  
    IIS_DAC_OUT_US0_SRCVOL   = 4,  
    IIS_DAC_OUT_US1_SRCVOL   = 5,  
    IIS_DAC_OUT_MIX_DACVOL   = 6,  
    IIS_DAC_OUT_DPD          = 7,  
} IIS_DAC_OUT_TYPEDEF;

typedef enum {
    IIS_PULSE_NORMALL        = 0 << 6,  //50% duty cycle
    IIS_PULSE_ONE_BIT_CLK    = 1 << 6,
    IIS_PULSE_ONE_SUBFRAME   = 2 << 6,
} IIS_WS_PULSE_TYPEDEF;

typedef enum {
    IIS_ONE_WIRE_OUTPUT      = 0 << 9,
    IIS_ONE_WIRE_INPUT       = 1 << 9,
} IIS_ONE_WIRE_TYPEDEF;

typedef enum {
    IIS_CH_CNT_2             = 0,  //双声道
    IIS_CH_CNT_4             = 1,
    IIS_CH_CNT_6             = 2,
    IIS_CH_CNT_8             = 3,
    IIS_CH_CNT_10            = 4,
    IIS_CH_CNT_12            = 5,
    IIS_CH_CNT_14            = 6,
    IIS_CH_CNT_16            = 7,
} IIS_CH_CNT_TYPEDEF;

typedef enum {
    IIS_CH_DEPTH_32          = 0,  //每个声道最大32个样点
    IIS_CH_DEPTH_64          = 1,
    IIS_CH_DEPTH_128         = 2,
    IIS_CH_DEPTH_256         = 3,
    IIS_CH_DEPTH_512         = 4,
    IIS_CH_DEPTH_1024        = 5,
    IIS_CH_DEPTH_2048        = 6,
} IIS_CH_DEPTH_TYPEDEF;

typedef struct {
    uint16_t tx_samples;
    // uint16_t rx_samples;  //等于ch_depth_sel的值
    uint8_t *dma_txbuf0_ptr;
    uint8_t *dma_txbuf1_ptr;
    uint8_t *dma_rxbuf0_ptr;
    uint8_t *dma_rxbuf1_ptr;
} iis_dma_cfg_typedef;

typedef struct {
    IIS_ROLE_TYPEDEF role;
    IIS_IOMAP_TYPEDEF iomap;
    IIS_DAT_MODE_TYPEDEF dat_mode;
    IIS_DAT_SRC_TYPEDEF tx_dat_src;
    IIS_DAC_OUT_TYPEDEF dac_out_sel;
    FUNCTIONAL_STATE tx_en;
    FUNCTIONAL_STATE rx_en;
    IIS_WS_PULSE_TYPEDEF ws_pulse_sel;
    uint8_t word_width;        //样点的位宽
    uint8_t tx_bit_depth;      //tx样点的有效位
    uint8_t rx_bit_depth;      //rx样点的有效位
    IIS_CH_CNT_TYPEDEF ch_cnt_sel;      //声道数
    IIS_CH_DEPTH_TYPEDEF ch_depth_sel;  //每个通道收发的最大样点数
    FUNCTIONAL_STATE one_wire_en;
    IIS_ONE_WIRE_TYPEDEF one_wire_dir;
    FUNCTIONAL_STATE mclk_out_en;
    FUNCTIONAL_STATE bclk_inv;
    FUNCTIONAL_STATE ws_inv;
    FUNCTIONAL_STATE slv_bclk_flt_en;
    FUNCTIONAL_STATE slv_ws_flt_en;
    FUNCTIONAL_STATE slv_di_flt_en;
    FUNCTIONAL_STATE tx_bit_rev;
    FUNCTIONAL_STATE rx_bit_rev;
    FUNCTIONAL_STATE slv_dat_sync_ws;
    FUNCTIONAL_STATE slv_lose_ch_hdl_ws;

    iis_dma_cfg_typedef dma_cfg;
} iis_init_typedef;

/**
  * @brief  Initializes the IIS peripheral io map according to the specified
  *         parameters in the iis_cb.
  * @param  iis_reg: IIS_REG.
  * @param  iis_cb: pointer to a iis_init_typedef structure that
  *         contains the configuration information for the specified IIS peripheral.
  * @retval None
  */
void iis_io_map_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb);

/**
  * @brief  Initializes the IIS peripheral dma according to the specified
  *         parameters in the iis_cb.
  * @param  iis_reg: IIS_REG.
  * @param  iis_cb: pointer to a iis_init_typedef structure that
  *         contains the configuration information for the specified IIS peripheral.
  * @retval None
  */
void iis_dma_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb);

/**
  * @brief  Initializes the IIS peripheral data src according to the specified
  *         parameters in the iis_cb.
  * @param  iis_reg: IIS_REG.
  * @param  iis_cb: pointer to a iis_init_typedef structure that
  *         contains the configuration information for the specified IIS peripheral.
  * @retval None
  */
void iis_data_src_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb);

/**
  * @brief  Initializes the IIS peripheral according to the specified
  *         parameters in the iis_cb.
  * @param  iis_reg: IIS_REG.
  * @param  iis_cb: pointer to a iis_init_typedef structure that
  *         contains the configuration information for the specified IIS peripheral.
  * @retval None
  */
void iis_init(iis_typedef *iis_reg, iis_init_typedef *iis_cb);

/**
  * @brief  Enable or disable the specified IIS peripheral.
  * @param  iis_reg: IIS_REG.
  * @param  state: state of the IIS peripheral.
            This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void iis_cmd(iis_typedef *iis_reg, FUNCTIONAL_STATE en);

/**
  * @brief  Kick IIS dma tx.
  * @param  iis_reg: IIS_REG.
  * @param  iis_cb: pointer to a iis_init_typedef structure that
  *         contains the configuration information for the specified IIS peripheral.
  * @param  buf: tx data.
  * @param  buf_len: tx data len.
  * @retval None
  */
void iis_dma_tx_kick(iis_typedef *iis_reg, iis_init_typedef *iis_cb, uint8_t *buf, uint16_t buf_len);

 /**
  * @brief  Enable or disable the IIS interrupt.
  * @param  iis_reg: IIS_REG.
  * @param  isr: function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  flag_type: interrup flag.
  * @param  state: the state of the IIS peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void iis_pic_config(iis_typedef *iis_reg, isr_t isr, int pr, IIS_FLAG_TYPEDEF flag_type, FUNCTIONAL_STATE state);

/**
  * @brief  Get the IIS interrupt pending.
  * @param  iis_reg: IIS_REG.
  * @param  flag_type: interrup flag.
  * @retval The state of interrupt_type (SET or RESET).
  */
FLAG_STATE iis_get_flag(iis_typedef *iis_reg, IIS_FLAG_TYPEDEF flag_type);

/**
  * @brief  Clear the IIS interrupt pending.
  * @param  iis_reg: IIS_REG.
  * @param  flag_type: interrup flag.
  * @retval None
  */
void iis_clear_flag(iis_typedef *iis_reg, IIS_FLAG_TYPEDEF flag_type);

/**
 * @brief  De-initialize the specified IIS peripheral.
 * @param  iis_reg: IIS_REG.
 * @param  iis_cb: pointer to a iis_init_typedef structure that
 *         contains the configuration information for the specified IIS peripheral.
 * @retval None
 */
void iis_deinit(iis_typedef *iis_reg, iis_init_typedef *iis_cb);

#endif  //_DRIVER_IIS_H

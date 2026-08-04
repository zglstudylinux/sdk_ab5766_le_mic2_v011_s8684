/*
 * @File name    : driver_ledc.h
 * @Author       : Bluetrum IOT Team.
 * @Date         : 2025-09-10
 * @Description  : This file contains all the functions protorypes for the SDDAC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_SDDAC_H
#define _DRIVER_SDDAC_H

#include "driver_com.h"

//dac analog gain definition
#define N_54DB       0x00        //0 0000
#define N_51DB       0x01        //0 0001
#define N_48DB       0x02        //0 0010
#define N_45DB       0x03        //0 0011
#define N_42DB       0x04        //0 0100
#define N_39DB       0x05        //0 0101
#define N_36DB       0x06        //0 0110
#define N_33DB       0x07        //0 0111
#define N_30DB       0x08        //0 1000
#define N_27DB       0x09        //0 1001
#define N_24DB       0x0A        //0 1010
#define N_21DB       0x0B        //0 1011
#define N_18DB       0x0C        //0 1100
#define N_15DB       0x0D        //0 1101
#define N_12DB       0x0E        //0 1110
#define N_11DB       0x0F        //0 1111
#define N_10DB       0x10        //1 0000
#define N_9DB        0x11        //1 0001
#define N_8DB        0x12        //1 0010
#define N_7DB        0x13        //1 0011
#define N_6DB        0x14        //1 0100
#define N_5DB        0x15        //1 0101
#define N_4DB        0x16        //1 0110
#define N_3DB        0x17        //1 0111
#define N_2DB        0x18        //1 1000
#define N_1DB        0x19        //1 1001
#define N_0DB        0x1A        //1 1010
#define P_1DB        0x1B        //1 1011
#define P_2DB        0x1C        //1 1100
#define P_3DB        0x1D        //1 1101
#define P_4DB        0x1E        //1 1110
#define P_5DB        0x1F        //1 1111

//数字音量
#define MAX_DIG_VAL         32767
// #define DIG_P15DB           184262    //(int)(MAX_DIG_VAL * 5.623413)           //15db
// #define DIG_P14DB           164224    //(int)(MAX_DIG_VAL * 5.011872)           //14db
// #define DIG_P13DB           146364    //(int)(MAX_DIG_VAL * 4.466836)           //13db
// #define DIG_P12DB           130447    //(int)(MAX_DIG_VAL * 3.981072)           //12db
// #define DIG_P11DB           116261    //(int)(MAX_DIG_VAL * 3.548134)           //11db
// #define DIG_P10DB           103618    //(int)(MAX_DIG_VAL * 3.162278)           //10db
// #define DIG_P9DB            92349     //(int)(MAX_DIG_VAL * 2.818383)           //9db
// #define DIG_P8DB            82306     //(int)(MAX_DIG_VAL * 2.511886)           //8db
// #define DIG_P7DB            73356     //(int)(MAX_DIG_VAL * 2.238721)           //7db
#define DIG_P6DB            65378     //(int)(MAX_DIG_VAL * 1.995262)           //6db
#define DIG_P5DB            58268     //(int)(MAX_DIG_VAL * 1.778279)           //5db
#define DIG_P4DB            51932     //(int)(MAX_DIG_VAL * 1.584893)           //4db
#define DIG_P3DB            46284     //(int)(MAX_DIG_VAL * 1.412538)           //3db
#define DIG_P2DB            41251     //(int)(MAX_DIG_VAL * 1.258925)           //2db
#define DIG_P1DB            36765     //(int)(MAX_DIG_VAL * 1.122018)           //1db
#define DIG_N0DB            32767     //(int)(MAX_DIG_VAL / 1.000000)           //0db
#define DIG_N1DB            29203     //(int)(MAX_DIG_VAL / 1.122018)           //-1db
#define DIG_N2DB            26027     //(int)(MAX_DIG_VAL / 1.258925)
#define DIG_N3DB            23197     //(int)(MAX_DIG_VAL / 1.412538)
#define DIG_N4DB            20674     //(int)(MAX_DIG_VAL / 1.584893)
#define DIG_N5DB            18426     //(int)(MAX_DIG_VAL / 1.778279)
#define DIG_N6DB            16422     //(int)(MAX_DIG_VAL / 1.995262)
#define DIG_N7DB            14636     //(int)(MAX_DIG_VAL / 2.238721)
#define DIG_N8DB            13044     //(int)(MAX_DIG_VAL / 2.511886)
#define DIG_N9DB            11626     //(int)(MAX_DIG_VAL / 2.818383)
#define DIG_N10DB           10361     //(int)(MAX_DIG_VAL / 3.162278)
#define DIG_N11DB           9234      //(int)(MAX_DIG_VAL / 3.548134)
#define DIG_N12DB           8230      //(int)(MAX_DIG_VAL / 3.981072)
#define DIG_N13DB           7335      //(int)(MAX_DIG_VAL / 4.466836)
#define DIG_N14DB           6537      //(int)(MAX_DIG_VAL / 5.011872)
#define DIG_N15DB           5826      //(int)(MAX_DIG_VAL / 5.623413)
#define DIG_N16DB           5193      //(int)(MAX_DIG_VAL / 6.309573)
#define DIG_N17DB           4628      //(int)(MAX_DIG_VAL / 7.079458)
#define DIG_N18DB           4125      //(int)(MAX_DIG_VAL / 7.943282)
#define DIG_N19DB           3676      //(int)(MAX_DIG_VAL / 8.912509)
#define DIG_N20DB           3276      //(int)(MAX_DIG_VAL / 10.000000)
#define DIG_N21DB           2920      //(int)(MAX_DIG_VAL / 11.220185)
#define DIG_N22DB           2602      //(int)(MAX_DIG_VAL / 12.589254)
#define DIG_N23DB           2319      //(int)(MAX_DIG_VAL / 14.125375)
#define DIG_N24DB           2067      //(int)(MAX_DIG_VAL / 15.848932)
#define DIG_N25DB           1842      //(int)(MAX_DIG_VAL / 17.782794)
#define DIG_N26DB           1642      //(int)(MAX_DIG_VAL / 19.952623)
#define DIG_N27DB           1463      //(int)(MAX_DIG_VAL / 22.387211)
#define DIG_N28DB           1304      //(int)(MAX_DIG_VAL / 25.118864)
#define DIG_N29DB           1162      //(int)(MAX_DIG_VAL / 28.183829)
#define DIG_N30DB           1036      //(int)(MAX_DIG_VAL / 31.622777)
#define DIG_N31DB           923       //(int)(MAX_DIG_VAL / 35.481339)
#define DIG_N32DB           823       //(int)(MAX_DIG_VAL / 39.810717)
#define DIG_N33DB           733       //(int)(MAX_DIG_VAL / 44.668359)
#define DIG_N34DB           653       //(int)(MAX_DIG_VAL / 50.118723)
#define DIG_N35DB           582       //(int)(MAX_DIG_VAL / 56.234133)
#define DIG_N36DB           519       //(int)(MAX_DIG_VAL / 63.095734)
#define DIG_N37DB           462       //(int)(MAX_DIG_VAL / 70.794578)
#define DIG_N38DB           412       //(int)(MAX_DIG_VAL / 79.432823)
#define DIG_N39DB           367       //(int)(MAX_DIG_VAL / 89.125094)
#define DIG_N40DB           327       //(int)(MAX_DIG_VAL / 100.000000)
#define DIG_N41DB           292       //(int)(MAX_DIG_VAL / 112.201845)
#define DIG_N42DB           260       //(int)(MAX_DIG_VAL / 125.892541)
#define DIG_N43DB           231       //(int)(MAX_DIG_VAL / 141.253754)
#define DIG_N44DB           206       //(int)(MAX_DIG_VAL / 158.489319)
#define DIG_N45DB           184       //(int)(MAX_DIG_VAL / 177.827941)
#define DIG_N46DB           164       //(int)(MAX_DIG_VAL / 199.526231)
#define DIG_N47DB           146       //(int)(MAX_DIG_VAL / 223.872114)
#define DIG_N48DB           130       //(int)(MAX_DIG_VAL / 251.188643)
#define DIG_N49DB           116       //(int)(MAX_DIG_VAL / 281.838293)
#define DIG_N50DB           103       //(int)(MAX_DIG_VAL / 316.227766)
#define DIG_N51DB           92        //(int)(MAX_DIG_VAL / 354.813389)
#define DIG_N52DB           82        //(int)(MAX_DIG_VAL / 398.107171)
#define DIG_N53DB           73        //(int)(MAX_DIG_VAL / 446.683592)
#define DIG_N54DB           65        //(int)(MAX_DIG_VAL / 501.187234)
#define DIG_N55DB           58        //(int)(MAX_DIG_VAL / 562.341325)
#define DIG_N56DB           51        //(int)(MAX_DIG_VAL / 630.957344)
#define DIG_N57DB           46        //(int)(MAX_DIG_VAL / 707.945784)
#define DIG_N58DB           41        //(int)(MAX_DIG_VAL / 794.328235)
#define DIG_N59DB           36        //(int)(MAX_DIG_VAL / 891.250938)
#define DIG_N60DB           0

typedef enum {
    SDDAC_SPR_48k      =  48000,
    SDDAC_SPR_44k1     =  44100,
    SDDAC_SPR_38k      =  38000,
    SDDAC_SPR_32k      =  32000,
    SDDAC_SPR_24k      =  24000,
    SDDAC_SPR_22k05    =  22050,
    SDDAC_SPR_16k      =  16000,
    SDDAC_SPR_12k      =  12000,
    SDDAC_SPR_11k025   =  11025,
    SDDAC_SPR_8k       =  8000,
    SDDAC_SPR_6k       =  6000,
    SDDAC_SPR_4k       =  4000,
    SDDAC_SPR_96k      =  96000,
    SDDAC_SPR_88k2     =  88200,
    SDDAC_SPR_76k      =  76000,
    SDDAC_SPR_64k      =  64000,
} SDDAC_SPR_TYPEDEF;

typedef enum {
    SDDAC_START_SIZE      = 0,
    SDDAC_START_SIZE_DIV2,
    SDDAC_START_SIZE_DIV4,
    SDDAC_START_SIZE_DIV8,
} SDDAC_START_SEL_TYPEDEF;

typedef enum {
    SDDAC_START_SIZE_64   = 0,
    SDDAC_START_SIZE_128,
    SDDAC_START_SIZE_256,
    SDDAC_START_SIZE_512,
} SDDAC_START_SIZE_TYPEDEF;

typedef enum {
    SDDAC_AUBUF0 = 0,
    SDDAC_AUBUF1,
} SDDAC_AUBUF_TYPEDEF;

typedef enum {
    SDDAC_PCM_16BITS = 0,
    SDDAC_PCM_24BITS,
} SDDAC_PCM_BITS_TYPEDEF;

typedef enum {
    SDDAC_FADE_IN = 0,
    SDDAC_FADE_OUT,
} SDDAC_FADE_DIR_TYPEDEF;

typedef enum {
    SDDAC_CH_SINGLE = 0, //单端MIC
    SDDAC_CH_DIFF,       //差分MIC
} SDDAC_CH_TYPEDEF;

typedef struct {
    bool au0_en;
    bool au0_full_flag;
    SDDAC_PCM_BITS_TYPEDEF au0_pcm_bits;  
    uint16_t au0_size;
    uint16_t au0_thr;
    uint32_t *au0_ptr;
    SDDAC_START_SEL_TYPEDEF  au0_start_sel;
    SDDAC_START_SIZE_TYPEDEF au0_start_size;

    bool au1_en;
    bool au1_full_flag;
    SDDAC_PCM_BITS_TYPEDEF au1_pcm_bits;  
    uint16_t au1_size;
    uint16_t au1_thr;
    uint32_t *au1_ptr;
    SDDAC_START_SEL_TYPEDEF  au1_start_sel;
    SDDAC_START_SIZE_TYPEDEF au1_start_size;
} sddac_aubuf_typedef;

typedef struct {
    bool mix_en;
    bool mix_div2_en;

    bool src0_mute;
    bool src0_bypass_en;
    uint16_t src0_vol;
    uint16_t au0mixcoef;
    SDDAC_SPR_TYPEDEF src0_spr;

    bool src1_mute;
    bool src1_bypass_en;
    uint16_t src1_vol;
    uint16_t au1mixcoef;
    SDDAC_SPR_TYPEDEF src1_spr;

    uint16_t total_vol;
    uint16_t dac_en;
} sddac_dig_typedef;

typedef struct {
    SDDAC_CH_TYPEDEF ch;
} sddac_ana_typedef;

typedef struct {
    bool init_flag;
    sddac_aubuf_typedef aubuf;
    sddac_dig_typedef dig;
    sddac_ana_typedef ana;
} sddac_cb_typedef;

extern sddac_cb_typedef sddac_cb;

/**
  * @brief  Initializes the SDDAC peripheral clock.
  * @param  None
  * @retval None
  */
void sddac_clock_init(void);

/**
  * @brief  Initializes the SDDAC peripheral AUBUF according to the specified
  *         parameters in the sddac_cb.
  * @param  sddac_cb: pointer to a sddac_cb_typedef structure that
  *         contains the configuration information for the specified SDDAC peripheral.
  * @retval None
  */
void sddac_aubuf_init(sddac_cb_typedef *sddac_cb);

/**
  * @brief  Initializes the SDDAC peripheral digital module according to the specified
  *         parameters in the sddac_cb.
  * @param  sddac_cb: pointer to a sddac_cb_typedef structure that
  *         contains the configuration information for the specified SDDAC peripheral.
  * @retval None
  */
void sddac_dig_power_on(sddac_cb_typedef *sddac_cb);

/**
 * @brief  De-initialize the specified SDDAC peripheral.
 * @retval None
 */
void sddac_deinit(void);

/**
  * @brief  Set the AUBUF size.
  * @param  idx: AUBUF index 0 or 1.
  * @param  size: AUBUF size.
  *               When configuring 16-bit mono mode, aubuf actually requires (size*2) byte of memory.
  *               When configuring 24-bit mono mode, aubuf actually requires (size*4) byte of memory.
  *               so, aubuf defaults to the uint32_t type.
  * @param  thr: AUBUF threshould value.
  *              When AUBUF is less than the threshold, activate AUBUF PND.
  * @retval None.
  */
void sddac_aubuf_size_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t size, uint16_t thr);

/**
  * @brief  Fill data into AUBUF.
  * @param  idx: AUBUF index 0 or 1.
  * @param  buf: data buf addr.
  * @param  samples: sample point number.
  * @retval None.
  */
void sddac_aubuf_put_samples(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf, uint32_t samples);

/**
  * @brief  Fill data into AUBUF via DMA method.
  * @param  idx: AUBUF index 0 or 1.
  * @param  buf: data buf addr.
  * @param  samples: sample point number.
  * @retval None.
  */
void sddac_aubuf_dma_kick(SDDAC_AUBUF_TYPEDEF idx, uint32_t *buf_addr, uint32_t samples);

/**
  * @brief  Wait for dma done.
  * @param  idx: AUBUF index 0 or 1.
  * @retval None.
  */
void sddac_aubuf_dma_kick_w4_done(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Register the AUBUF interrupt service function.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @retval None.
  */
void sddac_aubuf_pic_config(isr_t isr, int pr);

/**
  * @brief  Register the AUBUF DMA interrupt service function.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @retval None.
  */
void sddac_aubuf_dma_pic_config(isr_t isr, int pr);

/**
  * @brief  Check if the AUBUF is full.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_aubuf_is_full(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Get the AUBUF interrupt pending.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_aubuf_get_flag(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Clear the AUBUF interrupt pending.
  * @param  idx: AUBUF index 0 or 1.
  */
void sddac_aubuf_clear_flag(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Get the AUBUF DMA interrupt pending.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_aubuf_dma_get_flag(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Clear the AUBUF DMA interrupt pending.
  * @param  idx: AUBUF index 0 or 1.
  * @retval None.
  */
void sddac_aubuf_dma_clear_flag(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Output silent data.
  * @param  idx: AUBUF index 0 or 1.
  * @param  samples: sample point number.
  * @retval None.
  */
void sddac_aubuf_put_zero(SDDAC_AUBUF_TYPEDEF idx, uint16_t samples);

/**
  * @brief  Get the SDDAC sample rate.
  * @param  idx: AUBUF index 0 or 1.
  * @retval sample rate.
  */
uint32_t sddac_samp_rate_get(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Set the SDDAC sample rate.
  * @param  idx: AUBUF index 0 or 1.
  * @retval None.
  */
void sddac_samp_rate_set(SDDAC_AUBUF_TYPEDEF idx, uint32_t spr);

/**
  * @brief  Get the SDDAC src digital volume.
  * @param  idx: AUBUF index 0 or 1.
  * @retval digital volume.
  */
uint16_t sddac_dig_vol_get(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Set the SDDAC src digital volume.
  * @param  idx: AUBUF index 0 or 1.
  * @param  vol: digital volume.
  * @param  fade_step: fade step.
  * @retval None.
  */
void sddac_dig_vol_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t vol, uint8_t fade_step);

/**
  * @brief  Set the SDDAC fade in or fade out.
  * @param  fade_dir: fade direction in or out.
  * @param  fade_step: fade step.
  * @retval None.
  */
void sddac_fade_inout_set(SDDAC_FADE_DIR_TYPEDEF fade_dir, uint8_t fade_step);

/**
  * @brief  Get the SDDAC fade done flag.
  * @retval 0 or 1.
  */
bool sddac_fade_inout_done_flag(void);

/**
  * @brief  Wait the SDDAC fade done.
  * @retval None.
  */
void sddac_fade_inout_w4_done(void);

/**
  * @brief  Check if the SDDAC is mute.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_is_mute(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Set the SDDAC mute.
  * @param  idx: AUBUF index 0 or 1.
  * @param  en: enable or disable.
  * @retval None.
  */
void sddac_mute_en(SDDAC_AUBUF_TYPEDEF idx, bool en);

/**
  * @brief  Set the SDDAC mix enable.
  * @param  en: enable or disable.
  * @param  div2_en: enable or disable.
  * @retval None.
  */
void sddac_aubuf_mix_en(bool en, bool div2_en);

/**
  * @brief  Set the SDDAC mix coef.
  * @param  idx: AUBUF index 0 or 1.
  * @param  au_coef: mix coef.
  * @retval None.
  */
void sddac_aubuf_mix_coef_set(SDDAC_AUBUF_TYPEDEF idx, uint16_t au_coef);

/**
  * @brief  Check if the SDDAC AUBUF enable.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_aubuf_en_check(SDDAC_AUBUF_TYPEDEF idx);

/**
  * @brief  Check if the SDDAC pcm is 24-bits.
  * @param  idx: AUBUF index 0 or 1.
  * @retval 0 or 1.
  */
bool sddac_pcm_is_24bits(SDDAC_AUBUF_TYPEDEF idx);

#endif

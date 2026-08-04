#ifndef _API_SDADC_H
#define _API_SDADC_H

typedef enum{
  SDADC_ALL_DONE,
  SDADC_HALF_DONE,
}SDADC_DONE_TYPEDEF;

typedef enum {
  SDADC_MODE_MIC = 0x00,
  SDADC_MODE_DC  = 0x01,
} SDADC_MODE_TYPEDEF;

typedef enum {
  MIC_ANA_OUTPUT_CLOSE = 0,   //关闭模拟输出
  MIC_ANA_GAIN_0DB,
  MIC_ANA_GAIN_6DB,
  MIC_ANA_GAIN_12DB,
} MIC_ANA_GAIN_TYPEDEF;

typedef struct {
  SDADC_MODE_TYPEDEF adc_mode;
  uint8_t mic_mode;
  bool lv_mode_en;                //开LVMODE vddmic 1.9v，关LVMODE vddmic 2.6v
  uint8_t mic_bias_res_level;     //0~63.  Res=16300/mic_bias_res_level
  MIC_ANA_GAIN_TYPEDEF ana_gain;  //省RC：内部固定8.5dB，其它：MIC_ANA_GAIN_TYPEDEF
} sdadc_ana_cfg_typedef;

typedef void (*sdadc_done_callback)(SDADC_DONE_TYPEDEF type);

/**
  * @brief  Set the sdadc sampling completion callback function
            This callback function will be called after sdadc_done_proc_kick is executed
  * @param  func:callback function
  */
void sdadc_done_callback_set(sdadc_done_callback func);

/**
  * @brief  Triggers the aupcm thread to handle the sdadc callback function, which will be set by sdadc_done_callback_set.
            This function is usually called inside the sdadc interrupt function
  * @param  type:SDADC_ALL_DONE or SDADC_HALF_DONE
  */
void sdadc_done_proc_kick(SDADC_DONE_TYPEDEF type);

/**
  * @brief  Initializes the MIC analog according to the specified
  *         parameters in the ana_cfg.
  * @param  ana_cfg: pointer to a sdadc_ana_cfg_typedef structure that
  *         contains the configuration information for the specified MIC analog.
  * @retval None
  */
void sdadc_ana_init(sdadc_ana_cfg_typedef *ana_cfg);

/**
  * @brief  Set the analog gain of the MIC.
  * @param  gain: MIC_ANA_GAIN_TYPEDEF.
  * @retval None.
  */
void sdadc_ana_mic_gain_set(MIC_ANA_GAIN_TYPEDEF gain);

/**
  * @brief  Enable the low power mode of the MIC.
  * @param  en: Enable or Disable. The default mode is enable.
  * @retval None.
  */
void sdadc_ana_mic_lvmode_en(bool en);

/**
 * @brief  De-initialize the specified MIC analog.
 * @retval None
 */
void sdadc_ana_deinit(void);

/**
  * @brief  Initializes the MIC BIAS RES TRIM.
  * @param  lv_mode: 1:VDDMIC 1.9v, 0:VDDMIC 2.6V.
  * @param  dc_volt_targe: MIC IN targe voltage.
  * @retval None
  */
void mic_bias_trim_init(bool lv_mode, uint16_t dc_volt_targe);

#endif //_API_SDADC_H

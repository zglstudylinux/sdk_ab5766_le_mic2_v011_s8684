#ifndef _API_PWR_H_
#define _API_PWR_H_

typedef enum{
    PMU_CFG_BUCK_EN         = BIT(0),
    PMU_CFG_CHARGE_DIS      = BIT(1),
    PMU_CFG_VBAT_3V         = BIT(2),       //VBAT 干电池供电
    PMU_CFG_VBAT_4V2        = BIT(3),       //VBAT 锂电池供电
    PMU_CFG_VDDBT_CAPLESS   = BIT(4),       //VDDBT省电容，LDO模式下才可配置
    PMU_CFG_VUSB_TO_VDDIO   = BIT(7),
} PMU_CFG_ENUM;

/**
  * @brief  pmu initialization
  * @param  pmu_cfg: PMU_CFG_ENUM
  */
void pmu_init(uint16_t pmu_cfg);

/**
  * @brief  get vddio level, vddio = 1.5V + 0.15V * level
  * @return  level: 0 ~ 0xF
  */
u8 pmu_get_vddio(void);

/**
  * @brief  get vbg voltage values, uint:mV
  * @return  vbg voltage value, about 600mv
  */
u16 pmu_get_vbg_mv(void);

/**
  * @brief  set vddio level, vddio = 1.5V + 0.15V * level
  * @param  level: 0 ~ 0xF
  */
void pmu_set_vddio(u8 level);

/**
  * @brief  get vbat saradc voltage distribution coefficient
  * @return  vbat saradc voltage distribution coefficient, about 2 * 1000
  */
u16 pmu_get_vbatdet(void);

/**
  * @brief  get lvd level
  * @return  level: 0 ~ 0x7
  */
u8 pmu_get_lvd(void);

/**
  * @brief  set lvd level
    0: 1.8V
    1: 1.9V
    2-7: 2.0V + 0.2V * (level - 2)
  * @param  level: 0 ~ 0x7
  */
void pmu_set_lvd(u8 level);

/**
  * @brief  set vddbt level
  * @param  level: 0 ~ 0xf, vddbt=0.85 + 0.05*level
  */
void pmu_set_vddbt(uint8_t level);

/**
  * @brief  get vddbt level
  * @return  level: 0 ~ 0xf
  */
uint8_t pmu_get_vddbt(void);


/**
  * @brief  enable buck mode or disable buck mode
  * @param 1:enable buck mode 0:disable buck mode, pmu work in ldo mode
  */
void pmu_set_buck_mode(bool buck_en, bool vddbt_capless);


u8 charge_const_curr_trim(u8 val);
u8 charge_cutoff_curr_trim(u8 val);

void lowpwr_sleep_sfr_save(void);
void lowpwr_sleep_sfr_resume(void);

void lp_sleep(void (*sleep_cb)(void), void (*wakeup_cb)(void));

#endif // _API_PWR_H_


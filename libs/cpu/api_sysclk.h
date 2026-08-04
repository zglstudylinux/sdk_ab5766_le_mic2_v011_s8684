#ifndef _API_SYSCLK_H_
#define _API_SYSCLK_H_

#define SNF_RC_EN_BIT       ((cfg_snf_rc == CLK_SNF_RC300K) ? BIT(27) : BIT(19))
#define SNF_RC_SEL_BIT      ((cfg_snf_rc == CLK_SNF_RC300K) ? 1 : 2)
#define SNF_RC_IS_300K()    (cfg_snf_rc == CLK_SNF_RC300K)

enum {
    /* 448125000/28 */
    SYS_16M,
    SYS_24M,
    SYS_48M,
    SYS_60M,
    SYS_80M,
    SYS_120M,
    SYS_160M,
};

enum {
    CLK_SNF_RC2M,
    CLK_SNF_RC300K,
};

enum {
    INDEX_VOICE     = 0,
    INDEX_DECODE,
    INDEX_STACK,
    INDEX_USER,
    INDEX_MAX_NB,
};

extern uint8_t cfg_snf_rc;

/**
  * @brief  Get the clock value last set for the function sys_clk_set()
  * @return [SYS_16M - SYS_160M]
  */
u8 sys_clk_get(void);

/**
  * @brief  Get the current clock of the system
  * @return [SYS_16M - SYS_160M]
  */
u8 sys_clk_get_cur(void);

u32 sys_clk_nhz_get(void);

/**
  * @brief  Get RC2M clk frequency
  * @param  type: 0: system rc2m  1:rtc rc2m
  * @return RC2M clk frequency
  */
u32 rc2m_clk_nhz_get(u8 type);

u32 pll_clk_nhz_get(void);

/**
  * @brief  Set system clock,
  * @attention  If the clock to be set is lower than the value which be set by the function sys_clk_free, the setting will not take effect now
  * @param [SYS_16M, SYS_160M]
  */
void sys_clk_set(u32 clk_sel);

bool sys_clk_free_all(void);

/**
  * @brief Unset the value which be set by sys_clk_req
  * @param index: [INDEX_VOICE, INDEX_MAX_NB)
  * @return true or false
  */
bool sys_clk_free(uint8_t index);

/**
  * @brief Set the minimum clock frequency for the system
  * @attention  If the clock to be set is lower than the current clock of the system, the setting will not take effect now
  * @param index: [INDEX_VOICE, INDEX_MAX_NB) sys_clk:[SYS_16M - SYS_160M]
  * @return true or false
  */
bool sys_clk_req(uint8_t index, uint8_t sys_clk);

#endif

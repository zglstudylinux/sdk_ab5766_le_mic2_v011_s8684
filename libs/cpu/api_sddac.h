#ifndef __API_SDDAC_H
#define __API_SDDAC_H
#include "driver_sddac.h"

typedef enum {
    SDDAC_AU0_ALL_DONE  = BIT(0),
    SDDAC_AU1_ALL_DONE  = BIT(1),
} SDDAC_AU_TYPEDEF;

typedef void (*sddac_done_callback)(SDDAC_AU_TYPEDEF type);

/**
  * @brief  Init the dac analog.
  * @param  ana: sddac_ana_typedef
  */
void sddac_ana_init(sddac_ana_typedef *ana);

/**
  * @brief  Deinit the dac analog.
  * @param  None.
  */
void sddac_ana_deinit(void);

/**
  * @brief  Init the sddac sem.
  * @param  None.
  */
void sddac_sem_init(void);

/**
  * @brief  Set the sddac output completion callback function
            This callback function will be called after sddac_done_proc_kick is executed
  * @param  func:callback function
  */
void sddac_done_callback_set(sddac_done_callback func);

/**
  * @brief  Triggers the aupcm thread to handle the sddac callback function, which will be set by sddac_done_callback_set.
            This function is usually called inside the sddac interrupt function
  * @param  type:SDDAC_AU0_ALL_DONE or SDDAC_AU1_ALL_DONE
  */
void sddac_done_proc_kick(SDDAC_AU_TYPEDEF type);

void os_sem_au0_dma_take(int32_t time);
void os_sem_au0_dma_release(void);
void os_sem_au0_full_take(int32_t time);
void os_sem_au0_full_release(void);
void os_sem_au1_dma_take(int32_t time);
void os_sem_au1_dma_release(void);
void os_sem_au1_full_take(int32_t time);
void os_sem_au1_full_release(void);

void dac_power_off(void);
#endif //__API_SDDAC_H

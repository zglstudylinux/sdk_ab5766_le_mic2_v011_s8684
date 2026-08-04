#ifndef _API_SYS_H_
#define _API_SYS_H_

#define CACHE_ADDR          0x70000

extern const uint16_t libs_version;
#define LIBS_VERSION  libs_version

//error code
enum {
    ERROR_NO                            = 0x0000,
    ERROR_NULL_PTR                      = 0x0001,
    ERROR_INVALID_PARAM                 = 0x0002,
    ERROR_MEMORY_CAPA_EXCEED            = 0x0003,
    ERROR_COMMAND_DISALLOWED            = 0x0004,
    ERROR_UNSUPPORT                     = 0x0005,
    ERROR_TIMEOUT                       = 0x0006,
};


/**
  * @brief  Sets whether to call the tick interrupt callback function
  * @param  tmr5ms_en: set to true, usr_tmr5ms_thread_callback will be called once in 5ms
  * @param  tmr1ms_en: set to true, usr_tmr1ms_isr_callback will be called once in 1ms
  */
void sys_set_tmr_enable(bool tmr5ms_en, bool tmr1ms_en);

/**
  * @brief  system reset init
  * @param  wk pin 10s reset config
  * @return system reset source
    reset source:   BIT(31):PWRUP_RST
                    BIT(26):RTC_WDT_RST
                    BIT(25):GPIO_10S_RST
                    BIT(24):SW_RST
                    BIT(19):WK0_10S_RST
                    BIT(18):WAKEUP_RST
                    BIT(17):VUSB_RST
                    BIT(16):WDT_RST
                    BIT(20)|BIT(7):LVD_RST
  */
u32 sys_rst_init(bool wko10s_rst);

/**
  * @brief  dump the system reset source info
  * @param  reason: system reset source, can be get by the return value of the function sys_rst_init
  */
void sys_rst_dump(u32 reason);

/**
  * @brief  The system reset can be triggered by software
  * @param  software reset source [1 ~ 15], it can be define by the user
  */
void sw_reset_kick(u8 source);

/**
  * @brief  Get the software reset source,it must be called as soon as the system is reset
  * @return software reset source
  */
u8 sw_reset_source_get(void);

/**
  * @brief  Watch point can track the incoming addresses
            and print out the corresponding addresses of the functions that have been modified to those addresses.
            the interruption IRQn_RTC_LVD_WDT_WPT will also be registered by this function
  * @param  The address of the variable that needs to be tracked
  */
void watch_point_set(u32 addr);


void xosc_init(void);


/**
  * @brief  Gets a random number between 0 and num
  * @param  number
  */
u16 get_random(u16 num);

int s_abs(int x);

bool bt_get_ft_trim_value(void *rf_param);


/**
  * @brief  Specify a memory for malloc
  * @param  The memory address to be specified.
  * @param  The memory size to be specified.
  */
void heap_malloc_init(void *address, uint32_t size);

/**
  * @brief  Malloc a heap memory.
  * @param  Size of the allocated memory.
  * @ret    The allocated memory address. The actual memory consumed is (size + 8)
  */
void *heap_malloc(uint32_t size);

/**
  * @brief  realloc a heap memory.
  * @param  Address of the allocated memory.
  * @param  Size of new memory.
  * @ret    The new allocated memory address. The actual memory consumed is (new_size + 8)
  */
void *heap_realloc(void *address, uint32_t new_size);

/**
  * @brief  Free a allocated memory.
  * @param  The memory address to be freed.
  */
void heap_free(void *pv);

u32 sys_get_rand_key_init(uint32_t rand_seed);

void dev_init(u8 cfg);
bool dev_online_filter(u8 dev_num);
bool dev_offline_filter(u8 dev_num);


#endif // _API_SYS_H_


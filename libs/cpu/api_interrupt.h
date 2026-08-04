#ifndef _API_INTERRUPT_H
#define _API_INTERRUPT_H

#define GLOBAL_INT_DISABLE()    uint32_t cpu_ie = PICCON&BIT(0); PICCONCLR = BIT(0);asm volatile("":::"memory")
#define GLOBAL_INT_RESTORE()    asm volatile("":::"memory");PICCON |= cpu_ie

/**
 * @brief XBOX Interrupt Number Definition.
 */
typedef enum {
    IRQn_SOFT                       = 2,
    IRQn_TMR0,
    IRQn_TMR1,
    IRQn_TMR2,
    IRQn_IR_QDEC_LEDC,
    IRQn_USB_CONTROL,
    IRQn_KEYSCAN_AUBUFx             = 9,
    IRQn_SDADC,
    IRQn_PAR_ENC,
    IRQn_PACC,
    IRQn_UART                       = 14,
    IRQn_HSUART,
    IRQn_TMR3,
    IRQn_PWM,
    IRQn_DAC_DMA                    = 19,
    IRQn_SPIx,
    IRQn_FREQDET_TOUCH_KEY    = 23,
    IRQn_DAC_DMA_OUT,
    IRQn_PORT                       = 26,
    IRQn_DE_IIS_TDNN,
    IRQn_SARADC,
    IRQn_RTC_LVD_WDT_WPT,
    IRQn_IIC,
    IRQn_TICK0,
} IRQ_TYPEDEF;

typedef void (*isr_t)(void);

/**
 * @brief system interrupt init
 * @param interrupt number,define in IRQ_TYPEDEF
 * @param interrupt priority, 1 is high priority, 0 is low priority
 * @param interrupt callback function
 */
bool sys_irq_init(int vector, int pr, isr_t isr);

void wdt_isr(void);

void watch_point_isr(void);

#endif

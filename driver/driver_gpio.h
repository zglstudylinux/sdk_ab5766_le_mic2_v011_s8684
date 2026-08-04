/*
 * @File name    : driver_gpio.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-08
 * @Description  : This file contains all the functions prototypes for the GPIO library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_GPIO_H
#define _DRIVER_GPIO_H

#include "driver_com.h"

#define GPIO_EDGE_PENDING_CLR()     WKUPCPND = WKUPCPND_WKCPND_MASK;
#define GPIO_WK_SLEEP_EN()          WKUPCON |= WKUPCON_WKSLPEN_BIT;
#define GPIO_WK_SLEEP_DIS()         WKUPCON &= ~WKUPCON_WKSLPEN_BIT;

/**
 * @brief GPIO_Pins_Definition
 */
#define GPIO_PIN_ALL                ((uint16_t)0xffff)      /* All PIN bits definition */
#define GPIO_PIN_0                  ((uint16_t)0x0001)      /* PIN 0 bit definition */
#define GPIO_PIN_1                  ((uint16_t)0x0002)      /* PIN 1 bit definition */
#define GPIO_PIN_2                  ((uint16_t)0x0004)      /* PIN 2 bit definition */
#define GPIO_PIN_3                  ((uint16_t)0x0008)      /* PIN 3 bit definition */
#define GPIO_PIN_4                  ((uint16_t)0x0010)      /* PIN 4 bit definition */
#define GPIO_PIN_5                  ((uint16_t)0x0020)      /* PIN 5 bit definition */
#define GPIO_PIN_6                  ((uint16_t)0x0040)      /* PIN 6 bit definition */
#define GPIO_PIN_7                  ((uint16_t)0x0080)      /* PIN 7 bit definition */
#define GPIO_PIN_8                  ((uint16_t)0x0100)      /* PIN 8 bit definition */
#define GPIO_PIN_9                  ((uint16_t)0x0200)      /* PIN 9 bit definition */
#define GPIO_PIN_10                 ((uint16_t)0x0400)      /* PIN 10 bit definition */
#define GPIO_PIN_11                 ((uint16_t)0x0800)      /* PIN 11 bit definition */
#define GPIO_PIN_12                 ((uint16_t)0x1000)      /* PIN 12 bit definition */
#define GPIO_PIN_13                 ((uint16_t)0x2000)      /* PIN 13 bit definition */
#define GPIO_PIN_14                 ((uint16_t)0x4000)      /* PIN 14 bit definition */
#define GPIO_PIN_15                 ((uint16_t)0x8000)      /* PIN 15 bit definition */


/**
 * @brief GPIO Function Mapping Crossbar Peripheral Definition
 */
typedef enum {
    GPIO_CROSSBAR_OUT_I2CSCL    = 5,
    GPIO_CROSSBAR_OUT_I2CSDA,
    GPIO_CROSSBAR_OUT_PWM0      = 10,
    GPIO_CROSSBAR_OUT_PWM1,
    GPIO_CROSSBAR_OUT_PWM2,
    GPIO_CROSSBAR_OUT_PWM3,
    GPIO_CROSSBAR_OUT_UART0TX   = 17,
    GPIO_CROSSBAR_OUT_UART1TX,
    GPIO_CROSSBAR_OUT_HURTX,
    GPIO_CROSSBAR_OUT_HURRTS,
    GPIO_CROSSBAR_OUT_SPI1D0,
    GPIO_CROSSBAR_OUT_SPI1D1,
    GPIO_CROSSBAR_OUT_SPI1D2,
    GPIO_CROSSBAR_OUT_SPI1D3,
    GPIO_CROSSBAR_OUT_SPI1CLK,
    GPIO_CROSSBAR_OUT_LEDCDAT   = 27,
    GPIO_CROSSBAR_OUT_IRTX,
    GPIO_CROSSBAR_OUT_CLKOUT,

    GPIO_CROSSBAR_PERIPHERAL_INPUT_BASE,
    GPIO_CROSSBAR_IN_UART0RX,
    GPIO_CROSSBAR_IN_UART1RX,
    GPIO_CROSSBAR_IN_SPI1CLK,
    GPIO_CROSSBAR_IN_SPI1DI0,
    GPIO_CROSSBAR_IN_SPI1DI1,
    GPIO_CROSSBAR_IN_HURRX,
    GPIO_CROSSBAR_IN_HURCTS,
    GPIO_CROSSBAR_IN_T0INC,
    GPIO_CROSSBAR_IN_T1INC,
    GPIO_CROSSBAR_IN_T2CAP,
    GPIO_CROSSBAR_IN_T3CAP,
    GPIO_CROSSBAR_IN_RES,
    GPIO_CROSSBAR_IN_I2S_DI0,
    GPIO_CROSSBAR_IN_I2CSCL,
    GPIO_CROSSBAR_IN_I2CSDA,
    GPIO_CROSSBAR_IN_IRRX,
    GPIO_CROSSBAR_IN_QDECXDI0,
    GPIO_CROSSBAR_IN_QDECXDI1,
    GPIO_CROSSBAR_IN_PTMR0_CAP,
    GPIO_CROSSBAR_IN_PTMR1_CAP,
    GPIO_CROSSBAR_IN_PTMR2_CAP,
    GPIO_CROSSBAR_IN_PTMR3_CAP,
    GPIO_CROSSBAR_PERIPHERAL_MAX_IDX,
} GPIO_CROSSBAR_PERIPHERAL_DEF;

/**
 * @brief 10s reset source for chip
 */
typedef enum {
    GPIO_RSTSEL_MCLR,
    GPIO_RSTSEL_PA0,
    GPIO_RSTSEL_PA1,
    GPIO_RSTSEL_PA2,
    GPIO_RSTSEL_PA3,
    GPIO_RSTSEL_PA4,
    GPIO_RSTSEL_PA5,
    GPIO_RSTSEL_PA6,
    GPIO_RSTSEL_PA7,
    GPIO_RSTSEL_PA8,
    GPIO_RSTSEL_PA9,
    GPIO_RSTSEL_PA10,
    GPIO_RSTSEL_PA11,
    GPIO_RSTSEL_PA12,
    GPIO_RSTSEL_PA13,
    GPIO_RSTSEL_PA14,
    GPIO_RSTSEL_PA15,
    GPIO_RSTSEL_PB0,
    GPIO_RSTSEL_PB1,
    GPIO_RSTSEL_PB2,
    GPIO_RSTSEL_PB3,
    GPIO_RSTSEL_PB4,
    GPIO_RSTSEL_PB5,
    GPIO_RSTSEL_PB6,
    GPIO_RSTSEL_PB7,
    GPIO_RSTSEL_PB8,
    GPIO_RSTSEL_PB9,
    GPIO_RSTSEL_WK0,
} GPIO_RSTSEL_DEF;

typedef enum {
    GPIO_INT_PA0,
    GPIO_INT_PA1,
    GPIO_INT_PA2,
    GPIO_INT_PA3,
    GPIO_INT_PA4,
    GPIO_INT_PA5,
    GPIO_INT_PA6,
    GPIO_INT_PA7,
    GPIO_INT_PA8,
    GPIO_INT_PA9,
    GPIO_INT_PA10,
    GPIO_INT_PA11,
    GPIO_INT_PA12,
    GPIO_INT_PA13,
    GPIO_INT_PA14,
    GPIO_INT_PA15,
    GPIO_INT_PB0,
    GPIO_INT_PB1,
    GPIO_INT_PB2,
    GPIO_INT_PB3,
    GPIO_INT_PB4,
    GPIO_INT_PB5,
    GPIO_INT_PB6,
    GPIO_INT_PB7,
    GPIO_INT_PB8,
    GPIO_INT_PB9,
} GPIO_INT_SRC_TYPEDEF;

/**
 * @brief Bits detinition for GPIO
 */
typedef enum {
    BIT_RESET           = 0,
    BIT_SET,
} BIT_ACTION;

/**
 * @brief GPIO direction enumeration
 */
typedef enum {
    GPIO_DIR_OUTPUT     = 0x00,
    GPIO_DIR_INPUT      = 0x01,
} GPIO_DIR_TYPEDEF;

/**
 * @brief GPIO input pull resistance enumeration
 */
typedef enum {
    GPIO_PUPD_NULL      = 0x00,
    GPIO_PUPD_PU10K     = 0x01,
    GPIO_PUPD_PD10K     = 0x02,
    GPIO_PUPD_PU200K    = 0x03,
    GPIO_PUPD_PD200K    = 0x04,
    GPIO_PUPD_PU300     = 0x05,
    GPIO_PUPD_PD300     = 0x06,
} GPIO_PUPD_TYPEDEF;

/**
 * @brief GPIO port type enumeration
 */
typedef enum {
    GPIO_MODE_ANALOG    = 0x00,
    GPIO_MODE_DIGITAL   = 0x01,
} GPIO_MODE_TYPEDEF;

/**
 * @brief GPIO function mapping enable enumeration
 */
typedef enum {
    GPIO_FEN_GPIO       = 0x00,
    GPIO_FEN_PER        = 0x01,
} GPIO_FEN_TYPEDEF;

/**
 * @brief GPIO function mapping direction select register
 */
typedef enum {
    GPIO_FDIR_SELF      = 0x00,
    GPIO_FDIR_MAP       = 0x01,
} GPIO_FDIR_TYPEDEF;

/**
 * @brief GPIO output current enumeration, specially PB4 can output 64mA.
 */
typedef enum {
    GPIO_DRV_6MA,
    GPIO_DRV_24MA,
    GPIO_DRV_42MA,
    GPIO_DRV_60MA,
} GPIO_DRV_TYPEDEF;

typedef enum {
    GPIO_EDGE_FIL_DIS       = 0x00,
    GPIO_EDGE_FIL_2,
    GPIO_EDGE_FIL_8,
    GPIO_EDGE_FIL_32,
} GPIO_EDGE_FIL_SEL;

typedef enum {
    GPIO_EDGE_RISING        = 0x00,
    GPIO_EDGE_FALLING       = 0x01,
} GPIO_EDGE_SEL;

/**
 * @brief GPIO wakeup pin map enumeration, specially: one wkpinmap can have only one wkpinmap_int.
 */

typedef enum {
    GPIO_WK0MAP    = (1 << 0),
    GPIO_WK1MAP    = (1 << 1),
    GPIO_WK2MAP    = (1 << 2),
    GPIO_WK3MAP    = (1 << 3),
    GPIO_WK4MAP    = (1 << 4),
    GPIO_WK5MAP    = (1 << 5),
    GPIO_WK6MAP    = (1 << 6),
    GPIO_WK7MAP    = (1 << 7),
} GPIO_WKPINMAP_TYPEDEF;

typedef enum {
    GPIO_WK0_G0_PA7         = (0 << 0),
    GPIO_WK0_G1_PA0         = (1 << 0),
    GPIO_WK0_G2_PB0         = (2 << 0),
    GPIO_WK0_G3_PA5         = (3 << 0),

    GPIO_WK1_G0_PB1         = (0 << 2),
    GPIO_WK1_G1_PA1         = (1 << 2),
    GPIO_WK1_G2_PA10        = (2 << 2),
    GPIO_WK1_G3_RES         = (3 << 2),

    GPIO_WK2_G0_PB2         = (0 << 4),
    GPIO_WK2_G1_PA2         = (1 << 4),
    GPIO_WK2_G2_PB5         = (2 << 4),
    GPIO_WK2_G3_PA11        = (3 << 4),

    GPIO_WK3_G0_PB3         = (0 << 6),
    GPIO_WK3_G1_PA3         = (1 << 6),
    GPIO_WK3_G2_PA6         = (2 << 6),
    GPIO_WK3_G3_PB6         = (3 << 6),

    GPIO_WK4_G0_PB4         = (0 << 8),
    GPIO_WK4_G1_PA4         = (1 << 8),
    GPIO_WK4_G2_PA8         = (2 << 8),
    GPIO_WK4_G3_PB7         = (3 << 8),

    GPIO_WK5_G0_WK0         = (0 << 10),
    GPIO_WK5_G1_PA9         = (1 << 10),
    GPIO_WK5_G2_PA12        = (2 << 10),
    GPIO_WK5_G3_PB9         = (3 << 10),

    GPIO_WK6_G0_FALL        = (0 << 12),
    GPIO_WK6_G1_PA14        = (1 << 12),
    GPIO_WK6_G2_PB8         = (2 << 12),
    GPIO_WK6_G3_RES         = (3 << 12),

    GPIO_WK7_G0_RISE        = (0 << 14),
    GPIO_WK7_G1_PA15        = (1 << 14),
    GPIO_WK7_G2_PA13        = (2 << 14),
    GPIO_WK7_G3_RES         = (3 << 14),
} GPIO_WKPINMAP_INT_TYPEDEF;


/**
 * @brief GPIO Init Structure
 */
typedef struct {
    uint32_t              gpio_pin;
    GPIO_DIR_TYPEDEF      gpio_dir;
    GPIO_PUPD_TYPEDEF     gpio_pupd;
    GPIO_MODE_TYPEDEF     gpio_mode;
    GPIO_FEN_TYPEDEF      gpio_fen;
    GPIO_FDIR_TYPEDEF     gpio_fdir;
    GPIO_DRV_TYPEDEF      gpio_drv;
} gpio_init_typedef;

typedef struct {
    GPIO_EDGE_SEL                   edge;
    gpio_typedef*                   gpiox;
    uint16_t                        gpio_pin;
    GPIO_PUPD_TYPEDEF               gpio_pupd;
    GPIO_EDGE_FIL_SEL               filter;
    GPIO_WKPINMAP_TYPEDEF           wkpinmap;
    GPIO_WKPINMAP_INT_TYPEDEF       wkpinmap_int;
} gpio_edge_cap_typedef;


/************************ Function used to set or R/W GPIO *************************/

/**
  * @brief  Initializes the gpiox peripheral according to the specified
  *         parameters in the gpio_init_struct.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  gpio_init_struct: pointer to a gpio_init_typedef structure that
  *         contains the configuration information for the specified GPIO peripheral.
  * @retval None
  */
void gpio_init(gpio_typedef *gpiox, gpio_init_typedef *gpio_init_struct);

/**
 * @brief  De-initialize the specified GPIO peripheral.
 * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
 * @param  gpio_pin: specifies the port bits to be written. This parameter
 *         can be any combination of GPIO_PIN_x where x can be (0..15).
 * @retval None
 */
uint32_t gpio_deinit(gpio_typedef *gpiox, uint16_t gpio_pin);

/**
 * @brief  De-initialize the specified GPIO peripheral.
 * @param  gpiox: where x can be (B) to select the GPIO peripheral.
 * @param  gpio_pin: specifies the port bits to be written. This parameter
 *         can be any combination of GPIO_PIN_x where x can be (0..11).
 * @param  dir: GPIO_DIR_TYPEDEF.
 * @retval None
 */
void gpio_set_dir(gpio_typedef* gpiox, uint16_t gpio_pin, GPIO_DIR_TYPEDEF dir);

/**
  * @brief  Sets the selected data port bits.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  gpio_pin: specifies the port bits to be written. This parameter
  *         can be any combination of GPIO_PIN_x where x can be (0..15).
  * @retval None
  */
void gpio_set_bits(gpio_typedef *gpiox, uint16_t gpio_pin);

/**
  * @brief  Clears the selected data port bits.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  gpio_pin: specifies the port bits to be written.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15).
  * @retval None
  */
void gpio_reset_bits(gpio_typedef *gpiox, uint16_t gpio_pin);
void gpio_set_de(gpio_typedef* gpiox, uint16_t gpio_pin, uint16_t pin_de);
/**
  * @brief  Writes the specified GPIO output data port.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  port_val: specifies the port bits to be written. This parameter
  *         can be any combination of GPIO_PIN_x where x can be (0..15).
  * @retval None
  */
void gpio_write_data(gpio_typedef *gpiox, uint16_t port_val);

/**
  * @brief  Toggles the selected output data port bits.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  gpio_pin: specifies the port bits to be written. This parameter
  *         can be any combination of GPIO_PIN_x where x can be (0..15).
  * @retval None
  */
void gpio_toggle_bits(gpio_typedef *gpiox, uint16_t gpio_pin);

/**
  * @brief  Reads the specified GPIO input data port.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @retval GPIO output data port value.
  */
uint16_t gpio_read_data(gpio_typedef *gpiox);

/**
  * @brief  Reads the specified input port pin.
  * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
  * @param  gpio_pin: specifies the port bit to read.
  *         This parameter can be gpio_pin where x can be (0..15).
  * @retval The input port pin value.
  */
uint8_t gpio_read_bit(gpio_typedef *gpiox, uint16_t gpio_pin);

/**
 * @brief  Configure GPIO function mapping.
 * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
 * @param  gpio_pin: specifies the port bit to read.
 *         This parameter can be gpio_pin where x can be (0..15).
 * @param  func_idx: Peripheral index that need to config mapping. The value of this param
 *         see enumeration "GPIO_CROSSBAR_PERIPHERAL_DEF" in the file "driver_gpio.h".
 * @retval None
 */
void gpio_func_mapping_config(gpio_typedef* gpiox, uint16_t gpio_pin, GPIO_CROSSBAR_PERIPHERAL_DEF func_idx);

/**
 * @brief  Clear GPIO function mapping config.
 * @param  gpiox: where x can be (A.B) to select the GPIO peripheral.
 * @param  gpio_pin: specifies the port bit to read.
 *         This parameter can be gpio_pin where x can be (0..15).
 * @retval None
 */
void gpio_func_mapping_clear(gpio_typedef* gpiox, uint16_t gpio_pin);

/**
 * @brief  WK0 IO level detect config.
 * @param  pull_up_en: Whether WK0 internal pull up enable.
 * @param  pull_down_en: Whether WK0 internal pull down enable.
 * @param  input_en: Whether WK0 input enable.
 * @retval None
 */
void wko_io_config(FUNCTIONAL_STATE pull_up_en, FUNCTIONAL_STATE pull_down_en, FUNCTIONAL_STATE input_en);

/**
 * @brief  Read WK0 pin level status.
 * @retval The state of wk0 level.
 */
FLAG_STATE wko_io_read_bit(void);
void wk0_edge_capture_config(GPIO_EDGE_SEL edge);
void gpio_edge_capture_config(gpio_edge_cap_typedef* config);
void gpio_edge_capture_disable(void);
uint8_t gpio_edge_pending(void);
GPIO_INT_SRC_TYPEDEF gpio_get_fall_pending_src(void);
GPIO_INT_SRC_TYPEDEF gpio_get_rise_pending_src(void);
void gpio_edge_pic_config(isr_t isr, int pr, FUNCTIONAL_STATE state);

/**
 * @brief Select the 10s reset source.
 * @param  rst_pin: the reset source we choose.
 * @param  level:reset time from 0 to 7.
 *         reset time = (262144 + level * 65535) / Frc, the units is second.
 * @param  enable: whether enable 10s reset or not.
 * @retval None
 */
void gpio_reset_init(GPIO_RSTSEL_DEF rst_pin, uint8_t level, FUNCTIONAL_STATE enable);

#endif  // _DRIVER_GPIO_H

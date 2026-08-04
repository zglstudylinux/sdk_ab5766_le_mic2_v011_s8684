/*
 * @File name    : driver_com.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-11
 * @Description  : AB202X Device Peripheral Access Layer Header File.
 *                 This file contains all the peripheral register's definitions, bits
 *                 definitions and memory mapping for AB202X devices.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_COM_H
#define _DRIVER_COM_H

#include "global.h"
#include "api.h"

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FUNCTIONAL_STATE;
typedef enum {RESET = 0, SET = !RESET} FLAG_STATE, IT_STATE;
typedef enum {FAILED = 0, SUCCESS = !FAILED} STATUS_STATE;


#include "driver_gpio_reg.h"
#include "driver_pwm_reg.h"
#include "driver_tmr_reg.h"
#include "driver_uart_reg.h"
#include "driver_hsuart_reg.h"
#include "driver_keyscan_reg.h"
#include "driver_iic_reg.h"
#include "driver_ir_reg.h"
#include "driver_iis_reg.h"
#include "driver_ledc_reg.h"
#include "driver_qdec_reg.h"
#include "driver_saradc_reg.h"
#include "driver_sdadc_reg.h"
#include "driver_sddac_reg.h"
#include "driver_spi_reg.h"
#include "driver_despi_reg.h"
#include "driver_touch_key_reg.h"
#include "driver_wdt_reg.h"
#include "driver_rtc_reg.h"
#include "driver_clk.h"
#include "driver_charge.h"

#endif // _DRIVER_COM_H

/*
 * @File name    : driver_gpio.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-29
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the GPIO peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_gpio.h"

#define GPIO_TYPE3(gpiox, pin_idx)           ((GPIOB_REG == gpiox) && (GPIO_PIN_4 == (1 << pin_idx)))


void gpio_init(gpio_typedef* gpiox, gpio_init_typedef* gpio_init_struct)
{
    u32 reg;
    uint32_t pin_idx = 0, pin_bit_offset = 0x00;
    uint8_t i = 0x00;

    /*--- Configure the port pins ---*/
    for (pin_idx = 0; pin_idx < 16; pin_idx++) {
        pin_bit_offset = ((uint32_t)0x01) << pin_idx;
        if (gpio_init_struct->gpio_pin & pin_bit_offset) {
            /*--- Input or Output Configuration ---*/
            reg = gpiox->dir;
            reg &= ~pin_bit_offset;
            reg |= ((uint32_t)(gpio_init_struct->gpio_dir) << pin_idx);
            gpiox->dir = reg;

            /*--- Digital or Analog Configuration ---*/
            reg = gpiox->de;
            reg &= ~pin_bit_offset;
            reg |= ((uint32_t)(gpio_init_struct->gpio_mode) << pin_idx);
            gpiox->de = reg;

            /*--- Function Mapping Enable Configuration ---*/
            reg = gpiox->fen;
            reg &= ~pin_bit_offset;
            reg |= ((uint32_t)(gpio_init_struct->gpio_fen) << pin_idx);
            gpiox->fen = reg;

            /*--- GPIO Direction Select Configguratiopn ---*/
            reg = gpiox->fdir;
            reg &= ~pin_bit_offset;
            reg |= ((uint32_t)(gpio_init_struct->gpio_fdir) << pin_idx);
            gpiox->fdir = reg;

            /*--- Pull Up or Pull Down Configuration and clear driving cfg ---*/
            if (gpio_init_struct->gpio_dir == GPIO_DIR_INPUT) {
                gpiox->drv &= ~pin_bit_offset;
                for (i = 0; i < 6; i++) {
                    reg = gpiox->pupd[i];
                    reg &= ~pin_bit_offset;
                    if ((i + 1) == gpio_init_struct->gpio_pupd) {
                        reg |= pin_bit_offset;
                    }
                    gpiox->pupd[i] = reg;
                }
            /*--- Output driving Configuration and clear pupu cfg ---*/
            } else if (gpio_init_struct->gpio_dir == GPIO_DIR_OUTPUT) {
                for (i = 0; i < 6; i++) {
                    gpiox->pupd[i] &= ~pin_bit_offset;
                }
                if (gpio_init_struct->gpio_mode == GPIO_MODE_DIGITAL) {
                    if (GPIO_TYPE3(gpiox, pin_idx)) {
                        reg = gpiox->drv;
                        reg = (reg & ~(3 << 10)) | ((uint32_t)(gpio_init_struct->gpio_drv << 10));
                        gpiox->drv = reg;
                    } else {
                        if (gpio_init_struct->gpio_drv < GPIO_DRV_42MA) {
                            reg = gpiox->drv;
                            reg &= ~pin_bit_offset;
                            reg |= ((uint32_t)(gpio_init_struct->gpio_drv) << pin_idx);
                            gpiox->drv = reg;
                        }
                    }
                }
            }
        }
    }
}

uint32_t gpio_deinit(gpio_typedef *gpiox, uint16_t gpio_pin)
{
    u32 reg_original_sta;
    if (gpiox == GPIOB_REG) {
        gpio_pin &= 0x3ff;
    }

    reg_original_sta = gpiox->de;
    gpiox->de &= ~gpio_pin;

    return reg_original_sta;
}


AT(.com_periph.gpio.dir)
void gpio_set_dir(gpio_typedef* gpiox, uint16_t gpio_pin, GPIO_DIR_TYPEDEF dir)
{
    if (dir) {
        gpiox->dir |= gpio_pin;
    } else {
        gpiox->dir &= ~gpio_pin;
    }
}


AT(.com_periph.gpio.set)
void gpio_set_bits(gpio_typedef* gpiox, uint16_t gpio_pin)
{
    gpiox->io_set = gpio_pin;
}

AT(.com_periph.gpio.reset)
void gpio_reset_bits(gpio_typedef* gpiox, uint16_t gpio_pin)
{
    gpiox->io_clr = gpio_pin;
}

AT(.com_periph.gpio.de)
void gpio_set_de(gpio_typedef* gpiox, uint16_t gpio_pin, uint16_t pin_de)
{
    if (pin_de){
        gpiox->de |= gpio_pin;
    } else{
        gpiox->de &=~ gpio_pin;
    }
}

AT(.com_periph.gpio.write)
void gpio_write_data(gpio_typedef *gpiox, uint16_t port_val)
{
    gpiox->data = port_val;
}


AT(.com_periph.gpio.toggle)
void gpio_toggle_bits(gpio_typedef *gpiox, uint16_t gpio_pin)
{
    gpiox->data ^= gpio_pin;
}


AT(.com_periph.gpio.read_data)
uint16_t gpio_read_data(gpio_typedef* gpiox)
{
    return ((uint16_t)(gpiox->data));
}


AT(.com_periph.gpio.read_bit)
uint8_t gpio_read_bit(gpio_typedef* gpiox, uint16_t gpio_pin)
{
    uint8_t bit_status;

    if ((gpiox->data & gpio_pin) != BIT_RESET) {
        bit_status = (uint8_t)SET;
    } else {
        bit_status = (uint8_t)RESET;
    }

    return bit_status;
}


void gpio_func_mapping_config(gpio_typedef* gpiox, uint16_t gpio_pin, GPIO_CROSSBAR_PERIPHERAL_DEF func_idx)
{
    u8 i;
    u8 pin_idx;
    uint8_t register_idx, register_offset;
    u8 gpio_crossbar_idx_PA0 = 1;
    u8 gpio_crossbar_idx_PB0 = 17;

    /*--- Calculate the pin_dix for function mapping ---*/
    for (i = 0; i < 16; i++) {
        if ((1 << i) == gpio_pin) {
            break;
        }
    }
    if (gpiox == GPIOA_REG) {
        pin_idx = gpio_crossbar_idx_PA0 + i;
    } else {
        pin_idx = gpio_crossbar_idx_PB0 + i;
    }

    /*--- Check parameter validity ---*/
    if (func_idx >= GPIO_CROSSBAR_PERIPHERAL_MAX_IDX) {
        return;
    }
    if ((gpiox == GPIOB_REG) && (gpio_pin > GPIO_PIN_9)) {
        return;
    }

    /*--- function mapping config ---*/
    if (func_idx > GPIO_CROSSBAR_PERIPHERAL_INPUT_BASE) {
        register_idx = (func_idx - GPIO_CROSSBAR_PERIPHERAL_INPUT_BASE - 1) / 4;
        register_offset = (func_idx - GPIO_CROSSBAR_PERIPHERAL_INPUT_BASE - 1) % 4 * 8;

        /* Do nothing if the current peripheral is already mapped to the corresponding IO */
        if (((FUNCMAP->func_input_map[register_idx] >> register_offset) & 0x1f) == pin_idx) {
            return;
        }

        FUNCMAP->func_input_map[register_idx] |= (uint32_t)(0xff << register_offset);
        FUNCMAP->func_input_map[register_idx] |= (uint32_t)(pin_idx << register_offset);
    } else if (func_idx < GPIO_CROSSBAR_PERIPHERAL_INPUT_BASE) {
        register_idx = (pin_idx - 1) / 4;
        register_offset = (pin_idx - 1) % 4 * 8;

        /* Do nothing if the current peripheral is already mapped to the corresponding IO */
        if (((FUNCMAP->func_output_map[register_idx] >> register_offset) & 0x1f) == func_idx) {
            return;
        }

        FUNCMAP->func_output_map[register_idx] |= (uint32_t)(0xff << register_offset);
        FUNCMAP->func_output_map[register_idx] |= (uint32_t)(func_idx << register_offset);
    }
}


void gpio_func_mapping_clear(gpio_typedef* gpiox, uint16_t gpio_pin)
{
    uint8_t register_idx, register_offset;
    uint8_t i, j;
    u8 pin_idx;
    u8 gpio_crossbar_idx_PA0 = 1;
    u8 gpio_crossbar_idx_PB0 = 17;

    /* Calculate the pin_dix for function mapping */
    for (i = 0; i < 16; i++) {
        if ((1 << i) == gpio_pin) {
            break;
        }
    }
    if (gpiox == GPIOA_REG) {
        pin_idx = gpio_crossbar_idx_PA0 + i;
    } else {
        pin_idx = gpio_crossbar_idx_PB0 + i;
    }

    /* Clear output mapping */
    register_idx = (pin_idx - 1) / 4;
    register_offset = (pin_idx - 1) % 4 * 8;
    FUNCMAP->func_output_map[register_idx] |= (uint32_t)(0xff << register_offset);

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 32; j += 8)
        if (((FUNCMAP->func_input_map[i] >> j) & 0x1f) == pin_idx) {
            FUNCMAP->func_input_map[i] |= (uint32_t)(0xff << j);
        }
    }
}

void wko_io_config(FUNCTIONAL_STATE pull_up_en, FUNCTIONAL_STATE pull_down_en, FUNCTIONAL_STATE input_en)
{
    if (pull_up_en) {
        RTCCON1 |= (uint32_t)(1 << 4);
    } else {
        RTCCON1 &= ~(uint32_t)(1 << 4);
    }

    if (pull_down_en) {
        RTCCON1 |= (uint32_t)(1 << 1);
        RTCCON11 |= (uint32_t)(1 << 4);     // Sure this bit is enable, and recommends it is enabled normally.
    } else {
        RTCCON1 &= ~(uint32_t)(1 << 1);
    }

    if (input_en) {
        RTCCON1 |= (uint32_t)(1 << 0);
    } else {
        RTCCON1 &= ~(uint32_t)(1 << 0);
    }
}

AT(.com_periph.wk0.read_bit)
FLAG_STATE wko_io_read_bit(void)
{
    if (RTCCON & (1 << 19)) {
        return SET;
    } else {
        return RESET;
    }
}

void wk0_edge_capture_config(GPIO_EDGE_SEL edge)
{
    clk_gate0_cmd(CLK_GATE0_LP, CLK_EN);

    if(edge == GPIO_EDGE_FALLING){
        wko_io_config(1, 0, 1);
        WKUPEDG |= GPIO_WK5MAP;
    }else{
        wko_io_config(0, 1, 1);
        WKUPEDG &= ~GPIO_WK5MAP;
    }

    WKPINMAP = (WKPINMAP & ~WKPINMAP_WK5MAP_MASK) | GPIO_WK5_G0_WK0;

    WKUPCON |= GPIO_WK5MAP;

    WKUPCPND = WKUPCPND_WKCPND_MASK;
}


void gpio_edge_capture_config(gpio_edge_cap_typedef* config)
{
    clk_gate0_cmd(CLK_GATE0_LP, CLK_EN);

    if ((config->wkpinmap == GPIO_WK5MAP) && (config->wkpinmap_int == GPIO_WK5_G0_WK0))  {
        wk0_edge_capture_config(config->edge);
    } else {
        gpio_init_typedef gpio_config;
        uint32_t gpio_pin = config->gpio_pin;
        uint8_t wkmap_idx;

        if(config->gpiox == GPIOB_REG){
            gpio_pin = ((uint32_t)config->gpio_pin) << 16;
        }
        gpio_config.gpio_pin = config->gpio_pin;
        gpio_config.gpio_dir = GPIO_DIR_INPUT;
        gpio_config.gpio_fen = GPIO_FEN_GPIO;
        gpio_config.gpio_fdir = GPIO_FDIR_SELF;
        gpio_config.gpio_mode = GPIO_MODE_DIGITAL;
        gpio_config.gpio_pupd = config->gpio_pupd;

        gpio_init(config->gpiox, &gpio_config);

        for (wkmap_idx = 0; wkmap_idx < 8; wkmap_idx++) {
            if (config->wkpinmap & BIT(wkmap_idx)) {
                break;
            }
        }

        if(config->edge == GPIO_EDGE_RISING){
            WKUPCON = (WKUPCON & ~WKUPCON_PRISE_FIL_MASK) | (config->filter << WKUPCON_PRISE_FIL_LSB);
            if (config->wkpinmap_int == GPIO_WK7_G0_RISE) {
                PORTINTEDG &= ~gpio_pin;
                PORTINTEN |= gpio_pin;
            }
            WKUPEDG &= ~config->wkpinmap;
        }else{
            WKUPCON = (WKUPCON & ~WKUPCON_PFALL_FIL_MASK) | (config->filter << WKUPCON_PFALL_FIL_LSB);
            if (config->wkpinmap_int == GPIO_WK6_G0_FALL) {
                PORTINTEDG |= gpio_pin;
                PORTINTEN |= gpio_pin;
            }
            WKUPEDG |= config->wkpinmap;

        }
        WKPINMAP = (WKPINMAP & ~(WKPINMAP_WKMAP_MASK << (wkmap_idx * WKPINMAP_WKMAP_LEN + WKPINMAP_WKMAP_OFFSET))) | config->wkpinmap_int;
        WKUPCON |= config->wkpinmap;
    }

    WKUPCPND = WKUPCPND_WKCPND_MASK;
}

void gpio_edge_capture_disable(void)
{
    WKUPCON = 0;
    WKUPEDG = 0;
    PORTINTEDG = 0;
    PORTINTEN = 0;
}

AT(.com_periph.gpio.rise.src)
GPIO_INT_SRC_TYPEDEF gpio_get_rise_pending_src(void)
{
    return WKRSRC;
}

AT(.com_periph.gpio.fall.src)
GPIO_INT_SRC_TYPEDEF gpio_get_fall_pending_src(void)
{
    return ~WKFSRC;
}

AT(.com_periph.gpio.pending_is)
uint8_t gpio_edge_pending(void)
{
   return (WKUPEDG & WKUPEDG_WKPND_MASK) >> WKUPEDG_WKPND_LSB;
}


void gpio_edge_pic_config(isr_t isr, int pr, FUNCTIONAL_STATE state)
{
    if (state == ENABLE) {
        sys_irq_init(IRQn_PORT, pr, isr);

        WKUPCPND = WKUPCPND_WKCPND_MASK;
        WKUPCON |= WKUPCON_WKIE_BIT;
    } else {
        PICEN &= ~BIT(IRQn_PORT);
        WKUPCON &= ~WKUPCON_WKIE_BIT;
    }
}

void gpio_reset_init(GPIO_RSTSEL_DEF rst_pin, uint8_t level, FUNCTIONAL_STATE enable)
{
    if (ENABLE == enable) {
        WKUPCON = (WKUPCON & ~WKUPCON_RSTPINSEL_MASK) | (rst_pin << WKUPCON_RSTPINSEL_LSB) | WKUPCON_RSTPINSELWV_BIT;

        if (GPIO_RSTSEL_WK0 == rst_pin) {
            if (level <= 7) {
                RTCCON12 = (RTCCON12 &~(0x07 << 8)) | (level << 8) | BIT(11);
            }
            RTCCON12 &= ~(3<<0);
        } else {
            if (level <= 7) {
                RTCCON12 = (RTCCON12 &~(0x07 << 12)) | (level << 12) | BIT(15);
            }
            RTCCON12 &= ~(3<<4);
        }
    } else {
        WKUPCON = (WKUPCON & ~WKUPCON_RSTPINSEL_MASK) | (0 << WKUPCON_RSTPINSEL_LSB) | WKUPCON_RSTPINSELWV_BIT;

        if (GPIO_RSTSEL_WK0 == rst_pin) {
            RTCCON12 |= (3 << 0);
        } else {
            RTCCON12 |= (3 << 4);
        }
    }
}

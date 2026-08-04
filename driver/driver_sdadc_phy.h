#ifndef _DRIVER_SDADC_PHY_H
#define _DRIVER_SDADC_PHY_H
#include "include.h"

#define CHANNEL_L               0x000F
#define CHANNEL_R               0x00F0
#define CHANNEL_M               0x0F00

#define CH_MICM0                0xC00   /* MICM       -> adcm mic */
#define CH_MICL0                0x0C    /* MICL       -> left mic */
#define CH_MICR0                0xC0    /* MICR       -> right mic */

void my_spi_init_test(void);
void my_spi_putc_test(u8 ch);

void fpga_uart_putchar (uint8_t auphy, char ch);
void fpga_uart_init (void);
void bsp_set_auphy_spr (uint8_t auphy, uint8_t spr);
void bsp_auphy_adc_mode (uint8_t auphy, uint16_t mode);
void bsp_auphy_vcmbuf_en (void);
void bsp_auphy_set_dac_volume (uint8_t vol);
void bsp_auphy_change_volume (uint8_t vol);
void bsp_auphy_set_adc_analog_gain (uint8_t auphy, uint16_t channel, uint8_t gain);
void audio_fpga_setup (uint16_t channel, uint8_t spr);

#endif  //_DRIVER_SDADC_PHY_H
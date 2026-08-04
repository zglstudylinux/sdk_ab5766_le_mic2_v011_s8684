#include "include.h"
#include "driver_sdadc_phy.h"

#if FPGA_EN

#define CONST_CAT(x, y)     x ## y
#define SET_MACRO(x, y)     CONST_CAT(x, y)

#define CS_GP               A
#define SCLK_GP             A
#define SDAT_GP             A
#define CS_BIT              BIT(3)
#define SCLK_BIT            BIT(5)
#define SDAT_BIT            BIT(7)

#define CS_GPIODE           SET_MACRO(GPIO, SET_MACRO(CS_GP, DE))
#define CS_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(CS_GP, FEN))
#define CS_GPIODIR          SET_MACRO(GPIO, SET_MACRO(CS_GP, DIR))
#define CS_GPIOSET          SET_MACRO(GPIO, SET_MACRO(CS_GP, SET))
#define CS_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(CS_GP, CLR))
#define SCLK_GPIODE         SET_MACRO(GPIO, SET_MACRO(SCLK_GP, DE))
#define SCLK_GPIOFEN        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, FEN))
#define SCLK_GPIODIR        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, DIR))
#define SCLK_GPIOSET        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, SET))
#define SCLK_GPIOCLR        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, CLR))
#define SDAT_GPIODE         SET_MACRO(GPIO, SET_MACRO(SDAT_GP, DE))
#define SDAT_GPIOFEN        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, FEN))
#define SDAT_GPIODIR        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, DIR))
#define SDAT_GPIOSET        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, SET))
#define SDAT_GPIOCLR        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, CLR))

AT(.com_text.spi_init)
void my_spi_init_test(void)
{
    CS_GPIODE |= CS_BIT;
    CS_GPIOFEN &= ~CS_BIT;
    CS_GPIODIR &= ~CS_BIT;
    CS_GPIOSET = CS_BIT;

    SCLK_GPIODE |= SCLK_BIT;
    SCLK_GPIOFEN &= ~SCLK_BIT;
    SCLK_GPIODIR &= ~SCLK_BIT;
    SCLK_GPIOSET = SCLK_BIT;

    SDAT_GPIODE |= SDAT_BIT;
    SDAT_GPIOFEN &= ~SDAT_BIT;
    SDAT_GPIODIR &= ~SDAT_BIT;
    SDAT_GPIOSET = SDAT_BIT;
}

AT(.com_text.putc)
void my_spi_putc_test(u8 ch)
{
    GLOBAL_INT_DISABLE();
    CS_GPIOCLR = CS_BIT;
    asm("nop");asm("nop");
    asm("nop");asm("nop");
    asm("nop");asm("nop");

    u8 i;
    for(i=0; i<8; i++) {
        SCLK_GPIOCLR = SCLK_BIT;

        if(ch & 0x80) {
            SDAT_GPIOSET = SDAT_BIT;
        } else {
            SDAT_GPIOCLR = SDAT_BIT;
        }
        ch <<= 1;
        SCLK_GPIOSET = SCLK_BIT;
    }

    asm("nop");asm("nop");
    asm("nop");asm("nop");
    CS_GPIOSET = CS_BIT;
    GLOBAL_INT_RESTORE();
}

void fpga_uart_putchar (uint8_t auphy, char ch)
{
    if (auphy & BIT(0)) {
        while (! (FPGAUTCON & BIT(8)));
        FPGAUTDATA = ch;

//        while (! (FPGAUTCON2 & BIT(8)));
//        FPGAUTDAT2 = ch;
    }
}

void fpga_uart_init (void)
{
    uint32_t uart_baud;
    //XOS24M
    uart_baud = (sys_clk_nhz_get() + 115200/2) / 115200 - 1;

    FPGAUTBAUD  = (uart_baud << 16) | uart_baud;
    FPGAUTCON  |= BIT(7) | BIT(0);

//    FPGAUTBAUD2 = (uart_baud << 16) | uart_baud;
//    FPGAUTCON2 |= BIT(7) | BIT(0);
}

// ����AUPHY��SDADC������
void bsp_set_auphy_spr (uint8_t auphy, uint8_t spr)
{
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'S');
    fpga_uart_putchar (auphy, 0x30+spr);
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// �л�AUX��MIC����
void bsp_auphy_adc_mode (uint8_t auphy, uint16_t mode)
{
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'M');
    fpga_uart_putchar (auphy, 'M');
    fpga_uart_putchar (auphy, (mode>>8) & 0xff );
    fpga_uart_putchar (auphy,  mode & 0xff     );
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// ����VCMBUF
void bsp_auphy_vcmbuf_en (void)
{
    const uint8_t auphy = 1;
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'D');
    fpga_uart_putchar (auphy, 'V');
    fpga_uart_putchar (auphy, '1');
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// vol: 0~16
void bsp_auphy_set_dac_volume (uint8_t vol)
{
    const uint8_t auphy = 1;
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'V');
    fpga_uart_putchar (auphy, 'V'); // �����1byte
    fpga_uart_putchar (auphy, 'V');
    fpga_uart_putchar (auphy, vol);
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// call by bsp_change_volume
void bsp_auphy_change_volume (uint8_t vol)
{
    const uint8_t auphy = 1;
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'V');
    fpga_uart_putchar (auphy, '0'+!!vol);
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// Channel: 'L/R/S', Gain:0~3`
void bsp_auphy_set_adc_analog_gain (uint8_t auphy, uint16_t channel, uint8_t gain)
{
    fpga_uart_putchar (auphy, '#');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'U');
    fpga_uart_putchar (auphy, 'A');
    fpga_uart_putchar (auphy, 'M');
    fpga_uart_putchar (auphy, 'V');
    fpga_uart_putchar (auphy, (channel>>8) & 0xff);
    fpga_uart_putchar (auphy,  channel & 0xff    );
    fpga_uart_putchar (auphy, gain);
    fpga_uart_putchar (auphy, '*');
    delay_ms (10);
}

// platform/bsp/bsp_audio.c
void audio_fpga_setup (uint16_t channel, uint8_t spr)
{
    // auphy1����
    if ((channel & CHANNEL_L) || (channel & CHANNEL_R) || (channel & CHANNEL_M)) {
        if        (channel == CH_MICL0) {
            // ������MIC
            FPGATEST |= BIT(8);         // ���BIT�޸�phy MIC gain�����쳣
            bsp_auphy_adc_mode (1, CH_MICL0);
        } else if (channel == CH_MICR0) {
            // ������MIC
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICR0);
        } else if (channel == CH_MICR0) {
            // ����MICM
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICM0);
        } else if (channel == (CH_MICL0 | CH_MICR0)) {
            // ����˫MIC LR
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICL0 | CH_MICR0);
        } else if (channel == (CH_MICL0 | CH_MICR0)) {
            // ����˫MIC LM
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICL0 | CH_MICM0);
        } else if (channel == (CH_MICL0 | CH_MICR0)) {
            // ����˫MIC RM
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICM0 | CH_MICR0);
        } else if (channel == (CH_MICL0 | CH_MICR0 | CH_MICM0)) {
            // ������MIC
            FPGATEST |= BIT(8);
            bsp_auphy_adc_mode (1, CH_MICM0 | CH_MICL0 | CH_MICR0);
        } else if (channel == CHANNEL_L) {
            // ��SDADC����˫��
            bsp_auphy_adc_mode (1, 2);
        }
    }
}

#endif  //FPGA_EN

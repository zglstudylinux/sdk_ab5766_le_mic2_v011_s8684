#include "include.h"
#include "driver_saradc.h"
#include "bsp_saradc.h"
#include "bsp_saradc_vbat.h"

#if BSP_SARADC_EN

void bsp_saradc_init(void)
{
    clk_gate0_cmd(CLK_GATE0_SARADC, CLK_EN);

    saradc_base_init_typedef saradc_base_init_struct;

    saradc_base_init_struct.mode = SARADC_MODE_NORMAL;
    saradc_base_init_struct.baud = 200000;
    saradc_base_init_struct.auto_analog = SARADC_AUTO_ENABLE_ANALOG_EN;
    saradc_base_init_struct.auto_analog_io = SARADC_AUTO_ENABLE_ANALOG_IO_EN;

    saradc_init(&saradc_base_init_struct);

    saradc_cmd(ENABLE);
}

#endif

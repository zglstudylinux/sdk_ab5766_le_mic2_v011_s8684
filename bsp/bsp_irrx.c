#include "driver_ir.h"
#include "driver_gpio.h"
#include "bsp_ir.h"

static uint8_t irrx_flag = 0;   //0:nothing; 1:rx done; 2: key release;
static int16_t repeat_cnt = -1;
static uint32_t irrx_data = 0;

void irrx_io_init(void)
{
    gpio_init_typedef gpio_init_structure;

    gpio_init_structure.gpio_pin = IRRX_IO_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_MAP;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;

    gpio_init(IRRX_IO_PORT, &gpio_init_structure);
    gpio_func_mapping_config(IRRX_IO_PORT, IRRX_IO_PIN, GPIO_CROSSBAR_IN_IRRX);
}

AT(.com_text.irq)
void irrx_irq_handler(void)
{
    if (irrx_get_flag(IRRX_REG, IRRX_FLAG_RXPND)) {
        irrx_clear_flag(IRRX_REG, IRRX_FLAG_RXPND);
        irrx_flag = 1;
        irrx_data = irrx_get_data(IRRX_REG);
    }

    if (irrx_get_flag(IRRX_REG, IRRX_FLAG_KEYRELS)) {
        irrx_clear_flag(IRRX_REG, IRRX_FLAG_KEYRELS);
        irrx_flag = 2;
        irrx_data = 0;
    }
}

void irrx_example_init(void)
{
    irrx_init_typedef irrx_param;

    clk_gate1_cmd(CLK_GATE1_IRRX, CLK_EN);
    clk_gate2_cmd(CLK_GATE2_IRFLT, CLK_EN);
    clk_irrx_clk_set(CLK_IR_CLK_X24M_CLKDIV24);

    irrx_io_init();

    irrx_param.data_bits = IRRX_DATA_SELECT_32BIT;
    irrx_param.filter.filter_en = ENABLE;
    irrx_param.filter.filter_src = IRTX_FLT_SRC_RXIN;
    irrx_param.filter.filter_len = 3;

    irrx_init(IRRX_REG, &irrx_param);

    irrx_pic_config(IRRX_REG, irrx_irq_handler, 0, IRRX_FLAG_RXPND | IRRX_FLAG_KEYRELS, ENABLE);

    irrx_cmd(IRRX_REG, ENABLE);
}

/**
  * @brief  irrx fiq hanlder.Call it in main
  * @retval None
  */
void irrx_handler_example(void)
{
    static uint32_t tick = -1;

    if (irrx_flag == 1) {
        // key down
        printf("irrx key down, irrx_data:0x%x\n", irrx_data);
        irrx_flag = 0;
        repeat_cnt = 0;
        tick = tick_get();
    } else if (irrx_flag == 2) {
        //key up
        printf("irrx key up\n");
        irrx_flag = 0;
        repeat_cnt = -1;
    }

    if ((repeat_cnt != -1) && tick_check_expire(tick, 110)) {
        //key hold
        tick = tick_get();
        repeat_cnt++;
        printf("repeat_cnt:0x%x\n", repeat_cnt);
    }
}
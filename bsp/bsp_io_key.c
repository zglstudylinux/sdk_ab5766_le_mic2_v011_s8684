#include "include.h"
#include "bsp_io_key.h"
#include "driver_lowpwr.h"

#if BSP_IOKEY_EN

#define GPIO_SET_DIR_OUT(x, y)                  {x->dir&=~y;x->drv&=~y;delay_us(30);}
#define GPIO_SET_DIR_IN(x, y)                   {x->dir|=y;x->pupd[0]|=y;x->pupd[1]&=~y;delay_us(30);}

key_io_typedef key_io_table_column[IO_KEY_COL_TABLE_SIZE] = {
//    {GPIOA_REG, GPIO_PIN_0},
//    {GPIOA_REG, GPIO_PIN_1},
};

#if IO_KEY_SCAN_MODE
key_io_typedef key_io_table_row[IO_KEY_ROW_TABLE_SIZE] = {
    {GPIOA_REG, GPIO_PIN_0},
    {GPIOA_REG, GPIO_PIN_1},
    {GPIOA_REG, GPIO_PIN_2},
    {GPIOA_REG, GPIO_PIN_3},
    {GPIOA_REG, GPIO_PIN_4},
    {GPIOA_REG, GPIO_PIN_5},
};
#endif // IO_KEY_SCAN_MODE

void bsp_io_key_init(void)
{
    gpio_init_typedef gpio_init_structure;

    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_GPIO;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;

#if IO_KEY_WK0_EN
    /* KEY0(WK0/PP) Init */
    wko_io_config(ENABLE, DISABLE, ENABLE);
#endif

    /* KEYx Init */
    for (u8 i = 0; i < IO_KEY_COL_TABLE_SIZE; i++) {
        gpio_init_structure.gpio_pin = key_io_table_column[i].gpio_pin;
        gpio_init(key_io_table_column[i].gpiox, &gpio_init_structure);
    }

#if IO_KEY_SCAN_MODE
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    for (u8 i = 0; i < IO_KEY_ROW_TABLE_SIZE; i++) {
        gpio_init_structure.gpio_pin = key_io_table_row[i].gpio_pin;
        gpio_init(key_io_table_row[i].gpiox, &gpio_init_structure);
        gpio_reset_bits(key_io_table_row[i].gpiox, key_io_table_row[i].gpio_pin);
    }
#endif // IO_KEY_SCAN_MODE
}

AT(.com_text.key.scan)
u8 bsp_get_io_key_id(void)
{
#if IO_KEY_WK0_EN
    if (wko_io_read_bit() == RESET) {                                   // KEY0(WK0/PP)
        return KEY_ID_PP;
    }
#endif

#if IO_KEY_SCAN_MODE
    u8 column = 0;
    u8 row = 0;

    for (u8 i = 0; i < IO_KEY_COL_TABLE_SIZE; i++) {
        GPIO_SET_DIR_IN(key_io_table_column[i].gpiox, key_io_table_column[i].gpio_pin);
        column |= (gpio_read_bit(key_io_table_column[i].gpiox, key_io_table_column[i].gpio_pin))? 0x00: (0x01 << i);
        GPIO_SET_DIR_OUT(key_io_table_column[i].gpiox, key_io_table_column[i].gpio_pin);
        gpio_reset_bits(key_io_table_column[i].gpiox, key_io_table_column[i].gpio_pin);
    }

    for (u8 i = 0; i < IO_KEY_ROW_TABLE_SIZE; i++) {
        GPIO_SET_DIR_IN(key_io_table_row[i].gpiox, key_io_table_row[i].gpio_pin);
        row |= (gpio_read_bit(key_io_table_row[i].gpiox, key_io_table_row[i].gpio_pin))? 0x00: (0x01 << i);
        GPIO_SET_DIR_OUT(key_io_table_row[i].gpiox, key_io_table_row[i].gpio_pin);
        gpio_reset_bits(key_io_table_row[i].gpiox, key_io_table_row[i].gpio_pin);
    }

    /* no combination key case */
    switch (column) {
        case 0x01:
            switch (row) {
                case 0x01:
                    return KEY_ID_K1;

                case 0x02:
                    return KEY_ID_K2;

                case 0x04:
                    break;

                case 0x08:
                    break;

                case 0x10:
                    break;

                case 0x20:
                    break;

                default:
                    break;
            }
            break;

        case 0x02:
            switch (row) {
                case 0x01:
                    return KEY_ID_K3;

                case 0x02:
                    return KEY_ID_K4;

                case 0x04:
                    break;

                case 0x08:
                    break;

                case 0x10:
                    break;

                case 0x20:
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
#else // !IO_KEY_SCAN_MODE
    /* Assume that the IO definition order in key_io_table_column corresponds to the desired key_id order */
    /* Or adjust the list based on actual requirements, for example, define a list corresponding to key_io_table_column */
    u8 key_id = KEY_ID_K1;
    for (u8 i = 0; i < IO_KEY_COL_TABLE_SIZE; i++) {
        if (gpio_read_bit(key_io_table_column[i].gpiox, key_io_table_column[i].gpio_pin) == RESET) {
            return key_id;
        }
        key_id++;
    }
#endif // IO_KEY_SCAN_MODE

    return KEY_ID_NO;
}


AT(.com_text.gpio.isr)
void gpio_isr_callback(void)
{
    GPIO_EDGE_PENDING_CLR();

    lowpwr_sleep_delay_reset();
}

#if IO_KEY_SCAN_MODE
void bsp_gpio_scan_mode_de_save(void)
{
    uint32_t iokeyscan_gpioade = 0;
    uint32_t iokeyscan_gpiobde = 0;

    for (u8 i = 0; i < IO_KEY_ROW_TABLE_SIZE; i++) {
        if (GPIOA_REG == key_io_table_row[i].gpiox) {
            iokeyscan_gpioade |= key_io_table_row[i].gpio_pin;
        }
        if (GPIOB_REG == key_io_table_row[i].gpiox) {
            iokeyscan_gpiobde |= key_io_table_row[i].gpio_pin;
        }
    }
    //Retain the wakeup port configuration
    GPIOADE |= iokeyscan_gpioade & 0xffff;
    GPIOBDE |= iokeyscan_gpiobde & 0x3ff;
}
#endif

void bsp_gpio_wakeup_config(void)
{
    lowpwr_wakeup_typedef config;

    gpio_edge_capture_disable();

    config.source = WK_LP_GPIO ;
    config.gpio_cfg.edge = GPIO_EDGE_FALLING;
    config.gpio_cfg.filter = GPIO_EDGE_FIL_2;
    config.gpio_cfg.wkpinmap = GPIO_WK6MAP;
    config.gpio_cfg.wkpinmap_int = GPIO_WK6_G0_FALL;

    for (u8 i = 0; i < IO_KEY_COL_TABLE_SIZE; i++) {
        config.gpio_cfg.gpiox = key_io_table_column[i].gpiox;
        config.gpio_cfg.gpio_pin = key_io_table_column[i].gpio_pin;
        config.gpio_cfg.gpio_pupd = GPIO_PUPD_PU10K;
        lowpwr_gpio_wakeup_config(&config);
    }

    #if IO_KEY_WK0_EN
    config.source |= WK_LP_WK0;
    lowpwr_wk0_wakeup_config(&config);
    #endif

    #if IO_KEY_SCAN_MODE
    gpio_init_typedef gpio_init_structure;

    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_GPIO;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PD300;

    for (u8 i = 0; i < IO_KEY_ROW_TABLE_SIZE; i++) {
        gpio_init_structure.gpio_pin = key_io_table_row[i].gpio_pin;
        gpio_init(key_io_table_row[i].gpiox, &gpio_init_structure);
        gpio_reset_bits(key_io_table_row[i].gpiox, gpio_init_structure.gpio_pin);
    }

    if (sys_cb.sleep_sta == LOWPWR_SLEEP_PREPARE) {     //io scan模式进sleep流程才开中断，避免平时扫描过程一直进中断
        gpio_edge_pic_config(gpio_isr_callback, 0, ENABLE);
    }
    #else
        gpio_edge_pic_config(gpio_isr_callback, 0, ENABLE);
    #endif // IO_KEY_SCAN_MODE
}

#endif // BSP_IOKEY_EN

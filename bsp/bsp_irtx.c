#include "driver_ir.h"
#include "driver_gpio.h"
#include "bsp_ir.h"


extern void irrx_io_init(void);

// User should care about that the allocated address should be in ram
uint32_t irtx_capture_buf[0x90/4];
uint16_t irtx_learn_buf[0x90/2];
uint8_t irtx_learn_buf_idx = 0;

uint16_t ir_learn_rx_buf[128];
uint16_t ir_learn_tx_buf[128];

static uint8_t irtx_cap_done = 0;

void irtx_io_init(void)
{
    gpio_init_typedef gpio_init_structure;

    gpio_init_structure.gpio_pin = IRTX_IO_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_MAP;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_drv = GPIO_DRV_24MA;

    gpio_init(IRTX_IO_PORT, &gpio_init_structure);
    gpio_func_mapping_config(IRTX_IO_PORT, IRTX_IO_PIN, GPIO_CROSSBAR_OUT_IRTX);
}

AT(.com_text.irq)
void irtx_cap_done_set(void)
{
    irtx_cap_done = 1;
}

void irtx_cap_done_clr(void)
{
    irtx_cap_done = 0;
}

uint8_t irtx_cap_done_get(void)
{
    return irtx_cap_done;
}

AT(.com_text.irq)
void irtx_irq(void)
{
    if (irtx_get_flag(IRTX_REG, IRTX_FLAG_TX_PND)) {
        irtx_clear_flag(IRTX_REG, IRTX_FLAG_TX_PND);
    }

    if (irtx_get_flag(IRTX_REG, IRTX_FLAG_ODMA_ALL_PND)) {
        irtx_clear_flag(IRTX_REG, IRTX_FLAG_ODMA_ALL_PND);
        irtx_cap_done_set();
    }

    if (irtx_get_flag(IRTX_REG, IRTX_FLAG_ODMA_HALF_PND)) {
        irtx_clear_flag(IRTX_REG, IRTX_FLAG_ODMA_HALF_PND);
    }

    if (irtx_get_flag(IRTX_REG, IRTX_FLAG_IDMA_ALL_PND)) {
        irtx_clear_flag(IRTX_REG, IRTX_FLAG_IDMA_ALL_PND);
    }

    if (irtx_get_flag(IRTX_REG, IRTX_FLAG_IDMA_HALF_PND)) {
        irtx_clear_flag(IRTX_REG, IRTX_FLAG_IDMA_HALF_PND);
    }
}

void irtx_example_hw_enc_init(uint8_t irtx_example_mode)
{
    irtx_init_typedef irtx_param;
    memset(&irtx_param, 0, sizeof(irtx_param));

    clk_gate1_cmd(CLK_GATE1_IRTX, CLK_EN);
    clk_irtx_clk_set(CLK_IR_CLK_X24M_CLKDIV24);

    irtx_io_init();

    if (irtx_example_mode == IRTX_EXAMPLE_NEC) {
        irtx_param.encode_format = IRTX_ENCODE_FORMAT_NEC;
        irtx_param.carrier.carrier_en = ENABLE;//DISABLE;
        irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_3M;
        irtx_param.carrier.ircw_duty = (79/3)-1;    //1/3
        irtx_param.carrier.ircw_length = 79-1;      //3M/79=38KHz
    } else if (irtx_example_mode == IRTX_EXAMPLE_TG9012) {
        irtx_param.encode_format = IRTX_ENCODE_FORMAT_TC9012;
        irtx_param.carrier.carrier_en = DISABLE;
        irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_3M;
        irtx_param.carrier.ircw_duty = (79/3)-1;    //1/3
        irtx_param.carrier.ircw_length = 79-1;      //3M/79=38KHz
    } else if (irtx_example_mode == IRTX_EXAMPLE_RC5) {
        irtx_param.encode_format = IRTX_ENCODE_FORMAT_RC5;
        irtx_param.carrier.carrier_en = DISABLE;
        irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_4M;
        irtx_param.carrier.ircw_duty = (111/3)-1;    //1/3
        irtx_param.carrier.ircw_length = 111-1;      //4M/111=36KHz
    } else if (irtx_example_mode == IRTX_EXAMPLE_RC6) {
        irtx_param.encode_format = IRTX_ENCODE_FORMAT_RC6;
        irtx_param.carrier.carrier_en = DISABLE;
        irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_4M;
        irtx_param.carrier.ircw_duty = (111/3)-1;    //1/3
        irtx_param.carrier.ircw_length = 111-1;      //4M/111=36KHz
    }

    irtx_hw_enc_init(IRTX_REG, &irtx_param);

    irtx_cmd(IRTX_REG, ENABLE);
}

void irtx_example_capture_learn_init(uint8_t irtx_example_mode)
{
    irtx_init_typedef irtx_param;
    ir_capture_config_typedef ir_capture;
    ir_filter_config_typedef ir_filter;

    memset(&irtx_param, 0, sizeof(irtx_param));

    clk_irtx_clk_set(CLK_IR_CLK_X24M_CLKDIV24);
    clk_gate1_cmd(CLK_GATE1_IRTX, CLK_EN);
    clk_gate2_cmd(CLK_GATE2_IRFLT, CLK_EN);

    //tx
    irtx_io_init();

    //rx for capture
    irrx_io_init();

    irtx_param.encode_format = IRTX_ENCODE_FORMAT_CUSTOM;
    //carrier
    irtx_param.carrier.carrier_en = ENABLE;
    irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_3M;
    irtx_param.carrier.ircw_duty = (79/3)-1;    //1/3
    irtx_param.carrier.ircw_length = 79-1;      //3M/79=38KHz

    //capture  边沿触发后，捕获到 cap_dma_size 起pending
    ir_capture.capture_en = ENABLE;
    ir_capture.capture_edge = IRTX_CAP_EDGE_FALLING;
    ir_capture.cap_dma_en = ENABLE;
    ir_capture.cap_dma_size =  sizeof(ir_learn_rx_buf)/sizeof(ir_learn_rx_buf[0]);
    ir_capture.cap_dma_addr = (uint32_t)ir_learn_rx_buf;
    ir_capture_init(IRTX_REG, &ir_capture);

    //filter for capture
    ir_filter.filter_en = ENABLE;
    ir_filter.filter_src = IRTX_FLT_SRC_RXIN;
    ir_filter.filter_len = 3;
    irrx_filter_init(&ir_filter);

    irtx_pic_config(IRTX_REG, irtx_irq, 0, IRTX_FLAG_ODMA_ALL_PND, ENABLE);

    irtx_learn_init(IRTX_REG, &irtx_param);

    irtx_learn_cmd(IRTX_REG, ENABLE);
}

void irtx_example_init(uint8_t irtx_example_mode)
{
    if (irtx_example_mode <= IRTX_EXAMPLE_RC6) {
        irtx_example_hw_enc_init(irtx_example_mode);
    } else if (irtx_example_mode == IRTX_EXAMPLE_CAPTURE_LEARN){
        irtx_example_capture_learn_init(irtx_example_mode);
    } else if(irtx_example_mode == IRTX_EXAMPLE_D7C6) {
        irtx_extension_d7c6_example_init();
    }
}

uint8_t irtx_reverse_bits(uint8_t data, uint8_t bits)
{
    uint8_t result = 0;
    for (uint8_t i = 0; i < bits; i++) {
        result |= (data & 1) << (bits - 1 - i);
        data >>= 1;
    }
    return result;
}

uint32_t irtx_nec_or_tc9012_data_fill(uint8_t addr, uint8_t cmd)
{
    uint32_t result = 0;
    result |= addr << 0;
    result |= ((~addr)&0xff) << 8;
    result |= cmd << 16;
    result |= ((~cmd)&0xff) << 24;
    return result;
}

uint32_t irtx_rc5_data_fill(bool toggle, uint8_t addr, uint8_t cmd)
{
    uint32_t result = 0;
    result |= (toggle << 0);
    result |= (irtx_reverse_bits(addr, 5) << 1);
    result |= (irtx_reverse_bits(cmd, 6) << 6);
    return result;
}

uint32_t irtx_rc6_data_fill(bool toggle, uint8_t addr, uint8_t cmd)
{
    uint32_t result = 0;
    result |= (irtx_reverse_bits(addr, 8) << 0);
    result |= (irtx_reverse_bits(cmd, 8) << 8);
    return result;
}

uint16_t ir_capture_data_len_get(uint16_t *data_buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++) {
        //低14位是电平持续时长，持续时间为最大值则判断无信号
		if (0x7fff == data_buf[i] && 0x7fff == data_buf[i+1] && 0x7fff == data_buf[i+2] && 0x7fff == data_buf[i+3]) {
			return i;
		}
	}

	return len;
}

void irtx_send_example(uint8_t irtx_example_mode)
{
    static bool toggle = 0;
    uint8_t addr = 0;
    uint8_t cmd = 0;
    static uint8_t cnt = 0;
    static uint32_t irtx_send_tick = 0;
    uint32_t irtx_code = 0;

    if (tick_check_expire(irtx_send_tick, 108)) {
        irtx_send_tick = tick_get();

        toggle ^= 1; //rc5, 单击要手动翻转TR bit, 长按硬件自动保持
        addr = 0x12;
        cmd  = 0x34;
        if (irtx_example_mode == IRTX_EXAMPLE_NEC || irtx_example_mode == IRTX_EXAMPLE_TG9012) {
            irtx_code = irtx_nec_or_tc9012_data_fill(addr, cmd);
        } else if (irtx_example_mode == IRTX_EXAMPLE_RC5) {
            irtx_code = irtx_rc5_data_fill(toggle, addr, cmd);
        } else if (irtx_example_mode == IRTX_EXAMPLE_RC6) {
            irtx_code = irtx_rc6_data_fill(toggle, addr, cmd);
        } else {
            printf("irtx_send_example *****err!****\n");
            return;
        }

#if 0
        //长按
        if (cnt == 0) {
            irtx_set_repeat(IRTX_REG, ENABLE);   //kick一次后自动reapet
#else
        //单击
        if (cnt < 30) {
            irtx_set_repeat(IRTX_REG, DISABLE);
#endif
            irtx_set_data(IRTX_REG, irtx_code);
            irtx_hw_enc_kick(IRTX_REG, ENABLE);
            irtx_kick_wait(IRTX_REG);
        } else if (cnt == 30) {
            irtx_set_repeat(IRTX_REG, DISABLE);  //停止repeat
        }

        if (cnt < 30) {
            printf("irtx_code:%x,cnt:%d\n", irtx_code,cnt);
            cnt++;
        }
    }
}

void irtx_learn_example(void)
{
    if (irtx_cap_done_get()) {
        irtx_cap_done_clr();

        uint16_t capture_len = ir_capture_data_len_get(ir_learn_rx_buf, sizeof(ir_learn_rx_buf)/sizeof(ir_learn_tx_buf[0]));

        memcpy(ir_learn_tx_buf, ir_learn_rx_buf, capture_len * 2);

        irtx_learn_kick(IRTX_REG, ir_learn_tx_buf, capture_len, ENABLE);
        irtx_kick_wait(IRTX_REG);
        printf("learn data:\n");
        print_r(ir_learn_tx_buf, capture_len * 2);
    }
}

void irtx_extension_d7c6_example_init(void)
{
    irtx_init_typedef irtx_param;

    printf("%s\n", __func__);

    clk_gate1_cmd(CLK_GATE1_IRTX, CLK_EN);
    clk_irtx_clk_set(CLK_IR_CLK_X24M_CLKDIV24);

    irtx_io_init();

    // Carrier Config
    irtx_param.carrier.carrier_en = DISABLE;
    irtx_param.carrier.mod_clksel = IRTX_MOD_CLKSEL_4M;
    irtx_param.carrier.ircw_duty = 99 / 3;
    irtx_param.carrier.ircw_length = 99;

    irtx_learn_init(IRTX_REG, &irtx_param);

    irtx_pic_config(IRTX_REG, irtx_irq, 0, IRTX_FLAG_IDMA_ALL_PND, ENABLE);

    irtx_learn_cmd(IRTX_REG, ENABLE);
}

void irtx_d7c6_timing_set(uint8_t component_data, uint8_t bit_width)
{
    for (uint8_t bit = 0; bit < bit_width; bit++) {
        uint16_t tick_cnt;
        uint8_t data = component_data >> bit;
        if (data & 0x01) {
            tick_cnt = (TICK_CNT) & ~BIT(15);         //1T low level

            memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &tick_cnt, 2);

            tick_cnt = TICK_CNT | BIT(15);            //1T high level

            memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &tick_cnt, 2);

            tick_cnt = TICK_CNT | BIT(15);            //1T high level

            memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &tick_cnt, 2);
        } else {
            tick_cnt = (TICK_CNT) & ~BIT(15);         //1T low level

            memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &tick_cnt, 2);

            tick_cnt = TICK_CNT | BIT(15);            //1T high level

            memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &tick_cnt, 2);
        }
    }
}

void irtx_component(uint8_t type, uint8_t *pcomponent_data)
{
    uint8_t component_data = *pcomponent_data;

    if (D7C6_PREAMBLE == type) {
        uint16_t preamble = (4 * TICK_CNT) | BIT(15);     //4T high level preamble transfer to CLK tick counter.
        memcpy(&irtx_learn_buf[irtx_learn_buf_idx++], &preamble, 2);
    } else if (D7C6_DATA == type) {
        irtx_d7c6_timing_set(component_data, 7);
    } else if (D7C6_CUSTOM == type) {
        irtx_d7c6_timing_set(component_data, 6);
    }
}

void irtx_d7c6_send(uint8_t data, uint8_t custom)
{
    irtx_learn_buf_idx = 0;
    memset(irtx_learn_buf, 0x00, sizeof(irtx_learn_buf));

    irtx_component(D7C6_PREAMBLE, NULL);
    irtx_component(D7C6_DATA, &data);
    irtx_component(D7C6_CUSTOM, &custom);

    irtx_learn_kick(IRTX_REG, irtx_learn_buf, irtx_learn_buf_idx, ENABLE);
}

void irtx_extension_d7c6_handler_example(void)
{
    static uint32_t irtx_learn_tick = 0;

    if (tick_check_expire(irtx_learn_tick, 2000)) {
        irtx_learn_tick = tick_get();
        irtx_d7c6_send(0x3, 0x5);

        //print_r(irtx_learn_buf, sizeof(irtx_learn_buf));
    }
}

#include "include.h"
#include "driver_pwm.h"
#include "driver_gpio.h"
#include "bsp_pwm.h"

#define PWM_DEAD_ZONE_TEST              0
#define PWM_TMR_IE_TEST                 0

#define TIMERP_PWM0_PORT                GPIOA_REG
#define TIMERP_PWM0_PIN                 GPIO_PIN_0

#define TIMERP_PWM1_PORT                GPIOA_REG
#define TIMERP_PWM1_PIN                 GPIO_PIN_1

#if PWM_TMR_IE_TEST
AT(.com_text.isr)
void pwm_tmr_isq(void)
{
    if (pwm_tmr_get_flag(PWM_TMR0_REG) != RESET) {
        pwm_tmr_clear_flag(PWM_TMR0_REG);
    }
}
#endif

void pwm_init(void)
{
    gpio_init_typedef gpio_init_structure;
    pwm_tmr_base_init_typedef tmrp_base_init_struct;
    pwm_base_init_typedef pwm_base_init_cb;
#if PWM_DEAD_ZONE_TEST
    pwm_dz_base_init_typedef pwm_dz_base_init_cb;
#endif

    clk_gate2_cmd(CLK_GATE2_PWM, CLK_EN);

    /*****************************初始化配置**********************************/
    gpio_init_structure.gpio_pin = TIMERP_PWM0_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_MAP;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(TIMERP_PWM0_PORT, &gpio_init_structure);

    gpio_init_structure.gpio_pin = TIMERP_PWM1_PIN;
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_MAP;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(TIMERP_PWM1_PORT, &gpio_init_structure);

    gpio_func_mapping_config(TIMERP_PWM0_PORT, TIMERP_PWM0_PIN, GPIO_CROSSBAR_OUT_PWM0);
    gpio_func_mapping_config(TIMERP_PWM1_PORT, TIMERP_PWM1_PIN, GPIO_CROSSBAR_OUT_PWM1);

    pwm_base_init_cb.ch = PWM_CH0;
    pwm_base_init_cb.output_inv = DISABLE;
    pwm_base_init_cb.tmr_sel = PWM_SEL_TMR0;
    pwm_base_init(PWM_REG, &pwm_base_init_cb);

    pwm_base_init_cb.ch = PWM_CH1;
    pwm_base_init_cb.output_inv = ENABLE;
    pwm_base_init_cb.tmr_sel = PWM_SEL_TMR1;
    pwm_base_init(PWM_REG, &pwm_base_init_cb);

    tmrp_base_init_struct.clock_source = TMRP_XOSC24M;
    tmrp_base_init_struct.clock_div = TMRP_CLOCK_DIV_8;             //3M
    tmrp_base_init_struct.period = 9000 - 1;                        //3ms
    tmrp_base_init_struct.cnt = 0;
    tmrp_base_init_struct.duty = 6000 - 1;                          //2/3
    tmrp_base_init_struct.tmr_mode = TMRP_TMR_PWM_MODE;

    pwm_tmr_base_init(PWM_TMR0_REG, &tmrp_base_init_struct);

    tmrp_base_init_struct.clock_source = TMRP_XOSC24M;
    tmrp_base_init_struct.clock_div = TMRP_CLOCK_DIV_8;             //3M
    tmrp_base_init_struct.period = 6000 - 1;                        //2ms
    tmrp_base_init_struct.cnt = 0;
    tmrp_base_init_struct.duty = 3000 - 1;                          //1/2
    tmrp_base_init_struct.tmr_mode = TMRP_TMR_PWM_MODE;

    pwm_tmr_base_init(PWM_TMR1_REG, &tmrp_base_init_struct);

#if PWM_TMR_IE_TEST
    pwm_tmr_ie_cmd(PWM_TMR0_REG, ENABLE);
    pwm_tmr_pic_config(pwm_tmr_isq, 0, ENABLE);
#endif

    pwm_cmd(PWM_REG, PWM_CH0, ENABLE);
    pwm_cmd(PWM_REG, PWM_CH1, ENABLE);

    pwm_tmr_cmd(PWM_TMR0_REG, ENABLE);
    pwm_tmr_cmd(PWM_TMR1_REG, ENABLE);
#if PWM_DEAD_ZONE_TEST
    pwm_dz_base_init_cb.mode = PWM_DZMODE_0;
    pwm_dz_base_init_cb.len = 3000;
    pwm_dead_zone_init(PWM_DZ0_REG, &pwm_dz_base_init_cb);
    pwm_dead_zone_cmd(PWM_DZ0_REG, ENABLE);
#endif
}

void pwm_example(void)
{
    //测试，15s后关闭PWM
    static bool test_flag = 1;
    static uint32_t tick = -1;
    if (tick_check_expire(tick, 15000) && test_flag) {
        test_flag = 0;
        pwm_cmd(PWM_REG, PWM_CH0, DISABLE);
        pwm_cmd(PWM_REG, PWM_CH1, DISABLE);
        printf("*****close pwm*****\n");
    }
}

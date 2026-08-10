#include "include.h"
#include "bsp_led.h"
#include  "driver_gpio.h"

#if BSP_LED_EN

led_cb_t led_cb;

AT(.rodata.led.cfg)
const led_cfg_t led_cfg_poweron     = {0x00, 0xff, 1, 0};   //蓝灯常亮
AT(.rodata.led.cfg)
const led_cfg_t led_cfg_poweroff    = {0x00, 0x00, 0, 255}; //蓝灯灭
AT(.rodata.led.cfg)
const led_cfg_t led_cfg_pairing     = {0x00, 0x0f, 2, 2};   //蓝灯亮300ms灭450ms
AT(.rodata.led.cfg)
const led_cfg_t led_cfg_connected   = {0x00, 0xff, 1, 0};  //蓝灯常亮


static void led_cfg_init(uint32_t gpio_pin, gpio_typedef *gpio_port, u8 led2_flag)
{
    gpio_init_typedef gpio_init_structure_led;
    gpio_init_structure_led.gpio_pin = gpio_pin;
    gpio_init_structure_led.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure_led.gpio_fen = GPIO_FEN_GPIO;
    if (led2_flag) {
        gpio_init_structure_led.gpio_mode = GPIO_MODE_ANALOG;
    } else {
        gpio_init_structure_led.gpio_mode = GPIO_MODE_DIGITAL;
    }
    gpio_init_structure_led.gpio_drv = GPIO_DRV_6MA;
    gpio_init_structure_led.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init(gpio_port, &gpio_init_structure_led);
}

AT(.text.led_disp)
void led_init(void)
{
    memset(&led_cb, 0, sizeof(led_cb));

    if (xcfg_cb.bled_disp_en && xcfg_cb.rled_disp_en) {
        if (xcfg_cb.bled_io_sel == xcfg_cb.rled_io_sel) {
            led_cb.led2_flag = 1;
        }
    }

    if (xcfg_cb.bled_disp_en) {
        led_cfg_init(BSP_BLED_PIN, BSP_BLED_PORT, led_cb.led2_flag);
    }

    if (xcfg_cb.rled_disp_en && !led_cb.led2_flag) {
        led_cfg_init(BSP_RLED_PIN, BSP_RLED_PORT, led_cb.led2_flag);
    }
}

AT(.com_text.led_disp)
void led2_port_out_mode(uint8_t mode)                    //一个IO推两个LED设置数字IO使能状态
{
    gpio_set_de(BSP_BLED_PORT , BSP_BLED_PIN, mode);
}

AT(.com_text.led_disp)
void bled_set_on(void)
{
    if (!xcfg_cb.bled_disp_en) {
        return;
    }

    if (led_cb.led2_flag) {
        led2_port_out_mode(GPIO_MODE_DIGITAL);               //设为数字IO，拉低则点亮蓝灯
        gpio_reset_bits(BSP_BLED_PORT , BSP_BLED_PIN);
    } else {
        gpio_set_bits(BSP_BLED_PORT ,BSP_BLED_PIN);
    }
}

AT(.com_text.led_disp)
void bled_set_off(void)
{
    if (!xcfg_cb.bled_disp_en) {
        return;
    }

    if (led_cb.led2_flag) {
        led2_port_out_mode(GPIO_MODE_ANALOG);                //设为模拟IO，熄灭蓝灯/红灯
    } else {
        gpio_reset_bits(BSP_BLED_PORT , BSP_BLED_PIN);
    }
}

AT(.com_text.led_disp)
static void rled_set_on(void)
{
    if (led_cb.led2_flag) {
        led2_port_out_mode(GPIO_MODE_DIGITAL);              //设为数字IO，拉高则点亮红灯
        gpio_set_bits(BSP_RLED_PORT, BSP_RLED_PIN);
    } else {
        gpio_set_bits(BSP_RLED_PORT , BSP_RLED_PIN);
    }
}

AT(.com_text.led_disp)
static void rled_set_off(void)
{
    if (led_cb.led2_flag) {
        led2_port_out_mode(GPIO_MODE_ANALOG);               //设为模拟IO，熄灭蓝灯/红灯
    } else {
        gpio_reset_bits(BSP_RLED_PORT , BSP_RLED_PIN);
    }
}

AT(.com_text.led_disp)
void led_charge_on(void)
{
    led_cb.charge_flag = true;
    if(!led_cb.led2_flag) {
        bled_set_off();
    }
    rled_set_on();
}

AT(.com_text.led_disp)
void led_charge_off(void)
{
    rled_set_off();
    led_cb.charge_flag = false;
}

AT(.text.led_disp)
void led_power_up(void)
{
    //是否配置开机状态LED?
    if (xcfg_cb.led_pwron_config_en) {
        led_set_sta((led_cfg_t *)&xcfg_cb.led_poweron);
    } else {
        led_set_sta(&led_cfg_poweron);
    }
}

AT(.text.led_disp)
void led_power_down(void)
{
    //是否配置关机状态LED?
    if (xcfg_cb.led_pwroff_config_en) {
        led_set_sta((led_cfg_t *)&xcfg_cb.led_poweroff);
    } else {
        led_set_sta(&led_cfg_poweroff);
    }
}

AT(.text.led_disp)
void led_bt_idle(void)
{
    //是否配置蓝牙配对状态LED?
    if (xcfg_cb.led_btpair_config_en) {
        led_set_sta((led_cfg_t *)&xcfg_cb.led_pairing);
    } else {
        led_set_sta(&led_cfg_pairing);
    }
}

AT(.text.led_disp)
void led_w4_end(void)
{
    u32 tout = 20;
    while(led_cb.busy && --tout != 0) {
        delay_5ms(10);
    }
}

AT(.text.led_disp)
void led_bt_connected(void)
{
    //是否配置蓝牙已连接状态LED?
    if (xcfg_cb.led_btconn_config_en) {
        led_set_sta((led_cfg_t *)&xcfg_cb.led_connected);
    } else {
        led_set_sta(&led_cfg_connected);
    }
}

AT(.text.led_disp)
void led_set_sta_normal(const led_cfg_t *cfg)
{
    led_set_sta_p(cfg, &led_cb);
}

ALIGNED(256)
AT(.text.led_aligned)
void led_set_sta_p(const led_cfg_t *cfg, led_cb_t *s)
{
    led_cfg_t sta;
    memcpy(&sta, cfg, sizeof(sta));

    GLOBAL_INT_DISABLE();
    u32 tick = tick_get();

    s->bled_sta = sta.bluepat;
    s->unit     = sta.unit;
    s->period   = sta.cycle;

    if (s->period == 0xff) {
        s->circle = 0x7fff;
        s->busy   = true;
    } else {
        s->circle = s->unit * 8 + s->period;
    }

    s->init = (bool)(tick & BIT(31));
    s->offset = (tick/50 + 1) % s->circle;
    GLOBAL_INT_RESTORE();
}

AT(.com_text.led_disp)
void led_scan(u32 tick)
{
    u16 offset;

    if (!xcfg_cb.bled_disp_en) {
        return;
    }

    led_cb_t *s;
    s = &led_cb;

    if(s->unit == 0 || s->circle == 0) {
        return;
    }

    if (led_cb.charge_flag) {
        return;
    }

    bool init = (bool)(tick & BIT(31));
    if(init != s->init) {           //wrap around
        u16 old_offset = s->offset;
        old_offset += (0xffffffff / 50) % 50;
        s->offset = old_offset % s->circle;
        s->init = init;
    }

    tick = tick/50;
    offset = (tick + s->circle - s->offset) % s->circle;
    if(offset < (s->unit * 8)) {
        if(offset % s->unit == 0) {
            u8 bcnt = offset/s->unit;
            if (s->bled_sta & BIT(bcnt)) {
                bled_set_on();
            } else {
                bled_set_off();
            }
        }
    } else {
        s->busy = false;
        //只亮一次
        if(s->circle == 0x7fff) {
            s->unit = 0;
        }
    }
}

#if BSP_DLED_EN

static dled_cfg_t dled_cfg;

void dled_init(void)
{
    memset(&dled_cfg, 0, sizeof(dled_cfg_t));
    dled_onoff_sta_set(0, 0);
}

void dled_onoff_sta_set(u32 bled_onoff_set, u32 rled_onoff_set)
{
    dled_cfg.bled_onoff_sta = (bled_onoff_set & 0xFFFFF);
    dled_cfg.rled_onoff_sta = (rled_onoff_set & 0xFFFFF);
}

void dled_mode_set(u32 dled_mode)
{
    dled_onoff_sta_set(0, 0);
    switch (dled_mode) {
        case DLED_MODE_0:
            //灯1常亮，灯2常亮
            dled_onoff_sta_set(0xFFFFF, 0xFFFFF);
            break;

        case DLED_MODE_1:
            //灯1熄灭，灯2熄灭
            dled_onoff_sta_set(0, 0);
            break;

        case DLED_MODE_2:
             //灯1常亮，灯2熄灭
            dled_onoff_sta_set(0xFFFFF, 0);
            break;

        case DLED_MODE_3:
            //灯1熄灭，灯2常亮
            dled_onoff_sta_set(0, 0xFFFFF);
            break;

        case DLED_MODE_4:
            //灯1闪烁，灯2常亮
            dled_onoff_sta_set(0x3FF, 0xFFFFF);
            break;

        case DLED_MODE_5:
            //灯1常亮，灯2闪烁
            dled_onoff_sta_set(0xFFFFF, 0x3FF);
            break;

        case DLED_MODE_6:
            //同步闪烁
            dled_onoff_sta_set(0x3FF, 0x3FF);
            break;

        case DLED_MODE_7:
            //交替闪烁
            dled_onoff_sta_set(0xFFC00, 0x3FF);
            break;

        default:
        break;
    }
}

AT(.com_text.dled)
void dled_scan_10ms(void)
{
    dled_cfg.cnt_10ms++;
    if (dled_cfg.cnt_10ms >= 40) {
        dled_cfg.cnt_10ms = 0;

    }
    if (dled_cfg.cnt_10ms % 2) { //奇数
        if (dled_cfg.bled_onoff_sta &BIT(dled_cfg.cnt_10ms/2)) {
            bled_set_on();
        } else {
            bled_set_off();
        }
    } else {                    //偶数
        if (dled_cfg.rled_onoff_sta  &BIT(dled_cfg.cnt_10ms/2)) {
            rled_set_on();
        } else {
            rled_set_off();
        }
    }
}
#endif

#endif

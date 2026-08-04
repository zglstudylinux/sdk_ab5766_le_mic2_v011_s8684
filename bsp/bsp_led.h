#ifndef _BSP_LED_H
#define _BSP_LED_H

#if BSP_LED_EN

#define BSP_BLED_PORT        GPIO_PORT_GET(xcfg_cb.bled_io_sel)
#define BSP_BLED_PIN         GPIO_PIN_GET(xcfg_cb.bled_io_sel)
#define BSP_RLED_PORT        GPIO_PORT_GET(xcfg_cb.rled_io_sel)
#define BSP_RLED_PIN         GPIO_PIN_GET(xcfg_cb.rled_io_sel)


typedef struct {
    u8 redpat;
    u8 bluepat;
    u8 unit;
    u8 cycle;
} led_cfg_t;

typedef struct {
    u8 rled_sta;    //红灯 亮-灭控制
    u8 bled_sta;    //蓝灯 亮-灭控制
    u8 unit;        //50ms单位 (范围：1~255)
    u8 period;      //间隔周期 (范围：0~255)
    struct {
        u16 offset  : 15;
        u16 init    : 1;
    };
    u16 circle;

    volatile bool busy;
    bool lowbat;
    bool charge_flag;
    u8 led2_flag;
} led_cb_t;

#define led_set_sta(cfg)       led_set_sta_normal(cfg)
void led_init(void);
void bled_set_off(void);
void led_scan(u32 tick);
void led_power_up(void);
void led_power_down(void);
void led_bt_idle(void);
void led_w4_end(void);
void led_bt_connected(void);
void led_set_sta_normal(const led_cfg_t *cfg);
void led_set_sta_p(const led_cfg_t *cfg, led_cb_t *s);
void led2_port_out_mode(uint8_t mode);
void led_charge_on(void);
void led_charge_off(void);

#if BSP_DLED_EN
typedef enum {
    DLED_MODE_0       = 0x00,
    DLED_MODE_1       = 0x01,
    DLED_MODE_2       = 0x02,
    DLED_MODE_3       = 0x03,
    DLED_MODE_4       = 0x04,
    DLED_MODE_5       = 0x05,
    DLED_MODE_6       = 0x06,
    DLED_MODE_7       = 0x07,
} DLED_MODE_TYPEDEF;

typedef struct {
    volatile u32 cnt_10ms;
    volatile u32 bled_onoff_sta;
    volatile u32 rled_onoff_sta;
} dled_cfg_t;

void dled_init(void);
void dled_onoff_sta_set(u32 bled_onoff_set, u32 rled_onoff_set);
void dled_mode_set(u32 dled_mode);
void dled_scan_10ms(void);
#endif

#endif // BSP_LED_EN

#endif // _BSP_LED_H

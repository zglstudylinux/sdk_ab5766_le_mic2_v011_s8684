//该头文件由软件自动生成，请勿随意修改！
#ifndef _XCFG_H
#define _XCFG_H

#define XCFG_EN             1

typedef struct __attribute__((packed)) _xcfg_cb_t {
    u32 wireless_adapter_en              : 1;   //无线mic适配器
    u32 wireless_mic_emit_en             : 1;   //无线mic发射端
    u16 sys_off_time;                           //无连接时自动关机时间: 不关机: 0, 30秒钟后: 30, 1分钟后: 60, 2分钟后: 120, 3分钟后: 180, 4分钟后: 240, 5分钟后: 300, 6分钟后: 360, 7分钟后: 420, 8分钟后: 480, 9分钟后: 540, 10分钟后: 600, 15分钟后: 900, 20分钟后: 1200, 25分钟后: 1500, 30分钟后: 1800, 45分钟后: 2700, 1小时后: 3600
    u32 poweron_first_bat_en             : 1;   //发射端第一次上电开机
    u8 lpwr_warning_vbat_level;                 //低电提示电压: 0:不提示, 1:2.8V, 2:2.9V, 3:3.0V, 4:3.1V, 5:3.2V, 6:3.3V, 7:3.4V, 8:3.5V, 9:3.6V, 10:3.7V
    u8 lpwr_off_vbat_level;                     //低电关机电压: 0:不关机, 1:2.8V, 2:2.9V, 3:3.0V, 4:3.1V, 5:3.2V, 6:3.3V, 7:3.4V, 8:3.5V, 9:3.6V, 10:3.7V
    u32 buck_mode_en                     : 1;   //BUCK MODE
    u32 vddbt_capless_en                 : 1;   //VDDBT省电容
    u32 vddio_sel                        : 4;   //VDDIO电压: 2.7V: 8, 2.85V: 9, 3.0V: 10, 3.15V: 11, 3.3V: 12, 3.45V: 13, 3.6V: 14
    u32 vddbt_sel                        : 4;   //VDDBT电压: 1.1V: 5, 1.2V: 7, 1.3V: 9, 1.4V: 11, 1.5V: 13
    u32 vddcore_sel                      : 5;   //VDDCORE电压: 1.0V: 12, 1.1V: 16, 1.2V: 20, 1.3V: 24, 1.4V: 28
    u8 uosc_both_cap;                           //OSC基础电容: 0:0PF, 1:6PF
    u8 uosci_cap;                               //自定义OSCI电容
    u8 uosco_cap;                               //自定义OSCO电容
    u32 ft_osc_cap_en                    : 1;   //优先使用产测电容值
    u32 huart_en                         : 1;   //HUART调试(EQ/FCC)
    u32 huart_io_sel                     : 8;   //HUART串口选择: PA0: 160, PA1: 161, PA2: 162, PA3: 163, PA4: 164, PA5: 165, PA6: 166, PA7: 167, PA8: 168, PA9: 169, PA10: 170, PA11: 171, PA12: 172, PA13: 173, PA14: 174, PA15: 175, PB0: 176, PB1: 177, PB2: 178, PB3: 179, PB4: 180, PB5: 181, PB6: 182, PB7: 183, PB8: 184, PB9: 185
    u32 dac_sel                          : 2;   //DAC输出选择: 单端输出: 0, 差分输出: 1
    u32 charge_en                        : 1;   //充电使能
    u32 charge_trick_en                  : 1;   //涓流充电使能
    u32 charge_dc_in_rst                 : 1;   //插入DC复位
    u32 poweron_vusb_out_en              : 1;   //退出充电自动开机
    u32 charge_working_while_charging    : 1;   //边充电边工作使能
    u8 charge_stop_curr;                        //充电截止电流: 0:0mA, 1:2.5mA, 2:5mA, 3:7.5mA, 4:10mA, 5:12.5mA, 6:15mA, 7:17.5mA, 8:20mA, 9:22.5mA, 10:25mA, 11:27.5mA, 12:30mA, 13:32.5mA, 14:35mA, 15:37.5mA
    u8 charge_constant_curr;                    //恒流充电电流: 0:5mA, 1:10mA, 2:15mA, 3:20mA, 4:25mA, 5:30mA, 6:35mA, 7:40mA, 8:45mA, 9:50mA, 10:55mA, 11:60mA, 12:65mA, 13:70mA, 14:75mA, 15:80mA, 16:85mA, 17:90mA, 18:95mA, 19:100mA, 20:105mA, 21:110mA, 22:115mA, 23:120mA, 24:125mA, 25:130mA, 26:135mA, 27:140mA, 28:145mA, 29:150mA, 30:155mA, 31:160mA, 32:165mA
    u8 charge_trickle_curr;                     //涓流充电电流: 0:5mA, 1:10mA, 2:15mA, 3:20mA, 4:25mA, 5:30mA, 6:35mA, 7:40mA
    u16 charge_stop_time;                       //充电截止计时: 不计时: 0, 10min: 1, 20min: 2, 30min: 3, 40min: 4, 50min: 5, 60min: 6, 70min: 7, 80min: 8, 90min: 9, 120min: 12, 150min: 15, 180min: 18
    u32 ch_box_type                      : 2;   //充电仓类型: 关闭充电仓功能: 0, 5V不掉电(常5V): 1, 5V掉电到维持电压: 2
    char le_name[32];                           //蓝牙名称
    u8 le_addr[6];                              //蓝牙地址
    u32 rf_tx_pwr                        : 8;   //RF TX功率选择: 11dBm: 11, 8dBm: 8, 4dBm: 4, 0dBm: 0
    u32 bt_rf_param_en                   : 1;   //自定义RF参数
    u8 rf_pa_gain;                              //PA_GAIN
    u8 rf_pa_vsel;                              //PA_VSEL
    u8 rf_pa_mode;                              //PA_MODE
    u32 mic_bias_method                  : 2;   //MIC偏置电路配置: 省RC电路: 0, 单端外部电阻电容: 1, 差分: 2
    u8 mic_bias_res;                            //MIC_BIAS_RES
    u8 mic_dig_gain;                            //MICM数字增益
    u32 mic_anl_gain                     : 2;   //MICM模拟增益: 0db增益: 1, 6db增益: 2, 12db增益: 3
    u32 pwron_press_time                 : 3;   //软开机长按时间选择: 0.1秒: 0, 0.5秒: 1, 1秒: 2, 1.5秒: 3, 2秒: 4, 2.5秒: 5, 3秒: 6, 3.5秒: 7
    u32 pwroff_press_time                : 3;   //软关机长按时间选择: 1.5秒: 0, 2秒: 1, 2.5秒: 2, 3秒: 3, 3.5秒: 4, 4秒: 5, 4.5秒: 6, 5秒: 7
    u32 double_key_time                  : 4;   //双击响应时间选择: 200ms: 2, 300ms: 3, 400ms: 4, 500ms: 5, 600ms: 6, 700ms: 7, 800ms: 8, 900ms: 9
    u32 bled_disp_en                     : 1;   //系统指示灯(蓝灯)
    u32 bled_io_sel                      : 8;   //蓝灯IO选择: PA0: 160, PA1: 161, PA2: 162, PA3: 163, PA4: 164, PA5: 165, PA6: 166, PA7: 167, PA8: 168, PA9: 169, PA10: 170, PA11: 171, PA12: 172, PA13: 173, PA14: 174, PA15: 175, PB0: 176, PB1: 177, PB2: 178, PB3: 179, PB4: 180, PB5: 181, PB6: 182, PB7: 183, PB8: 184, PB9: 185
    u32 rled_disp_en                     : 1;   //充电指示灯(红灯)
    u32 rled_io_sel                      : 8;   //红灯IO选择: PA0: 160, PA1: 161, PA2: 162, PA3: 163, PA4: 164, PA5: 165, PA6: 166, PA7: 167, PA8: 168, PA9: 169, PA10: 170, PA11: 171, PA12: 172, PA13: 173, PA14: 174, PA15: 175, PB0: 176, PB1: 177, PB2: 178, PB3: 179, PB4: 180, PB5: 181, PB6: 182, PB7: 183, PB8: 184, PB9: 185
    u32 led_pwron_config_en              : 1;   //开机状态配置LED
    struct __attribute__((packed)) {
        u8 redpat;
        u8 bluepat;
        u8 unit;
        u8 cycle;
    } led_poweron;                              //开机闪灯控制
    u32 led_pwroff_config_en             : 1;   //关机状态配置LED
    struct __attribute__((packed)) {
        u8 redpat;
        u8 bluepat;
        u8 unit;
        u8 cycle;
    } led_poweroff;                             //关机闪灯控制
    u32 led_btpair_config_en             : 1;   //蓝牙配对状态配置LED
    struct __attribute__((packed)) {
        u8 redpat;
        u8 bluepat;
        u8 unit;
        u8 cycle;
    } led_pairing;                              //配对闪灯控制
    u32 led_btconn_config_en             : 1;   //蓝牙已连接状态配置LED
    struct __attribute__((packed)) {
        u8 redpat;
        u8 bluepat;
        u8 unit;
        u8 cycle;
    } led_connected;                            //已连接闪灯控制
    u8 xm_keep_start[0];                        //For Keep Area Start
    u8 osc_both_cap;                            //产测OSC基础电容
    u8 osci_cap;                                //产测OSCI电容
    u8 osco_cap;                                //产测OSCO电容
} xcfg_cb_t;

extern xcfg_cb_t xcfg_cb;
#endif

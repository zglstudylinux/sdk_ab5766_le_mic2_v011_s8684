#include "include.h"
#include "driver_saradc.h"
#include "driver_charge.h"
#include "bsp_saradc_vbat.h"

#if BSP_VBAT_DETECT_EN

#define VBAT_FILTER_DIFF_VAL    30  //30mV
#define VBAT_FILTER_BUF_LEN     3

static u32 vbat_voltage = 0;
static lpwr_param_t lpwr_param;

// 电池电量相关参数
const vbat_param_struct vbat_param = {
    .vbat_max_level = 100,
    .vbat_min_power = 2500,
    .vbat_max_power = 4200,
};

// 初始化Vbat检测
void bsp_vbat_detect_init(void)
{
    saradc_channel_init_typedef saradc_channel_init_struct;

    saradc_channel_init_struct.channel = ADC_CHANNEL_VBAT | ADC_CHANNEL_VBG;
    saradc_channel_init_struct.pullup_en = SARADC_PULLUP_DIS;
    saradc_channel_init_struct.setup_time = SARADC_ST_8_CLK;

    saradc_channel_init(&saradc_channel_init_struct);

    saradc_kick_start(ADC_CHANNEL_VBAT | ADC_CHANNEL_VBG);

    while(!saradc_get_channel_flag(ADC_CHANNEL_VBAT | ADC_CHANNEL_VBG));

    memset((u8 *)&lpwr_param, 0, sizeof(lpwr_param_t));

    bsp_vbat_proc();
}

uint32_t bsp_vbat_filter(uint32_t voltage)
{
    static uint8_t div = 1;
    static uint8_t index = 0;
    static uint32_t filter_buf[VBAT_FILTER_BUF_LEN] = {0};
    uint32_t total_vol = 0;
    uint32_t result = 0;

    filter_buf[index] = voltage;
    for (uint8_t i=0; i<VBAT_FILTER_BUF_LEN; i++) {
        total_vol += filter_buf[i];
    }
    result =  total_vol / div;
    div = (div >= VBAT_FILTER_BUF_LEN) ? VBAT_FILTER_BUF_LEN : (div+1);
    index = (index >= (VBAT_FILTER_BUF_LEN-1)) ? 0 : (index+1);

    return result;
}

AT(.text.app.proc.vbat)
void bsp_vbat_lowpwr_detect(void)
{
    bool charge_in = false;
    u16 bat_volt = bsp_get_vbat_level(1);

    printf("vbat: %d\n", bat_volt);
#if BSP_CHARGE_EN
    charge_in = (charge_detect_dc() == CHARGE_DC_STATUS_ONLINE) ? true : false;
#endif

    if (xcfg_cb.lpwr_warning_vbat_level != 0) {
        if (bat_volt < LPWR_WARNING_VBAT && !charge_in) {
            lpwr_param.lpwr_warning_vbat_cnt++;
            if (lpwr_param.lpwr_warning_vbat_cnt >= LPWR_DETECT_CNT) {
                lpwr_param.lpwr_warning_vbat_cnt = 0;
                if (!lpwr_param.lpwr_warning_vbat_flag) {
                    lpwr_param.lpwr_warning_vbat_flag = 1;
                    msg_enqueue(EVT_LPWR_WARNING);
                }
            }
            lpwr_param.lpwr_warning_vbat_exit_cnt = 0;
        } else {
            lpwr_param.lpwr_warning_vbat_exit_cnt++;
            if (lpwr_param.lpwr_warning_vbat_exit_cnt >= LPWR_DETECT_CNT) {
                lpwr_param.lpwr_warning_vbat_exit_cnt = 0;
                if (lpwr_param.lpwr_warning_vbat_flag) {
                    lpwr_param.lpwr_warning_vbat_flag = 0;
                    msg_enqueue(EVT_LPWR_WARNING_EXIT);
                }
            }
            lpwr_param.lpwr_warning_vbat_cnt = 0;
        }
    }

    if (xcfg_cb.lpwr_off_vbat_level != 0) {
        if (bat_volt <= LPWR_OFF_VBAT && !charge_in) {
            if (lpwr_param.lpwr_off_vbat_cnt < LPWR_DETECT_CNT) {
                lpwr_param.lpwr_off_vbat_cnt++;
            } else {
                msg_enqueue(EVT_LPWR_OFF);
            }
        } else {
            lpwr_param.lpwr_off_vbat_cnt = 0;
        }
    }
}

AT(.text.app.proc.vbat)
bool bsp_vbat_proc(void)
{
    uint32_t adc_val;
    uint32_t adc_vbg;
    uint32_t vbat_cur;
    uint16_t diff;

    if (saradc_get_channel_flag(ADC_CHANNEL_VBAT | ADC_CHANNEL_VBG)) {
        adc_val = saradc_get_data(ADC_CHANNEL_VBAT);
        adc_vbg = saradc_get_data(ADC_CHANNEL_VBG);
        vbat_cur = adc_val * ADC_VOLTAGE_VBG / adc_vbg;
        vbat_cur = bsp_vbat_filter(vbat_cur);
        if(vbat_voltage > vbat_cur) {
            diff = vbat_voltage - vbat_cur;
        } else {
            diff = vbat_cur - vbat_voltage;
        }
        if (diff >= VBAT_FILTER_DIFF_VAL) {
            vbat_voltage = vbat_cur;
        }
        saradc_kick_start(ADC_CHANNEL_VBAT | ADC_CHANNEL_VBG);

        bsp_vbat_lowpwr_detect();

        return true;
    }

    return false;
}

// 获取电池电量： 1：电压(mv)  0：电量等级(%)
uint16_t bsp_get_vbat_level(u8 mode)
{
    uint8_t vbat_level;
    if (mode == 0) {
        if (vbat_param.vbat_min_power > vbat_voltage) {
            return 0;
        }
        vbat_level = vbat_param.vbat_max_level * \
        (vbat_voltage - vbat_param.vbat_min_power) / (vbat_param.vbat_max_power - vbat_param.vbat_min_power);
        vbat_level = (vbat_level > vbat_param.vbat_max_level) ? vbat_param.vbat_max_level : vbat_level;
        return vbat_level;
    } else {
        return vbat_voltage;
    }
}

#endif // BSP_VBAT_DETECT_EN

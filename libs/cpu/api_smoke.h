#ifndef _API_SMOKE_H_
#define _API_SMOKE_H_
#include "include.h"

typedef struct{
    u32 AD_hot_voltage;         //定点VAT
    u32 AD_hot_voltage_mv;      //真实VAT
    u32 AD_BAT_voltage;         //VBAT

    u16 p_current;              //当前功率
    u16 p_prev;                 //前一次功率
    u8  power_on_flag;          //PWM开启标志位
    u8  hot_time_flag;          //开始8S计时标志
    u8  hot_time_cnt;           //PWM计时第几秒
    u8  timer_100us_cnt;        //100us计时
    u8  short_circuit_flag;     //短路标志位
    u8  short_circuit_cnt;      //短路累计次数，超过10次算短路
    u8  mic_start;              //mic start信号，仅双路adc使用
    u16 hot_res;                //双路adc测量的电热丝阻值，仅双路adc使用

volatile u32 adc1;
volatile u32 adc2;

    u8  final_power;            //目标功率
    u8  max_hot_time;           //最长加热时间
    u8  min_proportion;         //电热丝阻值和MOS内阻（130mΩ）的最小比例，阻值相近乘10提高精度，30--0.4Ω视作短路
    u16 max_proportion;         //电热丝阻值和MOS内阻（130mΩ）的最大比例，阻值相近乘10提高精度，153--2Ω视作开路
    u16 res19;                  //电阻定点值
    u16 res_cal_prev;           //电热丝阻值定点
    u8 smoke_sta;
}smoke_ctrl_t;

void smoke_start_init(smoke_ctrl_t *smk_info);

void timer_hot_dual_isr(void);//100us
void timer_hot_single_isr(void);//100us
#endif

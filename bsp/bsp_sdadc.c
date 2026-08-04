#include "include.h"
#include "driver_sdadc.h"
#include "bsp_sdadc.h"
#include "driver_gpio.h"
#include "driver_sdadc_phy.h"
#if WIRELESS_MIC_32K_EN
#define SDADC_DMA_SIZE                        (80*2)
#define WIRELESS_MIC_SAMPLES_RESELECT_SIZE     80

#else
#define SDADC_DMA_SIZE                        (WIRELESS_MIC_SAMPLES_SELECT*2)
#define WIRELESS_MIC_SAMPLES_RESELECT_SIZE     WIRELESS_MIC_SAMPLES_SELECT
#endif
int16_t sdadc_buf[SDADC_DMA_SIZE] AT(.buf.sdadc);
volatile bool sdadc_w4_kick AT(.buf.sdadc);
static volatile u16 sdadc_discard_cnt AT(.buf.sdadc);               //丢掉adc的开始的帧

static const u16 dig_gain_tbl[37] = {
    1024,   1148,   1289,   1446,   1622,    1820,  2043,    2292,
    2572,   2886,   3238,   3633,   4076,   4574,    5132,   5758,
    6461,   7249,   8133,   9126,   10240,  11489,  12891,  14464,
    16229,  18209,  20431,  22924,  25721,  28860,  32381,  36332,
    40766,  45740,  51321,  57583,  64610,
};

AT(.com_text.sdadc.isr)
static void sdadc_isr_cb(void)
{
    wl_save_tick1_time();
    if (sdadc_get_flag(SDADC_REG, SDADC_FLAG_DMA_ALL_DONE) != RESET) {
        sdadc_clear_flag(SDADC_REG, SDADC_FLAG_DMA_ALL_DONE);
        sdadc_done_proc_kick(SDADC_ALL_DONE);
    } else if (sdadc_get_flag(SDADC_REG, SDADC_FLAG_DMA_HALF_DONE) != RESET) {
        sdadc_clear_flag(SDADC_REG, SDADC_FLAG_DMA_HALF_DONE);
        sdadc_done_proc_kick(SDADC_HALF_DONE);
    }
}

AT(.text.mic_emit.proc.sdadc)
static void sdadc_proc_cb(SDADC_DONE_TYPEDEF type)
{
    if(type == SDADC_ALL_DONE){
        void *rbuf = &sdadc_buf[WIRELESS_MIC_SAMPLES_RESELECT_SIZE];
//        memcpy(rbuf, adc_test_data, 60);
        mic_enc_proc_cb(rbuf, WIRELESS_MIC_SAMPLES_RESELECT_SIZE);
    }

    if(type == SDADC_HALF_DONE){
        void *rbuf = sdadc_buf;
//        memcpy(rbuf, adc_test_data, 60);
        mic_enc_proc_cb(rbuf, WIRELESS_MIC_SAMPLES_RESELECT_SIZE);
    }
}

void bsp_sdadc_init(void)
{
    sdadc_init_typedef sdadc_cfg_init;
    sdadc_ana_cfg_typedef sdadc_ana_cfg;

    sdadc_done_callback_set(sdadc_proc_cb);

    clk_gate0_cmd(CLK_GATE0_SDADC, CLK_EN);
    clk_sdadda_clk_set(CLK_SDADDA_CLK48_A);

    sdadc_cfg_init.adc_mode = SDADC_MODE_MIC;  //MIC or DC(electronic cigarette)
    //adccon
    sdadc_cfg_init.swclk_en = DISABLE;
    sdadc_cfg_init.swclk_peri = 0;
    sdadc_cfg_init.swclk_duty = 0;
    //adc0con
#if WIRELESS_MIC_32K_EN
    sdadc_cfg_init.sample_rate = SAMPLE_RATE_32K;
#else
    sdadc_cfg_init.sample_rate = WIRELESS_MIC_SAMPLE_RATE_SELECT;
#endif
    sdadc_cfg_init.bits_sel = BITS_SEL_ITSEL_CH;
    sdadc_cfg_init.n6db_en = DISABLE;
    sdadc_cfg_init.inv_en = DISABLE;
    //adc0gain
    sdadc_cfg_init.dig_gain = dig_gain_tbl[MIC_DIG_GAIN];
    //adc0rmdccon
    sdadc_cfg_init.rmdc_en = ENABLE;
    sdadc_cfg_init.rmdc_order = RMDC_ORDER_1;
    sdadc_cfg_init.rmdc_sel = RMDC_SEL_11BIT;   
    sdadc_cfg_init.sw_dc_en = DISABLE;
    sdadc_cfg_init.get_dc_en = DISABLE;
    sdadc_cfg_init.sw_dc_value = 0;
    sdadc_init(SDADC_REG, &sdadc_cfg_init);

    //analog
    sdadc_ana_cfg.adc_mode = sdadc_cfg_init.adc_mode;
    sdadc_ana_cfg.mic_mode = MIC_ANA_MODE;
    sdadc_ana_cfg.lv_mode_en = MIC_VDD_LVMODE;
    sdadc_ana_cfg.mic_bias_res_level = MIC_BIAS_RES_LEVEL;
    sdadc_ana_cfg.ana_gain = MIC_ANA_GAIN;
    sdadc_ana_init(&sdadc_ana_cfg);


    sdadc_w4_kick = true;
    sdadc_pic_config(SDADC_REG, sdadc_isr_cb, 0, SDADC_FLAG_DMA_HALF_DONE | SDADC_FLAG_DMA_ALL_DONE, ENABLE);
    sdadc_dma_cmd(SDADC_REG, (u32)sdadc_buf, SDADC_DMA_SIZE - 1, DMA_DATA_MODE_16BITS, ENABLE);
    wl_tick_time_init();
//    sdadc0_cmd(SDADC_REG, ENABLE);
//    sdadc_total_cmd(SDADC_REG, ENABLE);
}

AT(.text.mic_emit.proc.sdadc)
void bsp_sdadc_kick(void)
{
    if(sdadc_w4_kick) {
        sdadc_w4_kick = false;
        sdadc_discard_cnt = 0;
        sdadc0_cmd(SDADC_REG, ENABLE);
        sdadc_total_cmd(SDADC_REG, ENABLE);
    }
}

AT(.text.mic_emit.proc.sdadc)
bool bsp_sdadc_discard_proc(void)
{
    if (sdadc_discard_cnt < 3+4) {     //跳过开始的N帧
        sdadc_discard_cnt++;
        if (sdadc_discard_cnt == 3) {  //7.5ms后恢复正常的RMDC
            sdadc_rmdc_restore();
        }
        return true;
    }
    return false;
}

void bsp_sdadc_exit(void)
{
    printf("bsp_sdadc_exit\n");
    sdadc_deinit(SDADC_REG);
}

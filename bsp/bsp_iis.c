#include "include.h"
#include "driver_iis.h"
#include "bsp_iis.h"

#define IIS_MASTER              BIT(0)
#define IIS_SLAVE               BIT(1)
#define IIS_TX                  BIT(2)
#define IIS_RX                  BIT(3)

#define IIS_MODE    (IIS_MASTER | IIS_TX)     //主机TX模式
// #define IIS_MODE    (IIS_SLAVE | IIS_RX)   //从机RX模式

#define IIS_TX_SAMPLES  120    //DMA TX样点数
#define IIS_TX_BUF_LEN  (IIS_TX_SAMPLES * 2 * 2)  //TX buf len = 样点 * 声道 * 位深（转成byte数）

#define IIS_RX_SAMPLES  120  //DMA RX样点数，小于ch_depth_sel配置的样点数
#define IIS_RX_BUF_LEN  (IIS_RX_SAMPLES * 2 * 2)  //RX buf len = 样点 * 声道 * 位深（转成byte数）

AT(.com_text.sindata)
const uint16_t SinData_48k_ch2_16bit[96] = {
    0x0000, 0x5A82, 0x10B5, 0x658C, 0x2121, 0x6ED9, 0x30FB, 0x7641, 0x3FFF, 0x7BA2,
    0x4DEB, 0x7EE7, 0x5A82, 0x7FFF, 0x658C, 0x7EE7, 0x6ED9, 0x7BA2, 0x7641, 0x7641,
    0x7BA2, 0x6ED9, 0x7EE7, 0x658C, 0x7FFF, 0x5A82, 0x7EE7, 0x4DEB, 0x7BA2, 0x4000,
    0x7641, 0x30FB, 0x6ED9, 0x2121, 0x658C, 0x10B5, 0x5A82, 0x0000, 0x4DEB, 0xEF4B,
    0x4000, 0xDEDF, 0x30FB, 0xCF05, 0x2121, 0xC001, 0x10B5, 0xB215, 0x0000, 0xA57E,
    0xEF4B, 0x9A74, 0xDEDF, 0x9127, 0xCF05, 0x89BF, 0xC001, 0x845E, 0xB215, 0x8119,
    0xA57E, 0x8001, 0x9A74, 0x8119, 0x9127, 0x845E, 0x89BF, 0x89BF, 0x845E, 0x9127,
    0x8119, 0x9A74, 0x8001, 0xA57E, 0x8119, 0xB215, 0x845E, 0xC000, 0x89BF, 0xCF05,
    0x9127, 0xDEDF, 0x9A74, 0xEF4B, 0xA57E, 0x0000, 0xB215, 0x10B5, 0xC000, 0x2121,
    0xCF05, 0x30FB, 0xDEDF, 0x3FFF, 0xEF4B, 0x4DEB};

AT(.buf.iis_buf)
uint8_t iis_txbuf[2][IIS_TX_BUF_LEN];

AT(.buf.iis_buf)
uint8_t iis_rxbuf[2][IIS_RX_BUF_LEN];

AT(.buf.iis_buf)
iis_init_typedef iis_cb;

void bsp_iis_dump(void)
{
    // printf("CLKCON1:%x\n", CLKCON1);
    // printf("IISCON0:%x\n", IISCON0);
    // printf("IISCON1:%x\n", IISCON1);
    // printf("IISDMATXADR0:%x\n", IISDMATXADR0);
    // printf("IISDMATXADR1:%x\n", IISDMATXADR1);
    // printf("IISDMARXADR0:%x\n", IISDMARXADR0);
    // printf("IISDMARXADR1:%x\n", IISDMARXADR1);
    // printf("IISDMATXSIZE0:%x\n", IISDMATXSIZE0);
    // printf("IISDMATXSIZE1:%x\n", IISDMATXSIZE1);
    // printf("IISMCLKDIV:%x\n", IISMCLKDIV);
    // printf("IISCPND:%x\n", IISCPND);
    // printf("iis_txbuf0:%x\n", iis_txbuf[0]);
    // printf("iis_txbuf1:%x\n", iis_txbuf[1]);
    // printf("iis_cb.dma_cfg.dma_txbuf_ptr0:%x\n", iis_cb.dma_cfg.dma_txbuf0_ptr);
    // printf("iis_cb.dma_cfg.dma_txbuf_ptr1:%x\n", iis_cb.dma_cfg.dma_txbuf1_ptr);
    // printf("rx_len:%d\n", (32 << iis_cb.ch_depth_sel) * (iis_cb.rx_bit_depth / 8) * 2);
}

AT(.com_text.iis_isr)
void bsp_iis_isr_rx_cb(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
//    uint8_t *rx_ptr = (uint8_t *)iis_reg->dma_rx_adr1;  //IIS收数是先写进adr0，写满后把adr0指向的地址给adr1，然后再更新adr0的地址，所以每次读adr1的数据即可。
//    uint16_t rx_len = (32 << iis_cb->ch_depth_sel) * (iis_cb->ch_cnt_sel+1)*2 * (iis_cb->rx_bit_depth / 8);  //样点 * 声道 * 位深（转成byte数）
//    for (int16_t i = 0; i < rx_len; i+=4) {
//        //测试：只取左声道数据
//        bsp_audio_dump(rx_ptr[i]);
//        bsp_audio_dump(rx_ptr[i+1]);
//    }
}

AT(.com_text.iis_isr)
void bsp_iis_isr_tx_cb(iis_typedef *iis_reg, iis_init_typedef *iis_cb)
{
    //测试:固定发双声道48K正弦波
    iis_dma_tx_kick(iis_reg, iis_cb, (uint8_t *)SinData_48k_ch2_16bit, sizeof(SinData_48k_ch2_16bit));
}

AT(.com_text.iis_isr)
void bsp_iis_isr(void)
{
    if (iis_get_flag(IIS_REG, IIS_FLAG_RX_PND) == SET) {
        iis_clear_flag(IIS_REG, IIS_FLAG_RX_PND);
        bsp_iis_isr_rx_cb(IIS_REG, &iis_cb);
    }

    if (iis_get_flag(IIS_REG, IIS_FLAG_TX_PND) == SET) {
        iis_clear_flag(IIS_REG, IIS_FLAG_TX_PND);
        bsp_iis_isr_tx_cb(IIS_REG, &iis_cb);
    }
}

void bsp_iis_init(void)
{
#if (IIS_MODE & IIS_MASTER)
//    pll0_init(PLL0_DIV_VAL);
    //48k bclk
    clk_iis_bclk_set(CLK_IIS_BCLK_PLL0_DIV2P5, 125);  // div = iis clk / CH_CNT / WORD_WIDTH / fs  ==> (PLL 480 000 000/2.5)/2/16/(48 000) = 125
    clk_gate1_cmd(CLK_GATE1_IIS, CLK_EN);
    clk_iis_mclk_set(CLK_IIS_MCLK_PLL0_DIV2P5, 125);
    clk_gate1_cmd(CLK_GATE1_IISM, CLK_EN);
    iis_cb.role = IIS_ROLE_MASTER;
    iis_cb.mclk_out_en = ENABLE;           //主机输出mclk
    iis_cb.tx_dat_src = IIS_DAT_SRC_DMA;   //TX数据源可选DAC / DMA

//    //测试：DAC数据源
//    if (iis_cb.tx_dat_src == IIS_DAT_SRC_DAC) {
//        bsp_sddac_init();
//    }
#elif (IIS_MODE & IIS_SLAVE)
    clk_iis_bclk_set(CLK_IIS_BCLK_BCLK_IN, 0);  //从机选BCLK IN，分频无效
    iis_cb.role = IIS_ROLE_SLAVE;
    clk_gate1_cmd(CLK_GATE1_IIS, CLK_EN);
    iis_cb.mclk_out_en = DISABLE;          //从机不用输出mclk
    iis_cb.tx_dat_src = IIS_DAT_SRC_DMA;   //TX数据源可选DAC / DMA
#endif

    iis_cb.tx_en = (bool)(IIS_MODE & IIS_TX);  //TX使能
    iis_cb.rx_en = (bool)(IIS_MODE & IIS_RX);  //RX使能
    iis_cb.iomap = IIS_IOMAP_0;  //支持两组IO，每组固定IO口
    iis_cb.dat_mode = IIS_MODE_NORMAL;        //标准IIS模式/左对齐
    iis_cb.dac_out_sel = IIS_DAC_OUT_SRC0;    //主机的DAC模式有效（dat_src == IIS_DAT_SRC_DAC），从DAC取数据
    iis_cb.ws_pulse_sel = IIS_PULSE_NORMALL;  //占空比
    iis_cb.word_width = 16;    //样点的位宽, 最小8位，最大32位
    iis_cb.tx_bit_depth = 16;  //tx样点的有效位，最小8位，最大32位，不能大于word_width
    iis_cb.rx_bit_depth = 16;  //rx样点的有效位，最小8位，最大32位，不能大于word_width
    iis_cb.ch_cnt_sel = IIS_CH_CNT_2;       //声道数，单线模式最多支持16声道，多线模式最多支持2声道
    iis_cb.ch_depth_sel = IIS_CH_DEPTH_128;  //每个声道收发的最大样点数，主机的DAC TX模式不受该参数限制
    iis_cb.slv_lose_ch_hdl_ws = DISABLE;    //slv：从机有效
    iis_cb.slv_dat_sync_ws = DISABLE;
    iis_cb.rx_bit_rev = DISABLE;
    iis_cb.tx_bit_rev = DISABLE;
    iis_cb.slv_di_flt_en = DISABLE;
    iis_cb.slv_ws_flt_en = DISABLE;
    iis_cb.slv_bclk_flt_en = DISABLE;
    iis_cb.ws_inv = DISABLE;
    iis_cb.bclk_inv = DISABLE;
    iis_cb.one_wire_dir = IIS_ONE_WIRE_OUTPUT;    //单线模式主从机都用DO做数据脚
    iis_cb.one_wire_en = DISABLE;    //单线模式主从机都用DO做数据脚

    iis_cb.dma_cfg.tx_samples = IIS_TX_SAMPLES;
    iis_cb.dma_cfg.dma_txbuf0_ptr =  iis_txbuf[0];
    iis_cb.dma_cfg.dma_txbuf1_ptr =  iis_txbuf[1];
    iis_cb.dma_cfg.dma_rxbuf0_ptr =  iis_rxbuf[0];
    iis_cb.dma_cfg.dma_rxbuf1_ptr =  iis_rxbuf[1];

    memset(iis_txbuf, 0, sizeof(iis_txbuf));
    memset(iis_rxbuf, 0, sizeof(iis_rxbuf));

    iis_init(IIS_REG, &iis_cb);
    iis_pic_config(IIS_REG, bsp_iis_isr, 0, IIS_FLAG_RX_PND | IIS_FLAG_TX_PND, ENABLE);
    iis_cmd(IIS_REG, ENABLE);
    iis_dma_init(IIS_REG, &iis_cb);

}

void bsp_iis_proc(void)
{

}

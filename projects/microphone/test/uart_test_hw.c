#include "include.h"
#include "uart_test_hw.h"

#if UART_SPEED_TEST_EN

#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_hsuart.h"
#include "driver_clk.h"

//UART1 与 HSUART 分时复用 PA0=TX / PA1=RX
#define TEST_TX_PIN                 GPIO_PA0
#define TEST_RX_PIN                 GPIO_PA1

//普通串口波特率时钟源分档: XOSC24M 实测稳定到 1.5M(调试口), 更高档切 XOSC48M
#define UART1_XOSC24M_MAX_BAUD      1500000

//HSUART 实测踩坑记录(两次回环崩溃, EPC 均为 0x1000bc3c, cause=中断异常+IRQ15 pending):
//1. 自写初始化(无论 DMA 还是 buf 模式)都在使能 HSUART 中断后立即 EXCEPT/REQ TOUT 复位;
//2. libplatform.a 中没有任何 HSUART 符号 => 本 SDK 官方从未用过 HSUART, driver_hsuart.c
//   是从 AB202X 移植的(文件头注释), RSTCON0|=1<<2 等"上电/复位"操作在 AB5766 上的副作用未知;
//3. 当前策略: 完全照抄原厂 bsp_huart.c 的初始化/收发序列(收发均 DMA、idle timer、
//   中断优先级 0、不动时钟 mux、baud 交驱动自动算), 先在原厂验证过的 <=1.5M 域跑通,
//   init 每一步之间打印定位标记, 若仍崩溃则把日志交给原厂定位;
//4. 缓冲布局: .buf.le_dut.uart_test 段必须整体 <= 2KB(0x17800~0x18000 安全窗口),
//   其余放普通 .bss; comm 区(0x10c00~0x17800)余量仅约 800B, .bss 增量不可超过该值。
#define UART1_RING_SIZE             512         //必须是 2 的幂
#define HS_RING_SIZE                512         //必须是 2 的幂
#define HS_DMA_BUF_SIZE             256         //原厂 le_dut 用例为 128B, 同量级

typedef struct {
    volatile u16 head;
    volatile u16 tail;
    u8 buf[UART1_RING_SIZE];
} test_ring_t;

//两个环形缓冲 + 两个协议解析缓冲 + HSUART RX DMA 缓冲放 .buf.le_dut.uart_test 段,
//合计约 1.8KB < 2KB 安全窗口(见文件顶部说明)
static test_ring_t uart1_ring AT(.buf.le_dut.uart_test);
static test_ring_t hs_ring AT(.buf.le_dut.uart_test);
static uart_hw_type_t active_type = UART_HW_UART1;
static volatile u32 ring_drop_cnt;

static u8 hs_dma_buf[HS_DMA_BUF_SIZE] AT(.buf.le_dut.uart_test);

static void ring_push(test_ring_t *r, u16 size_mask, u8 byte)
{
    u16 next = (r->head + 1) & size_mask;

    if (next != r->tail) {
        r->buf[r->head] = byte;
        r->head = next;
    } else {
        ring_drop_cnt++;
    }
}

u32 uart_hw_get_drop_cnt(void)
{
    return ring_drop_cnt;
}

u16 uart_hw_read(u8 *buf, u16 len)
{
    test_ring_t *r = (active_type == UART_HW_HSUART) ? &hs_ring : &uart1_ring;
    u16 mask = (active_type == UART_HW_HSUART) ? (HS_RING_SIZE - 1) : (UART1_RING_SIZE - 1);
    u16 cnt = 0;

    while ((cnt < len) && (r->tail != r->head)) {
        buf[cnt++] = r->buf[r->tail];
        r->tail = (r->tail + 1) & mask;
    }

    return cnt;
}

/*************************** 普通串口 UART1 ***************************/

AT(.com_text.isr)
void uart_test_uart1_isr(void)
{
    if (uart_get_flag(UART1_REG, UART_IT_RX) != RESET) {
        u8 byte = uart_receive_data(UART1_REG);

        uart_clear_flag(UART1_REG, UART_IT_RX);
        ring_push(&uart1_ring, UART1_RING_SIZE - 1, byte);
    }
}

static bool uart1_init(u32 baud)
{
    gpio_init_typedef gpio_init_structure;
    uart_init_typedef uart_init_struct;

    uart1_ring.head = 0;
    uart1_ring.tail = 0;

    clk_gate0_cmd(CLK_GATE0_UART1, CLK_EN);

    //RX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(TEST_RX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
    gpio_init(GPIO_PORT_GET(TEST_RX_PIN), &gpio_init_structure);

    //TX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(TEST_TX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(GPIO_PORT_GET(TEST_TX_PIN), &gpio_init_structure);

    gpio_func_mapping_config(GPIO_PORT_GET(TEST_RX_PIN), GPIO_PIN_GET(TEST_RX_PIN), GPIO_CROSSBAR_IN_UART1RX);
    gpio_func_mapping_config(GPIO_PORT_GET(TEST_TX_PIN), GPIO_PIN_GET(TEST_TX_PIN), GPIO_CROSSBAR_OUT_UART1TX);

    uart_init_struct.baud = baud;
    uart_init_struct.mode = UART_SEND_RECV_MODE;
    uart_init_struct.word_len = UART_WORD_LENGTH_8b;
    uart_init_struct.stop_bits = UART_STOP_BIT_1BIT;
    uart_init_struct.one_line_enable = UART_SEPARATE;
    uart_init_struct.baud_fix = UART_BAUD_FIX;
    uart_init(UART1_REG, &uart_init_struct);

    //覆盖 bsp_uart_transfer 注册的回显 ISR, 测试收发统一走这里
    uart_pic_config(UART1_REG, uart_test_uart1_isr, 0, UART_IT_RX, ENABLE);

    uart_cmd(UART1_REG, ENABLE);

    return true;
}

static bool uart1_set_baud(u32 baud)
{
    if (baud > UART1_XOSC24M_MAX_BAUD) {
        //XOSC48M 档位本仓库未实测, 若该档全误码则普通串口实测上限即 1.5M
        clk_gate2_cmd(CLK_GATE2_X48M, CLK_EN);
        clk_uart_clk_set(UART1_REG, CLK_UART_XOSC48M);
    } else {
        clk_uart_clk_set(UART1_REG, CLK_UART_XOSC24M);
    }

    uart_baud_config(UART1_REG, baud);

    return true;
}

static void uart1_deinit(void)
{
    uart_pic_config(UART1_REG, NULL, 0, UART_IT_RX, DISABLE);
    uart_cmd(UART1_REG, DISABLE);
    clk_gate0_cmd(CLK_GATE0_UART1, CLK_DIS);
}

/*************************** 高速串口 HSUT0 ***************************/

//HSUART RX 走 buf 模式+中断(不用 DMA, 原因见文件顶部), ISR 里从 FIFO 批量搬到环形缓冲
//照抄原厂 bsp_huart.c 的 huart_rx_isr: RX DMA 写满或线路空闲超时后触发,
//fifo_cnt 为本次 DMA 收到的字节数, 搬进环形缓冲后重新挂 RX DMA
AT(.com_text.isr)
void uart_test_hsuart_isr(void)
{
    if ((hsuart_get_flag(HSUART_REG, HSUART_IT_RX) != RESET) ||
        (hsuart_get_flag(HSUART_REG, HSUART_IT_RX_TMR_OV) != RESET)) {
        u16 cnt = hsuart_get_fifo_counter(HSUART_REG);
        u16 i;

        for (i = 0; i < cnt; i++) {
            ring_push(&hs_ring, HS_RING_SIZE - 1, hs_dma_buf[i]);
        }

        hsuart_clear_flag(HSUART_REG, HSUART_IT_RX | HSUART_IT_RX_TMR_OV);
        hsuart_dma_start(HSUART_REG, HSUT_RECEIVE, DMA_ADR(hs_dma_buf), HS_DMA_BUF_SIZE);
    }
}

//完全对齐原厂 bsp_huart_init 的序列(收发均 DMA), 每步打印用于崩溃定位
static bool hsuart_hw_init(u32 baud)
{
    gpio_init_typedef gpio_init_structure;
    hsuart_init_typedef hsuart_init_struct;

    hs_ring.head = 0;
    hs_ring.tail = 0;

    clk_gate0_cmd(CLK_GATE0_HSUART, CLK_EN);

    //RX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(TEST_RX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
    gpio_init(GPIO_PORT_GET(TEST_RX_PIN), &gpio_init_structure);

    //TX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(TEST_TX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(GPIO_PORT_GET(TEST_TX_PIN), &gpio_init_structure);

    gpio_func_mapping_config(GPIO_PORT_GET(TEST_RX_PIN), GPIO_PIN_GET(TEST_RX_PIN), GPIO_CROSSBAR_IN_HURRX);
    gpio_func_mapping_config(GPIO_PORT_GET(TEST_TX_PIN), GPIO_PIN_GET(TEST_TX_PIN), GPIO_CROSSBAR_OUT_HURTX);
    printf("[HS] 1.gpio ok RSTCON0=%x\n", RSTCON0);

    hsuart_init_struct.baud = baud;                         //原厂: 交驱动按当前时钟源自动计算
    hsuart_init_struct.tx_mode = HSUT_TX_DMA_MODE;          //原厂: 收发均 DMA
    hsuart_init_struct.rx_mode = HSUT_RX_DMA_MODE;
    hsuart_init_struct.rx_dma_buf_mode = HSUT_DMA_BUF_SINGLE;
    hsuart_init_struct.tx_stop_bit = HSUT_STOP_BIT_2BIT;    //原厂: TX 2 停止位
    hsuart_init_struct.tx_word_len = HSUT_TX_LENGTH_8b;
    hsuart_init_struct.rx_word_len = HSUT_RX_LENGTH_8b;
    hsuart_init_struct.one_line_en = HSUT_ONELINE_DIS;
    hsuart_init(HSUART_REG, &hsuart_init_struct);
    printf("[HS] 2.init ok\n");

    hsuart_clear_flag(HSUART_REG, HSUART_IT_RX | HSUART_IT_RX_TMR_OV);
    hsuart_rx_idle_config(HSUART_REG, 0x20, ENABLE);
    hsuart_pic_config(HSUART_REG, uart_test_hsuart_isr, 0, HSUART_IT_RX, ENABLE);   //原厂: 优先级 0
    printf("[HS] 3.pic ok\n");

    hsuart_cmd(HSUART_REG, HSUT_TRANSMIT | HSUT_RECEIVE, ENABLE);
    hsuart_dma_start(HSUART_REG, HSUT_RECEIVE, DMA_ADR(hs_dma_buf), HS_DMA_BUF_SIZE);
    printf("[HS] 4.rxdma ok\n");

    return true;
}

static void hsuart_hw_deinit(void)
{
    hsuart_pic_config(HSUART_REG, NULL, 0,
                      HSUART_IT_RX_DMA_HF | HSUART_IT_TX | HSUART_IT_RX, DISABLE);
    hsuart_cmd(HSUART_REG, HSUT_TRANSMIT | HSUT_RECEIVE, DISABLE);
    hsuart_deinit(HSUART_REG);
}

//原厂 huart_putchar 的模式: 等 TXPND 后写 data 寄存器(小帧发送)
static bool hsuart_putbyte_polled(u8 byte)
{
    u32 wait = 0;

    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET) {
        if (++wait > 1000000) {
            return false;
        }
    }
    hsuart_send_data(HSUART_REG, byte);

    return true;
}

/*************************** 统一接口 ***************************/

bool uart_hw_init(uart_hw_type_t type, u32 baud)
{
    active_type = type;
    ring_drop_cnt = 0;

    if (type == UART_HW_HSUART) {
        return hsuart_hw_init(baud);
    }

    return uart1_init(baud);
}

bool uart_hw_set_baud(uart_hw_type_t type, u32 baud)
{
    if (type == UART_HW_HSUART) {
        //原厂 huart_set_baudrate 的方式: 驱动按当前时钟源自动算分频
        hsuart_baud_config(HSUART_REG, baud);
        return true;
    }

    return uart1_set_baud(baud);
}

void uart_hw_deinit(uart_hw_type_t type)
{
    if (type == UART_HW_HSUART) {
        hsuart_hw_deinit();
    } else {
        uart1_deinit();
    }
}

bool uart_hw_putbyte(uart_hw_type_t type, u8 byte)
{
    if (type == UART_HW_HSUART) {
        return hsuart_putbyte_polled(byte);
    }

    //TXPND 空闲/完成时为 SET, 写 data 后硬件自动清零
    uart_send_data(UART1_REG, byte);
    {
        u32 wait = 0;

        while (uart_get_flag(UART1_REG, UART_IT_TX) != SET) {
            if (++wait > 200000) {
                return false;
            }
        }
    }

    return true;
}

//照抄原厂 bsp_huart_tx + bsp_huart_wait_txdone: TX DMA 发一块并等完成
bool uart_hw_hsuart_tx_block(u8 *buf, u16 len)
{
    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET) {
        WDT_CLR();
    }
    hsuart_dma_start(HSUART_REG, HSUT_TRANSMIT, DMA_ADR(buf), len);

    while (hsuart_get_flag(HSUART_REG, HSUART_IT_TX) == RESET) {
        WDT_CLR();
    }

    return true;
}

#endif // UART_SPEED_TEST_EN

#include "include.h"
#include "uart_ch340_test.h"

#if UART_CH340_TEST_EN

#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_clk.h"

/*************************** 测试定位 ***************************
 * 单板普通串口(UART1)接 PC 端 CH340 的收发极限测试(不双板):
 *   - 硬件: PA0(TX)->CH340 RXD, PA1(RX)<-CH340 TXD, 共地
 *   - PC 脚本(D:\Code\work\5766\ch340_uart_test.ps1)为时间主控, 每档:
 *       发 SET 命令(10B, 旧波特率) -> 双方换档 -> PC 发 55AA 流 2.5s(板子收流校验)
 *       -> 板子发 55AA 流 2.5s(PC 收流校验)
 *   - 板子循环执行"等 SET -> 换档 -> RX 3.5s -> 报告 -> TX 2.5s", 无档位表(由 PC 决定)
 *   - 结果走 PB3 调试口; PC 脚本自己统计发送/接收两侧
 *
 * 设计要点:
 *   - UART 时钟源固定晶振 X24M 不切换; 主频 SYS_120M 为高波特率中断留 CPU 余量
 *   - 收流校验: 锁定 55/AA 交替相位; 失配时按"丢失字节导致相位滑移"处理, 从实际流重新对齐
 *   - 60s 无 SET 自动回 115200 待机, 支持不复位直接重跑脚本
 **************************************************************/
#define C_TX_PIN                    GPIO_PA0
#define C_RX_PIN                    GPIO_PA1

#define C_RING_SIZE                 256         //必须是 2 的幂
#define C_RX_WINDOW_MS              3500        //收流窗口(须覆盖 PC 的 SET+换挡+2.5s 流, 含串口重开耗时)
#define C_TX_WINDOW_MS              2500        //发流窗口(与 PC 收流窗口对齐)
#define C_IDLE_BACKOFF_MS           60000       //无 SET 超时回 115200 待机

typedef struct {
    volatile u16 head;
    volatile u16 tail;
    u8 buf[C_RING_SIZE];
} c_ring_t;

static c_ring_t c_ring AT(.buf.le_dut.ch340);
static volatile u32 c_drop;

static volatile u32 c_rx_total;         //收流统计(当前档)
static volatile u32 c_rx_good;
static volatile u32 c_rx_bad;

static u32 c_baud;                      //当前波特率

/*************************** 接收环形缓冲 ***************************/

static void c_ring_reset(void)
{
    c_ring.head = 0;
    c_ring.tail = 0;
    c_drop = 0;
}

static void c_ring_push(u8 byte)
{
    u16 next = (c_ring.head + 1) & (C_RING_SIZE - 1);

    if (next != c_ring.tail) {
        c_ring.buf[c_ring.head & (C_RING_SIZE - 1)] = byte;
        c_ring.head = next;
    } else {
        c_drop++;
    }
}

AT(.com_text.isr)
static void c_uart1_isr(void)
{
    if (uart_get_flag(UART1_REG, UART_IT_RX) != RESET) {
        u8 byte = uart_receive_data(UART1_REG);

        uart_clear_flag(UART1_REG, UART_IT_RX);
        c_ring_push(byte);
    }
}

/*************************** UART1 初始化(原厂 driver 库, 全程 X24M) ***************************/

static void c_uart_init(u32 baud)
{
    gpio_init_typedef gpio_init_structure;
    uart_init_typedef uart_init_struct;

    c_ring_reset();
    c_rx_total = 0;
    c_rx_good = 0;
    c_rx_bad = 0;

    clk_gate0_cmd(CLK_GATE0_UART1, CLK_EN);

    gpio_init_structure.gpio_pin = GPIO_PIN_GET(C_RX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
    gpio_init(GPIO_PORT_GET(C_RX_PIN), &gpio_init_structure);

    gpio_init_structure.gpio_pin = GPIO_PIN_GET(C_TX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(GPIO_PORT_GET(C_TX_PIN), &gpio_init_structure);

    gpio_func_mapping_config(GPIO_PORT_GET(C_RX_PIN), GPIO_PIN_GET(C_RX_PIN), GPIO_CROSSBAR_IN_UART1RX);
    gpio_func_mapping_config(GPIO_PORT_GET(C_TX_PIN), GPIO_PIN_GET(C_TX_PIN), GPIO_CROSSBAR_OUT_UART1TX);

    uart_init_struct.baud = baud;
    uart_init_struct.mode = UART_SEND_RECV_MODE;
    uart_init_struct.word_len = UART_WORD_LENGTH_8b;
    uart_init_struct.stop_bits = UART_STOP_BIT_1BIT;
    uart_init_struct.one_line_enable = UART_SEPARATE;
    uart_init_struct.baud_fix = UART_BAUD_FIX;
    uart_init(UART1_REG, &uart_init_struct);

    uart_pic_config(UART1_REG, c_uart1_isr, 0, UART_IT_RX, ENABLE);
    uart_cmd(UART1_REG, ENABLE);

    c_baud = baud;
}

static void c_uart_deinit(void)
{
    uart_pic_config(UART1_REG, NULL, 0, UART_IT_RX, DISABLE);
    uart_cmd(UART1_REG, DISABLE);
    clk_gate0_cmd(CLK_GATE0_UART1, CLK_DIS);
}

/*************************** SET 命令扫描 ***************************
 * 帧格式: [A5][5A][baud u32 LE][~baud u32 LE], 共 10 字节, 校验 = 取反对
 **************************************************************/
enum {
    CSET_M0 = 0, CSET_M1, CSET_B0, CSET_B1, CSET_B2, CSET_B3,
    CSET_X0, CSET_X1, CSET_X2, CSET_X3, CSET_CNT
};

static u8 c_set_idx;                    //当前 SET 匹配进度
static u32 c_set_baud;                 //已解析出的目标波特率

//喂一个字节进扫描状态机; 返回 true = 解析出一帧完整 SET
static bool c_set_feed(u8 byte)
{
    switch (c_set_idx) {
    case CSET_M0:
        if (byte == 0xA5) {
            c_set_idx = CSET_M1;
        }
        break;

    case CSET_M1:
        if (byte == 0x5A) {
            c_set_idx = CSET_B0;
        } else if (byte != 0xA5) {
            c_set_idx = CSET_M0;
        }
        break;

    case CSET_B0:
    case CSET_B1:
    case CSET_B2:
    case CSET_B3:
        c_set_baud |= ((u32)byte << ((c_set_idx - CSET_B0) * 8));
        c_set_idx++;
        break;

    case CSET_X0:
    case CSET_X1:
    case CSET_X2:
    case CSET_X3:
        if (byte != (u8)~((c_set_baud >> ((c_set_idx - CSET_X0) * 8)) & 0xFF)) {
            //失配重新扫描; 0xA5 视为新帧首字节(已消费)
            c_set_baud = 0;
            c_set_idx = (byte == 0xA5) ? CSET_M1 : CSET_M0;
        } else {
            c_set_idx++;
            if (c_set_idx == CSET_CNT) {
                c_set_idx = CSET_M0;
                return true;
            }
        }
        break;

    default:
        c_set_idx = CSET_M0;
        break;
    }

    return false;
}

/*************************** 测试主流程 ***************************/

//收流窗口: 统计总量 + 55/AA 交替校验(相位滑移时从实际流重新对齐)
static void c_rx_phase(void)
{
    u32 t0 = tick_get();
    bool sync = false;
    u8 exp = 0x55;

    while (!tick_check_expire(t0, C_RX_WINDOW_MS)) {
        WDT_CLR();
        msg_dequeue();

        while (c_ring.tail != c_ring.head) {
            u8 byte = c_ring.buf[c_ring.tail & (C_RING_SIZE - 1)];

            c_ring.tail = (c_ring.tail + 1) & (C_RING_SIZE - 1);
            c_rx_total++;

            if (!sync) {
                if (byte == 0x55) {
                    sync = true;
                    exp = 0xAA;
                    c_rx_good++;
                } else if (byte == 0xAA) {
                    sync = true;
                    exp = 0x55;
                    c_rx_good++;
                } else {
                    c_rx_bad++;
                }
            } else if (byte == exp) {
                c_rx_good++;
                exp = (u8)(exp ^ 0xFF);
            } else {
                c_rx_bad++;
                exp = (u8)(byte ^ 0xFF);        //按相位滑移重新对齐
            }
        }
    }
}

//发流窗口: 55/AA 交替, 尽快发
static u32 c_tx_phase(void)
{
    u32 t0 = tick_get();
    u32 sent = 0;
    u8 byte = 0x55;

    while (!tick_check_expire(t0, C_TX_WINDOW_MS)) {
        u32 wait = 0;

        WDT_CLR();
        msg_dequeue();

        uart_send_data(UART1_REG, byte);
        while (uart_get_flag(UART1_REG, UART_IT_TX) != SET) {
            if (++wait > 200000) {
                break;
            }
        }
        byte = (u8)(byte ^ 0xFF);
        sent++;
    }

    return sent;
}

void uart_ch340_test(void)
{
    static bool test_finished;
    u32 t_idle;

    if (test_finished) {
        return;
    }
    test_finished = true;

    printf("%s\n", __func__);

    msg_queue_clear();
    lowpwr_pwroff_auto_dis();               //测试态禁自动关机(对齐双板测试)

    //UART 时钟源固定晶振 X24M(双板实验已证时钟源对工作区间无影响);
    //主频提档不影响通讯(波特率用独立晶振源), 仅为高波特率中断留 CPU 余量
    sys_clk_set(SYS_120M);
    printf("[CH340] ===== single-board UART1 vs CH340 =====\n");
    printf("[CH340] wire: PA0->CH340.RXD, PA1<-CH340.TXD, GND-GND\n");
    printf("[CH340] run PC script: ch340_uart_test.ps1\n");

    c_set_idx = CSET_M0;
    c_set_baud = 0;
    c_uart_init(115200);
    t_idle = tick_get();
    printf("[CH340] wait SET @115200...\n");

    while (1) {
        bool got_set = false;

        WDT_CLR();
        msg_dequeue();

        //扫描 SET 命令
        while (c_ring.tail != c_ring.head) {
            u8 byte = c_ring.buf[c_ring.tail & (C_RING_SIZE - 1)];

            c_ring.tail = (c_ring.tail + 1) & (C_RING_SIZE - 1);
            if (c_set_feed(byte)) {
                got_set = true;
                break;
            }
        }

        if (got_set) {
            u32 new_baud = c_set_baud;
            u32 sent;

            c_set_baud = 0;
            printf("[CH340] SET %u <- %u\n", new_baud, c_baud);
            c_uart_deinit();
            c_uart_init(new_baud);

            //RX 阶段: PC 的 SET+换挡+2.5s 流都落在这 3.5s 窗口内
            c_rx_phase();
            printf("[CH340] %u RX: total=%u good=%u bad=%u drop=%u\n",
                   c_baud, c_rx_total, c_rx_good, c_rx_bad, c_drop);

            //TX 阶段: 与 PC 的收流窗口(3.2s)对齐
            sent = c_tx_phase();
            printf("[CH340] %u TX: sent=%u\n", c_baud, sent);

            t_idle = tick_get();
        }

        //长时间无 SET(脚本结束/未启动)回 115200 待机, 支持直接重跑脚本
        if (tick_check_expire(t_idle, C_IDLE_BACKOFF_MS)) {
            if (c_baud != 115200) {
                printf("[CH340] idle %ds, back to 115200\n", C_IDLE_BACKOFF_MS / 1000);
                c_uart_deinit();
                c_uart_init(115200);
            }
            t_idle = tick_get();
        }
    }
}

#endif // UART_CH340_TEST_EN

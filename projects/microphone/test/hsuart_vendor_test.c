#include "include.h"
#include "hsuart_vendor_test.h"
#include "driver_clk.h"

#if UART_HSUART_VENDOR_TEST_EN

//---- 统一调库: 原厂 bsp 层源文件在本编译单元内真实编译 ----
//bsp_huart.c 被工程宏 LE_DUT_UART_EN 关闭(内容从未参与链接), 无法单独打开;
//此处在包含前把门宏置 1, 原厂代码原封不动编译进本 TU, 测试只调用原厂 API
//(bsp_huart_init/bsp_huart_tx/bsp_huart_wait_txdone/huart_set_baudrate)。
//不影响其他编译单元(它们的 LE_DUT_UART_EN 仍为 0)。
#undef LE_DUT_UART_EN
#define LE_DUT_UART_EN 1
#include "bsp_huart.c"

/*************************** 测试定位 ***************************
 * HSUART 测试完全调用原厂库函数(统一调库, 不手写初始化/收发序列):
 *   bsp_huart_init(&cfg)      初始化(gpio/交叉开关/hsuart_init/空闲阈值/中断/收发使能/RX DMA 一条龙)
 *   bsp_huart_tx(buf, len)    发送(任意长度 DMA, 寄存器即 DMA)
 *   bsp_huart_wait_txdone()   等发送完成(buf 复用前必调, 原厂铁律 3)
 *   huart_set_baudrate(baud)  波特率切换(驱动按当前时钟源自动算分频)
 *   RX: bsp_huart_init 注册回调, 原厂 ISR 完成回收调 -> 清标志 -> 重挂同一块 rxbuf
 *
 * 原厂方法对照实验结论(2026-09-16 双板实测, 详见 docs/test/高速串口测试.md):
 *   1. 时钟源不动(复位默认): 双板寄存器都报 idx=0/24M, 但 hello(5B 短帧)通、
 *      数据块(257B 连续流)两档全 LRC 错 —— 复现旧踩坑 2, 与旧测试结论一致:
 *      AB5766 双板必须显式 clk_hsut0_clk_set(CLK_HSUT0_XOSC24M)。
 *      本文件用 VTEST_FORCE_XOSC24M 开关控制(原厂时钟库函数), 置 0 可复现故障。
 *   2. huart_putchar 直写 data 寄存器: TX-DMA 模式下一字节都发不出去
 *      (双板 raw=0, 复现旧踩坑 3), 该路径在本芯片不可用, 发送统一走 bsp_huart_tx。
 *   3. RX 单缓冲同址重挂: 实测确认(XOSC24M 下 hello 通、连续数据块全 LRC 错,
 *      与旧测试 ping-pong 双缓冲全 PASS 形成直接对照) —— 原厂文档 §3.4 的 ISR
 *      顺序固定为 回调->清标志->重挂 rxbuf, 本测试在回调里把重挂目标切到
 *      另一块缓冲实现 ping-pong, 消除 DMA 盲区, 不改原厂任何代码。
 *
 * 接线/角色/帧协议/波特率表与旧测试相同, 结果可直接对照。
 **************************************************************/
#define VTEST_TX_PIN                GPIO_PA0
#define VTEST_RX_PIN                GPIO_PA1

#define VTEST_FORCE_XOSC24M         1           //1=用原厂库函数显式选晶振(双板必需), 0=复位默认(复现全错)

#define V_BAUD_BASE                 115200      //握手基础波特率
#define V_WINDOW_MS                 2000        //每档发送窗口
#define V_HELLO_TIMEOUT_MS          15000       //主机等握手总超时
#define V_RSP_TIMEOUT_MS            1000        //单次应答超时
#define V_SLAVE_RESYNC_MS           8000        //从机空闲重同步(回 115200)

//与旧测试相同的爬坡表
static const u32 v_baud_tbl[] = {460800, 921600, 1500000};
#define V_BAUD_CNT                  (sizeof(v_baud_tbl) / sizeof(v_baud_tbl[0]))

/*************************** 帧协议(与旧测试相同) ***************************
 * 帧格式: [A5][5A][cmd][len][payload(len)][lrc], lrc = cmd^len^payload 异或
 * 数据块也是帧: cmd=CMD_DATA, len=252, payload[0]=seq, 整帧 257B
 **************************************************************/
#define FRM_MAGIC0                  0xA5
#define FRM_MAGIC1                  0x5A

enum {
    CMD_HELLO_REQ = 0x01,
    CMD_HELLO_RSP = 0x02,
    CMD_SET_BAUD  = 0x03,           //payload = u32 LE 波特率
    CMD_BAUD_ACK  = 0x04,
    CMD_DATA      = 0x10,           //数据块
    CMD_STATS_REQ = 0x20,
    CMD_STATS_RSP = 0x21,           //payload 14B: u32 good + u32 miss + u16 blk_ok + u16 blk_crc + u16 drop
};

#define BLOCK_PAYLOAD               252
#define FRM_OVERHEAD                5
#define BLOCK_TOTAL                 (FRM_OVERHEAD + BLOCK_PAYLOAD)
#define STATS_PAYLOAD_LEN           14

/*************************** 缓冲布局 ***************************
 * .buf.le_dut.vtest 段与旧测试的 .buf.le_dut.uart_test 同落 comm_test 区
 * (0x17800~0x18000 共 2KB): 旧测试已占约 1.3KB, 本测试只放约 420B;
 * 数据块缓冲放普通 .bss(旧测试已验证 TX DMA 可访问 .bss 地址)。
 **************************************************************/
#define V_RING_SIZE                 256         //必须是 2 的幂
#define V_DMA_RXBUF_SIZE            128         //原厂 RX 单缓冲(le_dut 用例同 128B)

typedef struct {
    volatile u16 head;
    volatile u16 tail;
    u8 buf[V_RING_SIZE];
} v_ring_t;

static v_ring_t v_ring AT(.buf.le_dut.vtest);
static u8 v_dma_buf[2][V_DMA_RXBUF_SIZE] AT(.buf.le_dut.vtest); //RX ping-pong 双缓冲
static volatile u8 v_fmark;             //bit0/bit1: 对应缓冲已填满待主循环搬运
static volatile u16 v_flen[2];          //各缓冲的实际接收长度(fifo_cnt)
static u8 v_small_buf[32] AT(.buf.le_dut.vtest);                //主机帧解析缓冲(只收小帧)
static u8 v_blk_buf[BLOCK_TOTAL];                               //主机=DMA 发送源 / 从机=帧解析缓冲

static volatile u32 v_ring_drop;
static volatile u32 v_raw_total;                                //收到总字节(RAW 诊断用)
static volatile u8 v_raw_last[12];
static volatile u32 v_lrc_err;

typedef struct {
    u8 *buf;                        //解析缓冲(主机=v_small_buf, 从机=v_blk_buf)
    u16 max;                        //缓冲上限(超长帧直接丢弃重同步)
    u16 idx;
    u16 need;
    bool sync;
} v_parser_t;
static v_parser_t v_parser;

typedef struct {
    volatile bool hello_req;
    volatile bool hello_rsp;
    volatile bool set_baud;
    volatile bool baud_ack;
    volatile bool stats_req;
    volatile bool stats_ready;
    u32 set_baud_val;
    u8 stats_pl[STATS_PAYLOAD_LEN];
} v_flags_t;
static v_flags_t v_f;

typedef struct {
    u32 good;                       //正确接收的 payload 字节
    u32 miss;                       //seq 断档丢失块数
    u32 blk_ok;
    u32 blk_crc;                    //lrc 错帧数
    u8 last_seq;
    bool seq_valid;
} v_stat_t;
static v_stat_t v_stat;

static bool is_master;

/*************************** 接收: 原厂 ISR + 原厂回调铁律 ***************************/

static void v_ring_push(u8 byte)
{
    u16 next = (v_ring.head + 1) & (V_RING_SIZE - 1);

    if (next != v_ring.tail) {
        v_ring.buf[v_ring.head] = byte;
        v_ring.head = next;
    } else {
        v_ring_drop++;
    }
}

//原厂回调(huart_rx_isr_t)。原厂 ISR 顺序固定为 回调->清标志->重挂 rxbuf(文档 §3.4),
//回调执行期间 RX DMA 处于盲区 —— 在回调里逐字节搬运会把盲区拉宽到百微秒级,
//连续流下每个 DMA 周期丢几个字节(实测: 257B 数据块全 LRC 错)。
//因此回调只做微秒级的 3 件事: 记录缓冲号/长度 + 把原厂重挂目标切到另一块;
//搬运转主循环 v_poll_rx() 做(另一块缓冲填满前有 128 字节时间的裕量)。
static void v_rx_callback(u8 *rxbuf, u16 rx_buf_len)
{
    u8 idx = (rxbuf == v_dma_buf[0]) ? 0 : 1;

    v_flen[idx] = rx_buf_len;
    v_fmark |= (u8)(1u << idx);
    v_raw_total += rx_buf_len;
    bsp_hsuart_str.rxbuf = (idx == 0) ? v_dma_buf[1] : v_dma_buf[0];
}

/*************************** 发送/初始化: 全部原厂库函数 ***************************/

static bsp_hsuart_cfg_t v_cfg;

static void v_hw_init(u32 baud)
{
    v_ring.head = 0;
    v_ring.tail = 0;
    v_ring_drop = 0;
    v_raw_total = 0;
    v_lrc_err = 0;
    memset((u8 *)&v_f, 0, sizeof(v_f));
    memset((u8 *)&v_stat, 0, sizeof(v_stat));

#if VTEST_FORCE_XOSC24M
    //原厂时钟库函数。不设则双板数据块全 LRC 错(两次实测复现), 详见文件头结论 1
    clk_hsut0_clk_set(CLK_HSUT0_XOSC24M);
#endif
    printf("[VT] clk_hsut0 idx=%d freq=%d\n",
           (int)clk_hsut0_clk_get(CLK_VALUE_MODE_IDX), (int)clk_hsut0_clk_get(CLK_VALUE_MODE_FREQ));

    v_cfg.tx_gpio_pin = VTEST_TX_PIN;
    v_cfg.rx_gpio_pin = VTEST_RX_PIN;
    v_cfg.baud = baud;
    v_cfg.rx_isr_en = 1;
    v_cfg.rx_buf_ptr = v_dma_buf[0];
    v_cfg.rx_buf_len = V_DMA_RXBUF_SIZE;
    v_cfg.isr = v_rx_callback;
    v_cfg.pu_pd_en = 0;

    bsp_huart_init(&v_cfg);             //原厂库: 一条龙初始化
}

//波特率切换: 原厂库函数(驱动按当前时钟源自动算分频)
static void v_set_baud(u32 baud)
{
    huart_set_baudrate(baud);
}

//小帧发送: 同样走原厂 bsp_huart_tx DMA(任意长度), 发完等 TXPND(原厂铁律 3)
static u8 v_tx_frm[FRM_OVERHEAD + STATS_PAYLOAD_LEN];

static void v_send_frame(u8 cmd, const u8 *pl, u8 len)
{
    u8 lrc = (u8)(cmd ^ len);
    u8 i;

    v_tx_frm[0] = FRM_MAGIC0;
    v_tx_frm[1] = FRM_MAGIC1;
    v_tx_frm[2] = cmd;
    v_tx_frm[3] = len;
    for (i = 0; i < len; i++) {
        lrc ^= pl[i];
        v_tx_frm[4 + i] = pl[i];
    }
    v_tx_frm[FRM_OVERHEAD + len - 1] = lrc;

    bsp_huart_tx(v_tx_frm, FRM_OVERHEAD + len);
    bsp_huart_wait_txdone();
}

/*************************** 帧解析(主循环侧) ***************************/

static void v_parser_reset(void)
{
    v_parser.idx = 0;
    v_parser.need = 0;
    v_parser.sync = false;
}

static void v_dispatch(const u8 *f, u16 total)
{
    u8 cmd = f[2];
    u8 len = f[3];
    const u8 *pl = &f[4];
    u8 lrc = (u8)(cmd ^ len);
    u8 i;

    for (i = 0; i < len; i++) {
        lrc ^= pl[i];
    }
    if (lrc != f[total - 1]) {
        v_lrc_err++;
        return;
    }

    switch (cmd) {
    case CMD_HELLO_REQ:
        v_f.hello_req = true;
        break;

    case CMD_HELLO_RSP:
        v_f.hello_rsp = true;
        break;

    case CMD_SET_BAUD:
        memcpy((u8 *)&v_f.set_baud_val, pl, 4);
        v_f.set_baud = true;
        break;

    case CMD_BAUD_ACK:
        v_f.baud_ack = true;
        break;

    case CMD_DATA: {
        u8 seq = pl[0];

        if (v_stat.seq_valid && (seq != (u8)(v_stat.last_seq + 1))) {
            v_stat.miss += (u8)(seq - (u8)(v_stat.last_seq + 1));
        }
        v_stat.last_seq = seq;
        v_stat.seq_valid = true;
        v_stat.blk_ok++;
        v_stat.good += len;
        break;
    }

    case CMD_STATS_REQ:
        v_f.stats_req = true;
        break;

    case CMD_STATS_RSP:
        memcpy(v_f.stats_pl, pl, STATS_PAYLOAD_LEN);
        v_f.stats_ready = true;
        break;

    default:
        break;
    }
}

static void v_parser_feed(u8 byte)
{
    v_parser_t *p = &v_parser;

    if (!p->sync) {
        if (byte == FRM_MAGIC0) {
            p->buf[0] = byte;
            p->idx = 1;
            p->sync = true;
        }
        return;
    }

    if (p->idx == 1) {
        if (byte == FRM_MAGIC1) {
            p->buf[p->idx++] = byte;
        } else if (byte != FRM_MAGIC0) {
            p->sync = false;
        }
    } else if (p->idx == 2) {
        //cmd 字节: need 尚未有效, 只存储
        p->buf[p->idx++] = byte;
    } else if (p->idx == 3) {
        if ((u16)(FRM_OVERHEAD + byte) > p->max) {      //超本端缓冲的帧直接丢弃重同步
            v_parser_reset();
            if (byte == FRM_MAGIC0) {
                p->buf[0] = byte;
                p->idx = 1;
                p->sync = true;
            }
            return;
        }
        p->buf[p->idx++] = byte;
        p->need = FRM_OVERHEAD + byte;
    } else {
        p->buf[p->idx++] = byte;
        if (p->idx >= p->need) {
            v_dispatch(p->buf, p->need);
            v_parser_reset();
        }
    }
}

static void v_poll_rx(void)
{
    //搬运已填满的 DMA 缓冲(两块都置位时先搬 0 号 = 更早填满的, 保证字节序)
    while (v_fmark != 0) {
        u8 idx = (v_fmark & 1u) ? 0 : 1;
        u16 len = v_flen[idx];
        u16 i;

        for (i = 0; i < len; i++) {
            v_raw_last[i % (sizeof(v_raw_last))] = v_dma_buf[idx][i];
            v_ring_push(v_dma_buf[idx][i]);
        }
        v_fmark &= (u8)~(1u << idx);
    }

    while (v_ring.tail != v_ring.head) {
        u8 byte = v_ring.buf[v_ring.tail];

        v_ring.tail = (v_ring.tail + 1) & (V_RING_SIZE - 1);
        v_parser_feed(byte);
    }
}

static void v_raw_print(const char *tag)
{
    u8 i;

    printf("%s raw=%u last:", tag, v_raw_total);
    for (i = 0; i < sizeof(v_raw_last); i++) {
        printf(" %02x", v_raw_last[i]);
    }
    printf("\n");
}

/*************************** 主机流程 ***************************/

//带喂狗的延时: delay_ms 不喂看门狗, 超过 WDT 周期的延时会直接复位(实测踩过)
static void v_delay_ms_wdt(u32 ms)
{
    u32 t0 = tick_get();

    while (!tick_check_expire(t0, ms)) {
        WDT_CLR();
        msg_dequeue();
    }
}

static u32 v_rd32(const u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static void v_wr32(u8 *p, u32 v)
{
    p[0] = (u8)(v);
    p[1] = (u8)(v >> 8);
    p[2] = (u8)(v >> 16);
    p[3] = (u8)(v >> 24);
}

static bool master_hello(void)
{
    u32 t0 = tick_get();
    u32 last_req = 0;

    while (!tick_check_expire(t0, V_HELLO_TIMEOUT_MS)) {
        WDT_CLR();
        msg_dequeue();
        v_poll_rx();
        if ((last_req == 0) || tick_check_expire(last_req, 500)) {
            last_req = tick_get();
            v_send_frame(CMD_HELLO_REQ, NULL, 0);
        }
        if (v_f.hello_rsp) {
            v_f.hello_rsp = false;
            return true;
        }
    }

    printf("[VT][M] ERROR: no HELLO_RSP in %ds!\n", V_HELLO_TIMEOUT_MS / 1000);
    v_raw_print("[VT][M]");
    return false;
}

//发 SET_BAUD -> 等旧波特率下的 BAUD_ACK -> 双方再各自切换
static bool master_switch_baud(u32 baud)
{
    u8 pl[4];
    u8 retry;

    v_wr32(pl, baud);
    for (retry = 0; retry < 3; retry++) {
        u32 t0 = tick_get();

        v_f.baud_ack = false;
        v_send_frame(CMD_SET_BAUD, pl, 4);
        while (!tick_check_expire(t0, V_RSP_TIMEOUT_MS)) {
            WDT_CLR();
            msg_dequeue();
            v_poll_rx();
            if (v_f.baud_ack) {
                v_set_baud(baud);       //原厂 huart_set_baudrate
                v_delay_ms_wdt(100);
                printf("[VT][M] baud -> %u\n", baud);
                return true;
            }
        }
    }

    printf("[VT][M] ERROR: no BAUD_ACK for %u\n", baud);
    return false;
}

//2 秒连续数据块: 填 buf -> bsp_huart_tx -> 等完成(原厂铁律: buf 复用前必须等 TXPND)
static void master_stream(u32 *sent_bytes, u32 *sent_blocks, u32 *out_ms)
{
    u32 t0 = tick_get();
    u32 sent = 0;
    u32 blocks = 0;
    u8 seq = 1;

    while (!tick_check_expire(t0, V_WINDOW_MS)) {
        u16 i;
        u8 lrc = (u8)(CMD_DATA ^ BLOCK_PAYLOAD);

        WDT_CLR();
        msg_dequeue();

        v_blk_buf[0] = FRM_MAGIC0;
        v_blk_buf[1] = FRM_MAGIC1;
        v_blk_buf[2] = CMD_DATA;
        v_blk_buf[3] = BLOCK_PAYLOAD;
        lrc ^= (u8)seq;
        v_blk_buf[4] = (u8)seq;
        for (i = 5; i < BLOCK_TOTAL - 1; i++) {
            v_blk_buf[i] = (u8)(i * 7 + seq);
            lrc ^= v_blk_buf[i];
        }
        v_blk_buf[BLOCK_TOTAL - 1] = lrc;

        bsp_huart_tx(v_blk_buf, BLOCK_TOTAL);
        bsp_huart_wait_txdone();

        sent += BLOCK_PAYLOAD;
        blocks++;
        seq++;
    }

    *sent_bytes = sent;
    *sent_blocks = blocks;
    *out_ms = tick_get() - t0;
}

static bool master_stats(u32 *good, u32 *miss, u32 *blk_ok, u32 *blk_crc, u32 *drop)
{
    u8 retry;

    for (retry = 0; retry < 3; retry++) {
        u32 t0 = tick_get();

        v_f.stats_ready = false;
        v_send_frame(CMD_STATS_REQ, NULL, 0);
        while (!tick_check_expire(t0, V_RSP_TIMEOUT_MS)) {
            WDT_CLR();
            msg_dequeue();
            v_poll_rx();
            if (v_f.stats_ready) {
                *good = v_rd32(&v_f.stats_pl[0]);
                *miss = v_rd32(&v_f.stats_pl[4]);
                *blk_ok = (u32)(v_f.stats_pl[8] | (v_f.stats_pl[9] << 8));
                *blk_crc = (u32)(v_f.stats_pl[10] | (v_f.stats_pl[11] << 8));
                *drop = (u32)(v_f.stats_pl[12] | (v_f.stats_pl[13] << 8));
                return true;
            }
        }
    }

    printf("[VT][M] ERROR: no STATS_RSP\n");
    return false;
}

static void master_flow(void)
{
    u32 best_baud = 0;
    u32 best_thr = 0;
    u8 fail_streak = 0;
    u8 bi;

    if (!master_hello()) {
        return;
    }
    printf("[VT][M] hello ok\n");

    for (bi = 0; bi < V_BAUD_CNT; bi++) {
        u32 baud = v_baud_tbl[bi];
        u32 sent, blocks, ms, good, miss, blk_ok, blk_crc, drop;
        bool pass;

        if (!master_switch_baud(baud)) {
            break;
        }

        master_stream(&sent, &blocks, &ms);
        if (!master_stats(&good, &miss, &blk_ok, &blk_crc, &drop)) {
            break;
        }

        pass = ((miss == 0) && (blk_crc == 0) && (drop == 0));
        printf("[VT][M] %u: sent=%u(%u blk)/%ums good=%u miss=%u crc=%u drop=%u %s\n",
               baud, sent, blocks, ms, good, miss, blk_crc, drop, pass ? "PASS" : "FAIL");

        if (pass) {
            best_baud = baud;
            best_thr = good * 1000 / ms;
            fail_streak = 0;
        } else {
            if (blk_crc >= blk_ok) {
                printf("[VT][M] (all LRC bad -> clk offset or RX single-buf rearm gap)\n");
            } else if (miss + drop > 0) {
                printf("[VT][M] (miss/drop>0 -> RX rearm gap or ring overflow)\n");
            }
            fail_streak++;
            if (fail_streak >= 2) {
                printf("[VT][M] 2 consecutive fails, stop ramp\n");
                break;
            }
        }
    }

    if (best_baud != 0) {
        printf("[VT][M] SUMMARY: bsp-library HSUART max clean baud=%u thr=%u B/s\n", best_baud, best_thr);
    } else {
        printf("[VT][M] SUMMARY: NO clean baud\n");
    }
}

/*************************** 从机流程 ***************************/

static void slave_raw_watch(u32 *last_total, u32 *t_raw)
{
    if (v_raw_total != *last_total) {
        *last_total = v_raw_total;
        *t_raw = tick_get();
        if ((*last_total & 0x3f) == 0) {        //每 64 字节打印一次, 防刷屏
            v_raw_print("[VT][S]");
        }
    }
}

static void slave_flow(void)
{
    u32 t_sync = tick_get();
    u32 last_total = 0;
    u32 t_raw = tick_get();

    printf("[VT][S] wait HELLO_REQ @%u\n", V_BAUD_BASE);
    while (1) {
        WDT_CLR();
        msg_dequeue();
        v_poll_rx();

        if (v_f.hello_req) {
            v_f.hello_req = false;
            v_send_frame(CMD_HELLO_RSP, NULL, 0);
            printf("[VT][S] HELLO_REQ got, RSP sent\n");
            break;
        }

        slave_raw_watch(&last_total, &t_raw);

        //从机空闲重同步: 长时间无主机则回基础波特率(测试辅助)
        if (tick_check_expire(t_sync, V_SLAVE_RESYNC_MS)) {
            printf("[VT][S] idle, re-init @%u\n", V_BAUD_BASE);
            v_hw_init(V_BAUD_BASE);
            t_sync = tick_get();
        }
    }

    //测试主循环: 换波特率 / 收数据块 / 回统计
    while (1) {
        WDT_CLR();
        msg_dequeue();
        v_poll_rx();

        if (v_f.set_baud) {
            u32 baud = v_f.set_baud_val;

            v_f.set_baud = false;
            v_send_frame(CMD_BAUD_ACK, NULL, 0);        //旧波特率下应答, 之后双方再切换
            v_set_baud(baud);
            memset((u8 *)&v_stat, 0, sizeof(v_stat));
            v_lrc_err = 0;
            printf("[VT][S] baud -> %u\n", baud);
        }

        if (v_f.stats_req) {
            u8 pl[STATS_PAYLOAD_LEN];

            v_f.stats_req = false;
            v_wr32(&pl[0], v_stat.good);
            v_wr32(&pl[4], v_stat.miss);
            pl[8] = (u8)(v_stat.blk_ok);
            pl[9] = (u8)(v_stat.blk_ok >> 8);
            pl[10] = (u8)(v_lrc_err);
            pl[11] = (u8)(v_lrc_err >> 8);
            pl[12] = (u8)(v_ring_drop);
            pl[13] = (u8)(v_ring_drop >> 8);
            v_send_frame(CMD_STATS_RSP, pl, STATS_PAYLOAD_LEN);
            printf("[VT][S] stats: good=%u miss=%u crc=%u drop=%u\n",
                   v_stat.good, v_stat.miss, v_lrc_err, v_ring_drop);
        }
    }
}

/*************************** 测试入口 ***************************/

void hsvendor_uart_test(void)
{
    static bool test_finished;              //防止 func_run 大循环重复进入重跑测试

    if (test_finished) {
        return;
    }
    test_finished = true;

    sys_clk_set(SYS_120M);
    printf("[VT] ===== HSUART bsp-library test =====\n");
    printf("[VT] calls: bsp_huart_init/tx/wait_txdone + huart_set_baudrate\n");
    printf("[VT] rx: callback-swap ping-pong, copy deferred to main loop\n");
#if VTEST_FORCE_XOSC24M
    printf("[VT] clk: XOSC24M via clk_hsut0_clk_set (VTEST_FORCE_XOSC24M=1)\n");
#else
    printf("[VT] clk: reset default (VTEST_FORCE_XOSC24M=0)\n");
#endif

    is_master = wireless_role_is_adapter();
    printf("[VT] role=%s (adapter=master)\n", is_master ? "MASTER" : "SLAVE");

    printf("[VT] start in 3s...\n");
    v_delay_ms_wdt(1000);
    printf("[VT] 2...\n");
    v_delay_ms_wdt(1000);
    printf("[VT] 1...\n");
    v_delay_ms_wdt(1000);

    v_parser.buf = is_master ? v_small_buf : v_blk_buf;
    v_parser.max = is_master ? sizeof(v_small_buf) : sizeof(v_blk_buf);
    v_parser_reset();

    v_hw_init(V_BAUD_BASE);

    if (is_master) {
        master_flow();
    } else {
        slave_flow();
    }

    while (1) {                             //主机侧测试结束, 停在这里等用户断电
        WDT_CLR();
        msg_dequeue();
        v_poll_rx();
    }
}

#endif // UART_HSUART_VENDOR_TEST_EN

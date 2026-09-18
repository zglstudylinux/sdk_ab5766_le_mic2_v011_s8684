#include "include.h"
#include "hsuart_vendor_test.h"
#include "driver_clk.h"
#include "driver_uart.h"
#include "driver_gpio.h"

#if UART_HSUART_VENDOR_TEST_EN

//---- 统一调库: 原厂 bsp 层源文件在本编译单元内真实编译 ----
//bsp_huart.c 被工程宏 LE_DUT_UART_EN 关闭(内容从未参与链接), 无法单独打开;
//此处在包含前把门宏置 1, 原厂代码原封不动编译进本 TU, 测试只调用原厂 API。
//不影响其他编译单元(它们的 LE_DUT_UART_EN 仍为 0)。
#undef LE_DUT_UART_EN
#define LE_DUT_UART_EN 1
#include "bsp_huart.c"

/*************************** 测试定位 ***************************
 * 双板串口极限爬坡(统一调库): 普通串口 UART1 与高速串口 HSUART 分开测试,
 * 编译期由 UART_TEST_PORT_SEL 选择端口(0=UART1, 1=HSUART), 全程不做端口切换。
 *
 * 阶梯从已验证低档起步作锚点(前几档 PASS 即证明环境/代码正常), 再爬新档:
 *   460800 -> 921600 -> 1.5M -> 3M -> 4M -> 6M -> 8M -> 12M
 *   UART1  锚点域: 到 3M(旧测试全 PASS);  HSUART 锚点域: 到 1.5M(bsp 库版零误码)
 *
 * 时钟源按档位选择(原厂 clk 库): <=1.5M 用 X24M(已验证域), >1.5M 用 X48M(新域)。
 * 每档切换统一"停 -> 切时钟 -> 重新 init": SET_BAUD/BAUD_ACK 在旧档协商完成、
 * 双方切换期间无数据在途, 之后各自 deinit -> 切时钟 -> 全新 init(新档)。
 * 不在活体外设上热切时钟(实测会引入毛刺中断/派发异常, EPC=0 崩溃)。
 *
 * HSUART 全套沿用已验证组件: bsp_huart_init + 回调切缓冲 + 延迟搬运
 * (原厂代码一行未改, 460800/921600/1.5M 零误码 PASS 版本)。
 * UART1 沿用原厂 driver 库 + 逐字节 RX 中断(无 DMA, 高档位 RX 中断风暴是已知风险)。
 *
 * 接线: A.PA0->B.PA1 交叉 + 共地; 角色: adapter=主机, mic=从机。
 **************************************************************/
#define VTEST_TX_PIN                GPIO_PA0
#define VTEST_RX_PIN                GPIO_PA1

#define V_X24M_MAX                  1500000     //<=此档用 X24M(已验证域), >此档切 X48M
//实验宏: 1=HSUART 全阶梯强制 X24M(验证 X24M 高档位表现, 用户指定实验), 0=按 V_X24M_MAX 分档
#define VTEST_HS_FORCE_X24M         1
//实验宏: 1=UART1 全阶梯强制 X24M(对照实验, 用户指定), 0=按 V_X24M_MAX 分档
#define VTEST_UART1_FORCE_X24M      1
#define V_BAUD_BASE                 115200      //握手基础波特率
#define V_WINDOW_MS                 2000        //每档发送窗口
#define V_HELLO_TIMEOUT_MS          15000       //主机等握手总超时
#define V_RSP_TIMEOUT_MS            1000        //单次应答超时
#define V_SLAVE_RESYNC_MS           3000        //从机线路空闲回落 115200

//低档锚点 + 12M 阶梯
static const u32 v_baud_tbl[] = {
    460800, 921600, 1500000, 3000000, 4000000, 6000000, 8000000, 12000000
};
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

/*************************** 端口选择(编译期) ***************************/

#define VPORT_UART1                 0
#define VPORT_HS                    1

#if UART_TEST_PORT_SEL
#define VTEST_PORT                  VPORT_HS
#define VTEST_PORT_NAME             "B:HSUART"
#else
#define VTEST_PORT                  VPORT_UART1
#define VTEST_PORT_NAME             "A:UART1"
#endif

/*************************** 缓冲布局 ***************************
 * .buf.le_dut.vtest 段是 NOLOAD(启动不清零)! 其中 v_ring 的 head/tail 等
 * 控制状态必须在使用前显式清零(v_rx_state_reset), 否则野下标访问 -> DQ ERR。
 * 该段 0x17800~0x17A24, 共 0x224B。
 **************************************************************/
#define V_RING_SIZE                 256         //必须是 2 的幂
#define V_DMA_RXBUF_SIZE            128         //HSUART RX 单块(原厂 le_dut 同 128B)

typedef struct {
    volatile u16 head;
    volatile u16 tail;
    u8 buf[V_RING_SIZE];
} v_ring_t;

static v_ring_t v_ring AT(.buf.le_dut.vtest);
static u8 v_dma_buf[2][V_DMA_RXBUF_SIZE] AT(.buf.le_dut.vtest); //HSUART RX ping-pong 双缓冲
static volatile u8 v_fmark;             //bit0/bit1: 对应缓冲已填满待主循环搬运
static volatile u16 v_flen[2];          //各缓冲的实际接收长度(fifo_cnt)
static u8 v_small_buf[32] AT(.buf.le_dut.vtest);                //主机帧解析缓冲(只收小帧)
static u8 v_blk_buf[BLOCK_TOTAL];                               //主机=发送源 / 从机=帧解析缓冲

static volatile u32 v_ring_drop;
static volatile u32 v_raw_total;                                //收到总字节(RAW 诊断/空闲检测用)
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

/*************************** 接收公共: 环形缓冲 + 解析 ***************************/

static void v_ring_push(u8 byte)
{
    u16 next = (v_ring.head + 1) & (V_RING_SIZE - 1);

    if (next != v_ring.tail) {
        v_ring.buf[v_ring.head & (V_RING_SIZE - 1)] = byte;     //防御: head 异常时不野写
        v_ring.head = next;
    } else {
        v_ring_drop++;
    }
}

/*************************** UART1(原厂 driver 库) ***************************/

//接收相关状态复位, 两个端口每次 init 都必须调用!
//注意 .buf.le_dut.vtest 段是 NOLOAD(启动不清零), v_ring 的 head/tail 若不显式清零,
//首个字节就会按垃圾下标访问 -> DQ ERR 总线异常(实测崩溃根因)
static void v_rx_state_reset(void)
{
    v_ring.head = 0;
    v_ring.tail = 0;
    v_ring_drop = 0;
    v_raw_total = 0;
    v_lrc_err = 0;
    v_fmark = 0;
    v_flen[0] = 0;
    v_flen[1] = 0;
    memset((u8 *)&v_f, 0, sizeof(v_f));
    memset((u8 *)&v_stat, 0, sizeof(v_stat));
}

//RX 逐字节中断 -> 环形缓冲(UART1 无 DMA, 高波特率下此 ISR 是瓶颈, 实测记录)
AT(.com_text.isr)
static void v_uart1_isr(void)
{
    if (uart_get_flag(UART1_REG, UART_IT_RX) != RESET) {
        u8 byte = uart_receive_data(UART1_REG);

        uart_clear_flag(UART1_REG, UART_IT_RX);
        v_ring_push(byte);
        v_raw_total++;
    }
}

//init 不碰时钟: boot 时复位默认 XOSC24M 跑 115200 握手;
//阶梯档位的时钟切换由 v_port_set_baud 的"先 deinit 再切时钟"完成
static void v_uart1_init(u32 baud)
{
    gpio_init_typedef gpio_init_structure;
    uart_init_typedef uart_init_struct;

    v_rx_state_reset();

    clk_gate0_cmd(CLK_GATE0_UART1, CLK_EN);

    //RX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(VTEST_RX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_INPUT;
    gpio_init_structure.gpio_fen = GPIO_FEN_PER;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_mode = GPIO_MODE_DIGITAL;
    gpio_init_structure.gpio_pupd = GPIO_PUPD_PU10K;
    gpio_init(GPIO_PORT_GET(VTEST_RX_PIN), &gpio_init_structure);

    //TX
    gpio_init_structure.gpio_pin = GPIO_PIN_GET(VTEST_TX_PIN);
    gpio_init_structure.gpio_dir = GPIO_DIR_OUTPUT;
    gpio_init_structure.gpio_fdir = GPIO_FDIR_SELF;
    gpio_init_structure.gpio_drv = GPIO_DRV_6MA;
    gpio_init(GPIO_PORT_GET(VTEST_TX_PIN), &gpio_init_structure);

    gpio_func_mapping_config(GPIO_PORT_GET(VTEST_RX_PIN), GPIO_PIN_GET(VTEST_RX_PIN), GPIO_CROSSBAR_IN_UART1RX);
    gpio_func_mapping_config(GPIO_PORT_GET(VTEST_TX_PIN), GPIO_PIN_GET(VTEST_TX_PIN), GPIO_CROSSBAR_OUT_UART1TX);

    uart_init_struct.baud = baud;
    uart_init_struct.mode = UART_SEND_RECV_MODE;
    uart_init_struct.word_len = UART_WORD_LENGTH_8b;
    uart_init_struct.stop_bits = UART_STOP_BIT_1BIT;
    uart_init_struct.one_line_enable = UART_SEPARATE;
    uart_init_struct.baud_fix = UART_BAUD_FIX;
    uart_init(UART1_REG, &uart_init_struct);
    printf("[VT] uart1 clk freq=%u\n", (unsigned)clk_uart_clk_get(UART1_REG, CLK_VALUE_MODE_FREQ));

    uart_pic_config(UART1_REG, v_uart1_isr, 0, UART_IT_RX, ENABLE);
    uart_cmd(UART1_REG, ENABLE);
}

static void v_uart1_deinit(void)
{
    //注意: 只禁 RX! TX|RX 全禁会走 driver 的 "PICEN &= ~BIT(IRQn_UART)" 分支,
    //而 deinit->init 在每档换挡热路径上, 重 init 后 RX 中断可能永久失活(第四轮审查 M1)
    uart_pic_config(UART1_REG, NULL, 0, UART_IT_RX, DISABLE);
    uart_cmd(UART1_REG, DISABLE);
    clk_gate0_cmd(CLK_GATE0_UART1, CLK_DIS);
}

//原厂 driver 库逐字节阻塞发送(UART1 无 DMA)
static void v_uart1_tx(const u8 *buf, u16 len)
{
    u16 i;

    for (i = 0; i < len; i++) {
        u32 wait = 0;

        uart_send_data(UART1_REG, buf[i]);
        while (uart_get_flag(UART1_REG, UART_IT_TX) != SET) {
            WDT_CLR();
            if (++wait > 200000) {
                break;
            }
        }
    }
}

/*************************** HSUART(原厂 bsp 库) ***************************/

static bsp_hsuart_cfg_t v_cfg;

//原厂回调(huart_rx_isr_t)。原厂 ISR 顺序固定为 回调->清标志->重挂 rxbuf(文档 §3.4),
//回调执行期间 RX DMA 处于盲区 —— 回调只做微秒级的记录 + 切换重挂目标,
//搬运延迟到主循环 v_poll_rx()(另一块缓冲填满前有 128 字节时间的裕量)。
static void v_rx_callback(u8 *rxbuf, u16 rx_buf_len)
{
    u8 idx = (rxbuf == v_dma_buf[0]) ? 0 : 1;

    v_flen[idx] = rx_buf_len;
    v_fmark |= (u8)(1u << idx);
    v_raw_total += rx_buf_len;
    bsp_hsuart_str.rxbuf = (idx == 0) ? v_dma_buf[1] : v_dma_buf[0];
}

//停 -> 按档位切时钟 -> bsp_huart_init 全新初始化(已验证 PASS 版的调用顺序: clk 在 init 前)
static void v_hs_init(u32 baud)
{
    v_rx_state_reset();

    //寄存器访问前先开 CLK_GATE0_HSUART(bsp_huart_init 内部才会开, 这里更早碰了 con)
    clk_gate0_cmd(CLK_GATE0_HSUART, CLK_EN);
    hsuart_cmd(HSUART_REG, HSUT_TRANSMIT | HSUT_RECEIVE, DISABLE);
    hsuart_deinit(HSUART_REG);

    //时钟源按档位: X48M 档必须先开 CLK_GATE2_X48M 门控(复位默认关, 不开则外设无时钟)
    clk_gate2_cmd(CLK_GATE2_X48M, CLK_EN);
#if VTEST_HS_FORCE_X24M
    clk_hsut0_clk_set(CLK_HSUT0_XOSC24M);       //实验: 全阶梯强制 X24M
#else
    clk_hsut0_clk_set((baud > V_X24M_MAX) ? CLK_HSUT0_XOSC48M : CLK_HSUT0_XOSC24M);
#endif
    printf("[VT] hsuart clk idx=%d freq=%d\n",
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

/*************************** 端口统一接口 ***************************/

static void v_port_tx(const u8 *buf, u16 len)
{
    if (VTEST_PORT == VPORT_HS) {
        bsp_huart_tx(buf, len);             //原厂: 寄存器即 DMA
        bsp_huart_wait_txdone();            //原厂铁律 3: buf 复用前等 TXPND
    } else {
        v_uart1_tx(buf, len);
    }
}

//换档策略(HSUART): 同时钟源内用原厂 huart_set_baudrate 活体切换(已验证零误码 PASS 版同款);
//跨时钟源(X24M<->X48M)才走"停->切时钟->重新 init"——重新 init 存在崩溃窗口,
//实测 6M 档(无时钟切换的重 init)在窗口内 EPC=0, 能少走就少走
static u8 v_hs_cur48;                   //当前 HSUART 时钟源是否 X48M(boot=0: X24M)

//每档统计复位: 同时钟源活体换档不经过 init, 统计必须手动清,
//否则 good 累计、seq 跨档接续产生假 miss(实测踩过); v_ring_drop 同理(每档上报+判据)
static void v_stat_reset(void)
{
    memset((u8 *)&v_stat, 0, sizeof(v_stat));
    v_lrc_err = 0;
    v_ring_drop = 0;
}

static void v_port_set_baud(u32 baud)
{
    if (VTEST_PORT == VPORT_HS) {
#if VTEST_HS_FORCE_X24M
        u8 need48 = 0;                          //实验: 全阶梯强制 X24M
#else
        u8 need48 = (baud > V_X24M_MAX) ? 1 : 0;
#endif

        if (need48 == v_hs_cur48) {
            huart_set_baudrate(baud);       //原厂库: 驱动按当前时钟源自动算分频
        } else {
            v_hs_cur48 = need48;
            v_hs_init(baud);                //跨时钟源: 停->切时钟->全新 init(内含 v_rx_state_reset)
        }
    } else {
        v_uart1_deinit();
        clk_gate2_cmd(CLK_GATE2_X48M, CLK_EN);
#if VTEST_UART1_FORCE_X24M
        clk_uart_clk_set(UART1_REG, CLK_UART_XOSC24M);      //实验: 全阶梯强制 X24M
#else
        clk_uart_clk_set(UART1_REG, (baud > V_X24M_MAX) ? CLK_UART_XOSC48M : CLK_UART_XOSC24M);
#endif
        v_uart1_init(baud);
    }

    //从机视角: 换档后清每档统计(主机在收到 ACK 后自切, 不经此路径也无妨)
    v_stat_reset();
}

//小帧发送: 填帧 -> 端口发送(HS=bsp_huart_tx DMA / UART1=逐字节)
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

    v_port_tx(v_tx_frm, (u16)(FRM_OVERHEAD + len));
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

static void v_poll_rx(void)
{
    //HSUART: 搬运已填满的 DMA 缓冲(两块都置位时先搬 0 号 = 更早填满的, 保证字节序)。
    //清位必须关中断: v_poll_rx 对 v_fmark 是读-改-写, ISR 可能同时置另一位
    while (v_fmark != 0) {
        u8 idx;
        u16 len;
        u16 i;

        GLOBAL_INT_DISABLE();
        idx = (v_fmark & 1u) ? 0 : 1;
        v_fmark &= (u8)~(1u << idx);
        len = v_flen[idx];
        GLOBAL_INT_RESTORE();

        for (i = 0; i < len; i++) {
            v_raw_last[i % (sizeof(v_raw_last))] = v_dma_buf[idx][i];
            v_ring_push(v_dma_buf[idx][i]);
        }
    }

    while (v_ring.tail != v_ring.head) {
        u8 byte = v_ring.buf[v_ring.tail & (V_RING_SIZE - 1)];      //防御: tail 异常时不野读

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

//发 SET_BAUD -> 等旧波特率下的 BAUD_ACK -> 双方各自"停->切时钟->重新 init"
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
                v_port_set_baud(baud);
                v_delay_ms_wdt(100);
                printf("[VT][M] baud -> %u\n", baud);
                return true;
            }
        }
    }

    printf("[VT][M] ERROR: no BAUD_ACK for %u\n", baud);
    return false;
}

//2 秒连续数据块: 填 buf -> 端口发送 -> 等完成(原厂铁律: buf 复用前必须等发送完)
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

        v_port_tx(v_blk_buf, BLOCK_TOTAL);

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

    printf("[VT][M] ===== phase %s =====\n", VTEST_PORT_NAME);
    if (!master_hello()) {
        goto summary;
    }
    printf("[VT][M] hello ok\n");

    for (bi = 0; bi < V_BAUD_CNT; bi++) {
        u32 baud = v_baud_tbl[bi];
        u32 sent, blocks, ms, good = 0, miss = 0, blk_ok = 0, blk_crc = 0, drop = 0;
        bool pass;

        if (!master_switch_baud(baud)) {
            break;
        }

        master_stream(&sent, &blocks, &ms);
        if (!master_stats(&good, &miss, &blk_ok, &blk_crc, &drop)) {
            fail_streak++;
            printf("[VT][M] %u: sent=%u(%u blk)/%ums no STATS_RSP FAIL\n", baud, sent, blocks, ms);
            if (fail_streak >= 2) {
                printf("[VT][M] link dead, stop ramp\n");
                break;
            }
            continue;
        }

        pass = ((good == sent) && (miss == 0) && (blk_crc == 0) && (drop == 0));
        printf("[VT][M] %u: sent=%u(%u blk)/%ums good=%u miss=%u crc=%u drop=%u %s\n",
               baud, sent, blocks, ms, good, miss, blk_crc, drop, pass ? "PASS" : "FAIL");

        if (pass) {
            best_baud = baud;
            best_thr = good * 1000 / ms;
            fail_streak = 0;
        } else {
            fail_streak++;
        }
    }

summary:
    if (best_baud != 0) {
        printf("[VT][M] SUMMARY: %s max clean baud=%u thr=%u B/s\n",
               VTEST_PORT_NAME, best_baud, best_thr);
    } else {
        printf("[VT][M] SUMMARY: %s NO clean baud\n", VTEST_PORT_NAME);
    }
}

/*************************** 从机流程 ***************************/

static void slave_flow(void)
{
    u32 t_rx = tick_get();
    u32 last_total = 0;

    printf("[VT][S] wait HELLO_REQ @%u (port=%s)\n", V_BAUD_BASE,
           (VTEST_PORT == VPORT_HS) ? "HSUART" : "UART1");

    while (1) {
        WDT_CLR();
        msg_dequeue();
        v_poll_rx();

        if (v_raw_total != last_total) {
            last_total = v_raw_total;
            t_rx = tick_get();
        }

        if (v_f.hello_req) {
            v_f.hello_req = false;
            v_send_frame(CMD_HELLO_RSP, NULL, 0);
            printf("[VT][S] HELLO_REQ got, RSP sent\n");
        }

        if (v_f.set_baud) {
            u32 baud = v_f.set_baud_val;

            v_f.set_baud = false;
            v_send_frame(CMD_BAUD_ACK, NULL, 0);        //旧波特率下应答, 之后双方各自重 init
            v_port_set_baud(baud);
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

        //线路空闲回落: 长时间无字节则回基础波特率(等待主机重测)。
        //回落可能发生在半截帧中间, 解析器一并复位(否则回落后第一个拼接帧 LRC 错一次)
        if (tick_check_expire(t_rx, V_SLAVE_RESYNC_MS) && (v_raw_total != 0)) {
            printf("[VT][S] idle, re-init @%u\n", V_BAUD_BASE);
            v_parser_reset();
            v_port_set_baud(V_BAUD_BASE);
            t_rx = tick_get();
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
    printf("[VT] ===== single-port 12M ladder (bsp/driver lib) =====\n");
    printf("[VT] port=%s ladder: 460800..12M, window 2s\n", VTEST_PORT_NAME);
#if UART_TEST_PORT_SEL && VTEST_HS_FORCE_X24M
    printf("[VT] clk: X24M all steps (EXPERIMENT, oversampling test)\n");
#elif !UART_TEST_PORT_SEL && VTEST_UART1_FORCE_X24M
    printf("[VT] clk: X24M all steps (EXPERIMENT, UART1)\n");
#else
    printf("[VT] clk: X24M <=1.5M, X48M >1.5M, re-init per step\n");
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

#if UART_TEST_PORT_SEL
    v_uart1_deinit();                       //HS 模式: 收掉 boot 遗留的 UART1(共享 PA0/PA1)
    v_hs_init(V_BAUD_BASE);
#else
    v_uart1_init(V_BAUD_BASE);
#endif

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

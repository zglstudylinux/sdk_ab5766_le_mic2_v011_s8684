#include "include.h"
#include "uart_speed_test.h"
#include "uart_test_hw.h"
#include "uart_loopback_test.h"

#if UART_SPEED_TEST_EN

/*************************** 测试参数 ***************************/

#define TEST_BASE_BAUD              115200      //两板上电初始通讯波特率
#define TEST_WINDOW_MS              2000        //每档波特率的数据发送窗口
#define TEST_IDLE_MS                150         //数据窗口结束判定的线路空闲时间
#define TEST_RSP_TIMEOUT_MS         300         //应答超时
#define TEST_HELLO_TIMEOUT_MS       10000       //等待从机握手的总超时
#define TEST_FALLBACK_IDLE_MS       3000        //从机线路空闲自动回落到 TEST_BASE_BAUD
#define TEST_FALLBACK_WAIT_MS       3500        //主机切换阶段等待对端回落的时间

//普通串口波特率爬坡表(>1.5M 自动切 XOSC48M 时钟源)
static const u32 uart1_baud_tbl[] = {
    115200, 250000, 460800, 921600, 1500000, 2000000, 2500000, 3000000,
};

//高速串口波特率爬坡表: 先限制在原厂 bsp_huart 验证过的 <=1.5M 域(默认时钟源+驱动自动分频),
//基础通路跑通后再探索更高档(需要原厂确认时钟源配置)
static const u32 hsuart_baud_tbl[] = {
    460800, 921600, 1500000,
};

/*************************** 帧协议 ***************************
 * 帧格式: [A5][5A][cmd][len][payload(len)][lrc]
 * lrc = cmd ^ len ^ payload 全部字节的异或
 * 数据块也是帧: cmd=CMD_DATA, len=BLOCK_PAYLOAD, payload[0]=seq
 **************************************************************/
#define FRM_MAGIC0                  0xA5
#define FRM_MAGIC1                  0x5A

enum {
    CMD_HELLO_REQ = 0x01,           //主机->从机 握手
    CMD_HELLO_RSP = 0x02,
    CMD_BAUD_REQ  = 0x03,           //双方切波特率, payload=u32 LE
    CMD_BAUD_RSP  = 0x04,
    CMD_RUN_REQ   = 0x05,           //开始数据窗口, payload=u32 窗口ms
    CMD_RUN_RSP   = 0x06,
    CMD_STATS_REQ = 0x07,           //查询从机统计
    CMD_STATS_RSP = 0x08,           //payload: u32 good, u32 miss, u16 blk_ok, u16 blk_crc, u16 drop
    CMD_HS_REQ    = 0x09,           //切换到高速串口, payload=u32 初始波特率
    CMD_HS_RSP    = 0x0A,
    CMD_DONE      = 0x0B,           //全部结束
    CMD_DATA      = 0x10,           //数据块
};

#define BLOCK_PAYLOAD               252
#define FRM_OVERHEAD                5       //magic2 + cmd + len + lrc
#define BLOCK_TOTAL                 (FRM_OVERHEAD + BLOCK_PAYLOAD)
#define STATS_PAYLOAD_LEN           14      //u32 good + u32 miss + u16 blk_ok + u16 blk_crc + u16 drop

/*************************** 公共状态 ***************************/

static uart_hw_type_t active_type = UART_HW_UART1;
static bool is_master;

typedef struct {
    u16 idx;
    u16 need;
    u8 buf[BLOCK_TOTAL];
} frm_parser_t;

static u32 bad_frame_cnt;           //lrc 校验失败的帧数(数据块即 crc 错块数)

static void frm_reset(frm_parser_t *p)
{
    p->idx = 0;
    p->need = 0;
}

static void frm_dispatch(frm_parser_t *p, void (*on_frame)(u8, u8, u8 *))
{
    u8 cmd = p->buf[2];
    u8 len = p->buf[3];
    u8 lrc = (u8)(cmd ^ len);
    u16 i;

    for (i = 0; i < len; i++) {
        lrc ^= p->buf[4 + i];
    }

    if (lrc == p->buf[4 + len]) {
        on_frame(cmd, len, &p->buf[4]);
    } else {
        bad_frame_cnt++;
    }

    p->idx = 0;
}

static void frm_feed(frm_parser_t *p, u8 byte, void (*on_frame)(u8, u8, u8 *))
{
    if (p->idx == 0) {
        if (byte == FRM_MAGIC0) {
            p->buf[p->idx++] = byte;
        }
    } else if (p->idx == 1) {
        if (byte == FRM_MAGIC1) {
            p->buf[p->idx++] = byte;
        } else {
            p->idx = (byte == FRM_MAGIC0) ? 1 : 0;
            if (p->idx == 1) {
                p->buf[0] = FRM_MAGIC0;
            }
        }
    } else if (p->idx == 2) {
        //cmd 字节: 此时 need 还是上一帧残留/0, 不能做帧尾判断, 只存储
        p->buf[p->idx++] = byte;
    } else if (p->idx == 3) {
        if (byte > BLOCK_PAYLOAD) {         //非法长度, 重新找帧头
            p->idx = (byte == FRM_MAGIC0) ? 1 : 0;
            p->buf[0] = FRM_MAGIC0;
            return;
        }
        p->buf[p->idx++] = byte;
        p->need = FRM_OVERHEAD + byte;      //len 到达后 need 才有效
    } else {
        p->buf[p->idx++] = byte;
        if (p->idx >= p->need) {
            frm_dispatch(p, on_frame);
        }
    }
}

static bool send_frame(u8 cmd, const u8 *payload, u8 len)
{
    u8 lrc = (u8)(cmd ^ len);
    u8 i;

    if (!uart_hw_putbyte(active_type, FRM_MAGIC0)) {
        return false;
    }
    if (!uart_hw_putbyte(active_type, FRM_MAGIC1)) {
        return false;
    }
    if (!uart_hw_putbyte(active_type, cmd)) {
        return false;
    }
    if (!uart_hw_putbyte(active_type, len)) {
        return false;
    }
    for (i = 0; i < len; i++) {
        lrc ^= payload[i];
        if (!uart_hw_putbyte(active_type, payload[i])) {
            return false;
        }
    }

    return uart_hw_putbyte(active_type, lrc);
}

/*************************** 主机侧 ***************************/

static frm_parser_t master_parser AT(.buf.le_dut.uart_test); //位于 .buf.le_dut 段, 该段必须整体 <= 2KB(0x17800~0x18000), 详见 uart_test_hw.c 顶部说明
static volatile bool rsp_ready;
static volatile u8 rsp_pending_cmd;
static u8 rsp_payload[16];

static void master_on_frame(u8 cmd, u8 len, u8 *payload)
{
    u8 i;

    switch (cmd) {
        case CMD_HELLO_RSP:
        case CMD_BAUD_RSP:
        case CMD_RUN_RSP:
        case CMD_STATS_RSP:
        case CMD_HS_RSP:
            if (len > sizeof(rsp_payload)) {
                len = sizeof(rsp_payload);
            }
            for (i = 0; i < len; i++) {
                rsp_payload[i] = payload[i];
            }
            rsp_pending_cmd = cmd;
            rsp_ready = true;
            break;

        default:
            break;
    }
}

static void master_pump_rx(void)
{
    u8 tmp[128];
    u16 n = uart_hw_read(tmp, sizeof(tmp));
    u16 i;

    for (i = 0; i < n; i++) {
        frm_feed(&master_parser, tmp[i], master_on_frame);
    }
}

static bool wait_rsp(u8 cmd, u32 timeout_ms)
{
    u32 t0 = tick_get();

    rsp_ready = false;
    rsp_pending_cmd = 0;

    while (!tick_check_expire(t0, timeout_ms)) {
        WDT_CLR();
        msg_dequeue();
        master_pump_rx();
        if (rsp_ready) {
            return (rsp_pending_cmd == cmd);
        }
    }

    return false;
}

typedef struct {
    bool ok;                //无误码
    u32 baud;
    u32 tx_bytes;
    u32 tx_ms;
    u32 rx_good;
    u32 rx_miss;
    u16 blk_ok;
    u16 blk_crc;
    u16 rx_drop;
} baud_result_t;

//缓冲留在低地址普通 RAM(.bss): HSUART DMA 访问 0x18000 以上会总线超时(REQ TOUT)
static u8 tx_block[BLOCK_TOTAL];
static u8 tx_block_lrc_base;        //lrc 中除 seq 外的固定部分

static void tx_block_template_init(void)
{
    u16 i;

    tx_block[0] = FRM_MAGIC0;
    tx_block[1] = FRM_MAGIC1;
    tx_block[2] = CMD_DATA;
    tx_block[3] = BLOCK_PAYLOAD;

    //payload 固定图样: payload[0]=seq(逐块变化), 其后 i*7+0x33
    tx_block_lrc_base = (u8)(CMD_DATA ^ BLOCK_PAYLOAD);
    for (i = 1; i < BLOCK_PAYLOAD; i++) {
        tx_block[4 + i] = (u8)(i * 7 + 0x33);
        tx_block_lrc_base ^= tx_block[4 + i];
    }
}

static bool send_data_block(u8 seq)
{
    u16 i;

    tx_block[4] = seq;
    tx_block[BLOCK_TOTAL - 1] = tx_block_lrc_base ^ seq;

    if (active_type == UART_HW_HSUART) {
        return uart_hw_hsuart_tx_block(tx_block, BLOCK_TOTAL);
    }

    for (i = 0; i < BLOCK_TOTAL; i++) {
        if (!uart_hw_putbyte(UART_HW_UART1, tx_block[i])) {
            return false;
        }
    }

    return true;
}

static bool run_window(u32 *out_bytes, u32 *out_ms)
{
    u32 t0 = tick_get();
    u32 sent = 0;
    u8 seq = 0;

    while (!tick_check_expire(t0, TEST_WINDOW_MS)) {
        if (!send_data_block(seq)) {
            printf("TX abort at seq=%u\n", seq);
            *out_bytes = sent;
            *out_ms = tick_get() - t0;
            return false;
        }
        sent += BLOCK_PAYLOAD;
        if ((seq & 0x07) == 0) {
            WDT_CLR();
            msg_dequeue();
        }
        seq++;
    }

    *out_bytes = sent;
    *out_ms = tick_get() - t0;
    if (*out_ms == 0) {
        *out_ms = 1;
    }

    return true;
}

static bool send_cmd_with_retry(u8 cmd, const u8 *payload, u8 len, u8 rsp_cmd, u8 retry)
{
    u8 i;

    for (i = 0; i < retry; i++) {
        WDT_CLR();
        if (send_frame(cmd, payload, len) && wait_rsp(rsp_cmd, TEST_RSP_TIMEOUT_MS)) {
            return true;
        }
        delay_ms(50);
    }

    return false;
}

//跑一档波特率: 切波特率 -> 数据窗口 -> 收对端统计。返回 false 表示对端无应答
static bool run_one_baud(uart_hw_type_t type, u32 baud, baud_result_t *r)
{
    u8 pl[4];
    u32 tx_bps, theo_bps;

    memset(r, 0, sizeof(baud_result_t));
    r->baud = baud;
    put_be32(pl, baud);

    //1.在旧波特率下通知对端切换
    if (!send_cmd_with_retry(CMD_BAUD_REQ, pl, 4, CMD_BAUD_RSP, 3)) {
        return false;
    }
    delay_ms(30);                       //等对端应答完全发出
    uart_hw_set_baud(type, baud);
    delay_ms(20);

    //2.数据窗口
    put_be32(pl, TEST_WINDOW_MS);
    if (!send_cmd_with_retry(CMD_RUN_REQ, pl, 4, CMD_RUN_RSP, 3)) {
        return false;
    }
    delay_ms(10);

    bool tx_ok = run_window(&r->tx_bytes, &r->tx_ms);

    //3.等线路空闲后取对端统计
    delay_ms(TEST_IDLE_MS + 100);
    if (!send_cmd_with_retry(CMD_STATS_REQ, NULL, 0, CMD_STATS_RSP, 3)) {
        return false;
    }

    r->rx_good = get_be32(&rsp_payload[0]);
    r->rx_miss = get_be32(&rsp_payload[4]);
    r->blk_ok = get_be16(&rsp_payload[8]);
    r->blk_crc = get_be16(&rsp_payload[10]);
    r->rx_drop = get_be16(&rsp_payload[12]);
    r->ok = tx_ok && (r->rx_good > 0) && (r->rx_miss == 0) &&
            (r->blk_crc == 0) && (r->rx_drop == 0);

    tx_bps = r->tx_bytes * 1000 / r->tx_ms;
    theo_bps = baud / 10;

    printf("[%s] baud=%u  tx=%uB/%ums  thr=%uB/s (%u%% of theo)  | rx good=%u miss=%u blk=%u crc=%u drop=%u  %s\n",
           (type == UART_HW_HSUART) ? "HSUART" : "UART1",
           baud, r->tx_bytes, r->tx_ms, tx_bps,
           theo_bps ? (tx_bps * 100 / theo_bps) : 0,
           r->rx_good, r->rx_miss, r->blk_ok, r->blk_crc, r->rx_drop,
           r->ok ? "PASS" : "FAIL");

    return true;
}

static const char *type_name(uart_hw_type_t type)
{
    return (type == UART_HW_HSUART) ? "HSUART" : "UART1 ";
}

//原始链路诊断(握手失败时): 持续发 55AA 裸流 30 秒并统计本板收到的一切,
//结合从机侧 [SLAVE][RAW] 打印即可判定 断线/共地/跳线未拔/波特率/协议 层问题
static void raw_diag_master(void)
{
    static const u8 pat[8] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
    u8 rxb[32];
    u8 first[16];
    u32 sent = 0;
    u32 recv = 0;
    u32 t0 = tick_get();
    u16 first_len = 0;

    printf("[TEST] ERROR: no slave response! enter RAW diagnose 30s\n");
    printf("[DIAG] sending raw 55AA stream, watch SLAVE board [SLAVE][RAW] log\n");
    printf("[DIAG] keep dual-board wires connected, loopback jumper REMOVED\n");

    while (!tick_check_expire(t0, 30000)) {
        u16 i;
        u16 n;

        WDT_CLR();
        msg_dequeue();
        for (i = 0; i < 8; i++) {
            uart_hw_putbyte(UART_HW_UART1, pat[i]);
        }
        sent += 8;

        n = uart_hw_read(rxb, sizeof(rxb));
        for (i = 0; i < n; i++) {
            if (first_len < (u16)sizeof(first)) {
                first[first_len] = rxb[i];
                first_len++;
            }
        }
        recv += n;
        delay_ms(100);
    }

    printf("[DIAG] master sent=%u recv=%u first_rx:", sent, recv);
    for (u16 i = 0; i < first_len; i++) {
        printf(" %02x", first[i]);
    }
    printf("\n");
    printf("[DIAG] recv=0          -> slave sent nothing back: see slave log\n");
    printf("[DIAG] rx=55 aa 55 aa  -> own echo: loopback jumper still installed!\n");
    printf("[DIAG] rx=a5 5a ...    -> slave frames arrived: protocol issue\n");
    printf("[DIAG] rx=00/ff/garbage-> GND/baud mismatch\n");
}

//爬坡: 连续 2 档失败即停, best 记录最高无误码档
static void ramp_phase(uart_hw_type_t type, const u32 *tbl, u8 cnt,
                       u32 *best_baud, u32 *best_thr)
{
    baud_result_t res;
    int fail_streak = 0;
    u8 i;

    *best_baud = 0;
    *best_thr = 0;

    for (i = 0; i < cnt; i++) {
        bool got = run_one_baud(type, tbl[i], &res);

        if (got && res.ok) {
            *best_baud = tbl[i];
            *best_thr = res.tx_bytes * 1000 / res.tx_ms;
            fail_streak = 0;
        } else {
            fail_streak++;
            printf("[%s] baud=%u %s\n", type_name(type), tbl[i],
                   got ? "data error (see stats above)" : "no response");
            if (fail_streak >= 2) {
                printf("[%s] 2 consecutive failed bauds, stop ramp\n", type_name(type));
                return;
            }
        }
    }
}

static void master_flow(void)
{
    u8 pl[4];
    u32 u1_best_baud = 0, u1_best_thr = 0;
    u32 hs_best_baud = 0, hs_best_thr = 0;

    //握手
    printf("[TEST] master waiting slave hello...\n");
    u32 t0 = tick_get();
    u32 last_req = 0;
    bool hello = false;

    while (!tick_check_expire(t0, TEST_HELLO_TIMEOUT_MS)) {
        WDT_CLR();
        msg_dequeue();
        master_pump_rx();
        if ((last_req == 0) || tick_check_expire(last_req, 500)) {
            last_req = tick_get();
            pl[0] = 1;                  //payload: 协议版本
            send_frame(CMD_HELLO_REQ, pl, 1);
        }
        if (rsp_ready && (rsp_pending_cmd == CMD_HELLO_RSP)) {
            hello = true;
            break;
        }
    }

    if (!hello) {
        //握手失败 -> 原始链路诊断: 绕过协议层直接发裸字节, 从机侧同步打印收到的原始字节
        raw_diag_master();
        while (1) {
            WDT_CLR();
            msg_dequeue();
        }
    }
    printf("[TEST] slave hello OK, start ramp\n\n");

    //Phase 1: 普通串口 UART1
    printf("===== Phase 1: UART1 (normal uart) =====\n");
    ramp_phase(UART_HW_UART1, uart1_baud_tbl, sizeof(uart1_baud_tbl) / sizeof(u32),
               &u1_best_baud, &u1_best_thr);

    //Phase 2: 切换高速串口(同一对线, 分时复用)
    //统一从基础波特率发起: 主机静默 TEST_FALLBACK_WAIT_MS, 从机空闲 3s 自动回落到基础波特率
    printf("\n===== Phase 2: HSUART (high speed uart) =====\n");
    uart_hw_set_baud(UART_HW_UART1, TEST_BASE_BAUD);
    printf("[TEST] waiting slave fallback to %u bps...\n", TEST_BASE_BAUD);
    t0 = tick_get();
    while (!tick_check_expire(t0, TEST_FALLBACK_WAIT_MS)) {
        WDT_CLR();
        msg_dequeue();
        master_pump_rx();
    }
    frm_reset(&master_parser);

    put_be32(pl, hsuart_baud_tbl[0]);
    if (send_cmd_with_retry(CMD_HS_REQ, pl, 4, CMD_HS_RSP, 5)) {
        delay_ms(100);                  //等对端一起切换
        uart_hw_deinit(UART_HW_UART1);
        uart_hw_init(UART_HW_HSUART, hsuart_baud_tbl[0]);
        active_type = UART_HW_HSUART;
        delay_ms(50);
        frm_reset(&master_parser);
        bad_frame_cnt = 0;

        ramp_phase(UART_HW_HSUART, hsuart_baud_tbl, sizeof(hsuart_baud_tbl) / sizeof(u32),
                   &hs_best_baud, &hs_best_thr);
    } else {
        printf("[TEST] switch cmd no response, skip HSUART phase\n");
    }

    //汇总
    printf("\n===== SUMMARY =====\n");
    printf("UART1  max clean baud : %u (thr %u B/s)\n", u1_best_baud, u1_best_thr);
    printf("HSUART max clean baud : %u (thr %u B/s)\n", hs_best_baud, hs_best_thr);
    printf("note: clean = rx miss=0, crc=0, drop=0 in %ums window\n", TEST_WINDOW_MS);

    //通知从机结束(基础波特率或 HS 波特率下尽力发)
    send_frame(CMD_DONE, NULL, 0);
}

/*************************** 从机侧 ***************************/

static frm_parser_t slave_parser AT(.buf.le_dut.uart_test);
static u32 slave_cur_baud;
static u32 slave_new_baud;
static u32 slave_hs_baud;
static u32 slave_last_rx_tick;
static bool pending_baud_switch;
static bool pending_hs_switch;
static u8 pl_tmp[4];

typedef struct {
    u32 good;
    u32 miss;
    u16 blk_ok;
    u8 last_seq;
    bool window_active;
    u32 last_byte_tick;
} slave_stats_t;

static slave_stats_t slave_stats;

static void slave_send_stats(void)
{
    u8 pl[STATS_PAYLOAD_LEN];

    put_be32(&pl[0], slave_stats.good);
    put_be32(&pl[4], slave_stats.miss);
    put_be16(&pl[8], slave_stats.blk_ok);
    put_be16(&pl[10], bad_frame_cnt);
    put_be16(&pl[12], (u16)uart_hw_get_drop_cnt());

    send_frame(CMD_STATS_RSP, pl, STATS_PAYLOAD_LEN);
}

static void slave_on_frame(u8 cmd, u8 len, u8 *payload)
{
    switch (cmd) {
        case CMD_HELLO_REQ:
            pl_tmp[0] = 1;                  //1 = slave
            send_frame(CMD_HELLO_RSP, pl_tmp, 1);
            printf("[SLAVE] HELLO_REQ got, RSP sent\n");
            break;

        case CMD_BAUD_REQ:
            slave_new_baud = get_be32(payload);
            send_frame(CMD_BAUD_RSP, NULL, 0);
            pending_baud_switch = true;
            break;

        case CMD_RUN_REQ:
            memset(&slave_stats, 0, sizeof(slave_stats));
            bad_frame_cnt = 0;
            slave_stats.window_active = true;
            slave_stats.last_byte_tick = tick_get();
            send_frame(CMD_RUN_RSP, NULL, 0);
            break;

        case CMD_DATA:
            if (len == BLOCK_PAYLOAD) {
                u8 seq = payload[0];
                u8 missed = (u8)(seq - slave_stats.last_seq - 1);

                slave_stats.blk_ok++;
                slave_stats.good += len;
                if (slave_stats.blk_ok > 1) {
                    slave_stats.miss += missed;
                }
                slave_stats.last_seq = seq;
                slave_stats.last_byte_tick = tick_get();
            }
            break;

        case CMD_STATS_REQ:
            slave_send_stats();
            break;

        case CMD_HS_REQ:
            slave_hs_baud = get_be32(payload);
            send_frame(CMD_HS_RSP, NULL, 0);
            pending_hs_switch = true;
            break;

        case CMD_DONE:
            printf("[SLAVE] done. good=%u miss=%u blk=%u crc=%u drop=%u\n",
                   slave_stats.good, slave_stats.miss, slave_stats.blk_ok,
                   bad_frame_cnt, uart_hw_get_drop_cnt());
            break;

        default:
            break;
    }
}

static void slave_flow(void)
{
    u8 tmp[256];
    u8 raw_buf[16];                     //原始链路诊断: 最近收到的字节样本
    u16 raw_len = 0;
    u32 raw_total = 0;
    u32 raw_print_tick = 0;

    slave_cur_baud = TEST_BASE_BAUD;
    slave_last_rx_tick = tick_get();

    for (;;) {
        WDT_CLR();
        msg_dequeue();

        //收包解析
        u16 n = uart_hw_read(tmp, sizeof(tmp));
        u16 i;

        if (n > 0) {
            slave_last_rx_tick = tick_get();
        }
        for (i = 0; i < n; i++) {
            frm_feed(&slave_parser, tmp[i], slave_on_frame);
        }

        //原始链路诊断: 基础波特率空闲期, 收到任何字节都记录并限流打印 hex
        //(主机发什么这里就显示什么: 55aa=裸流诊断, a5 5a=协议帧, 00/ff=电平/波特率异常)
        if ((n > 0) && (slave_cur_baud == TEST_BASE_BAUD) && !slave_stats.window_active &&
            !pending_baud_switch && !pending_hs_switch) {
            for (i = 0; i < n; i++) {
                if (raw_len < (u16)sizeof(raw_buf)) {
                    raw_buf[raw_len] = tmp[i];
                    raw_len++;
                }
                raw_total++;
            }
            if (tick_check_expire(raw_print_tick, 1000)) {
                raw_print_tick = tick_get();
                printf("[SLAVE][RAW] total=%u last:", raw_total);
                for (i = 0; i < raw_len; i++) {
                    printf(" %02x", raw_buf[i]);
                }
                printf("\n");
                raw_len = 0;
            }
        }

        //波特率切换: 应答已发出, 延时后切
        if (pending_baud_switch) {
            pending_baud_switch = false;
            delay_ms(20);
            uart_hw_set_baud(active_type, slave_new_baud);
            slave_cur_baud = slave_new_baud;
        }

        //切换高速串口
        if (pending_hs_switch) {
            pending_hs_switch = false;
            delay_ms(100);
            uart_hw_deinit(UART_HW_UART1);
            uart_hw_init(UART_HW_HSUART, slave_hs_baud);
            active_type = UART_HW_HSUART;
            slave_cur_baud = slave_hs_baud;
            frm_reset(&slave_parser);
            bad_frame_cnt = 0;
        }

        //数据窗口空闲判定
        if (slave_stats.window_active &&
            tick_check_expire(slave_stats.last_byte_tick, TEST_IDLE_MS)) {
            slave_stats.window_active = false;
            printf("[SLAVE] window end: good=%u miss=%u blk=%u crc=%u\n",
                   slave_stats.good, slave_stats.miss, slave_stats.blk_ok,
                   bad_frame_cnt);
        }

        //线路空闲超时自动回落到基础波特率, 供主机重新同步(如断链恢复/切换阶段)
        //只在 UART1 模式生效: HSUART 是测试目标, 空闲回落会改掉正在使用的测试波特率
        if ((active_type == UART_HW_UART1) && (slave_cur_baud != TEST_BASE_BAUD) &&
            !slave_stats.window_active &&
            tick_check_expire(slave_last_rx_tick, TEST_FALLBACK_IDLE_MS)) {
            uart_hw_set_baud(active_type, TEST_BASE_BAUD);
            slave_cur_baud = TEST_BASE_BAUD;
            printf("[SLAVE] idle fallback to %u bps\n", TEST_BASE_BAUD);
        }
    }
}

/*************************** 入口 ***************************/

void func_uart_test(void)
{
    static bool test_finished;              //防止 func_run 大循环重复进入重跑测试

    if (test_finished) {
        while (1) {
            WDT_CLR();
            msg_dequeue();
        }
    }

    printf("%s\n", __func__);

    msg_queue_clear();
    lowpwr_pwroff_auto_dis();

    //HSUART 收发走 CPU 逐字节路径(无 DMA), 高波特率下需要主频兜底;
    //测试模式不跑无线/音频, 且串口波特率用独立晶振时钟源, 提频不影响双方通讯
    sys_clk_set(SYS_120M);

    is_master = wireless_role_is_adapter();
    active_type = UART_HW_UART1;
    frm_reset(&master_parser);
    frm_reset(&slave_parser);
    tx_block_template_init();

    if (!uart_hw_init(UART_HW_UART1, TEST_BASE_BAUD)) {
        printf("[TEST] uart init fail\n");
        test_finished = true;
        return;
    }

#if UART_LOOPBACK_TEST_EN
    //第一步: 单板回环测试(跳线 PA0-PA1), 验证本板两路串口收发通路正常
    if (!uart_loopback_test_all()) {
        while (1) {
            WDT_CLR();
            msg_dequeue();
        }
    }

    //第二步: 双板测试, 等待用户从回环跳线改接为双板交叉线
    printf("\n[TEST] dual-board phase: wire A.PA0->B.PA1, B.PA0->A.PA1, GND-GND\n");
    printf("[TEST] IMPORTANT: remove the loopback jumper first!\n");
    printf("[TEST] start in 8s...\n");
    {
        u8 i, j;

        for (i = 8; i > 0; i--) {
            printf("[TEST] %d...\n", i);
            for (j = 0; j < 4; j++) {
                WDT_CLR();
                delay_ms(250);
            }
        }
    }
#else
    printf("\n[TEST] dual-board phase: wire A.PA0->B.PA1, B.PA0->A.PA1, GND-GND\n");
    printf("[TEST] start in 3s...\n");
    {
        u8 i, j;

        for (i = 3; i > 0; i--) {
            printf("[TEST] %d...\n", i);
            for (j = 0; j < 4; j++) {
                WDT_CLR();
                delay_ms(250);
            }
        }
    }
#endif
    uart_hw_init(UART_HW_UART1, TEST_BASE_BAUD);    //清掉回环残留数据, 重开双板通道

    if (is_master) {
        printf("[TEST] role=MASTER (adapter)\n");
        master_flow();
    } else {
        printf("[TEST] role=SLAVE (mic)\n");
        slave_flow();
    }

    test_finished = true;                   //主机侧测试结束, 停在这里等用户断电
    printf("[TEST] finished, reset board to re-run\n");
    while (1) {
        WDT_CLR();
        msg_dequeue();
    }
}

#endif // UART_SPEED_TEST_EN

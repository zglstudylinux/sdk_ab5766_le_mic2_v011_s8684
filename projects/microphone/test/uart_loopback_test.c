#include "include.h"
#include "uart_loopback_test.h"
#include "uart_test_hw.h"

#if UART_SPEED_TEST_EN

//每档波特率自发自收的字节量与分段大小
//缓冲布局: loop 发收缓冲放普通 .bss(comm 区余量仅约 800B, 故每块只给 256B)
#define LOOP_VOLUME_BYTES           4096
#define LOOP_CHUNK                  256     //不超过 RX 环形缓冲(512B)
#define LOOP_CHUNK_TIMEOUT_MS       300     //每段收满的超时
#define LOOP_BASE_UART1_BAUD        115200  //双板测试依赖的基础档, 必须通过
#define LOOP_BASE_HS_BAUD           460800

//回环波特率表(与本板时钟自发自收, 理论上各档都应无误码;
//高档位验证 XOSC48M/PLL 时钟源是否真实可用)
static const u32 uart1_loop_baud_tbl[] = {
    115200, 250000, 460800, 921600, 1500000, 2000000, 2500000, 3000000,
};

//回环波特率表: HSUART 先限制在原厂验证过的 <=1.5M 域(跑通后再探索更高档)
static const u32 hs_loop_baud_tbl[] = {
    460800, 921600, 1500000,
};

static u8 loop_tx_buf[LOOP_CHUNK];
static u8 loop_rx_buf[LOOP_CHUNK];

static const char *loop_type_name(uart_hw_type_t type)
{
    return (type == UART_HW_HSUART) ? "HSUART" : "UART1 ";
}

//回环测一档波特率, 返回 true=全收对且无误码
static bool loop_one_baud(uart_hw_type_t type, u32 baud)
{
    u32 total_sent = 0;
    u32 total_recv = 0;
    u32 mismatch = 0;
    u8 tx_cnt = 0;
    u8 rx_expect = 0;
    u16 chunk;

    while (total_sent < LOOP_VOLUME_BYTES) {
        u16 i;
        u16 got = 0;
        u32 t0;

        chunk = LOOP_CHUNK;
        if (chunk > LOOP_VOLUME_BYTES - total_sent) {
            chunk = LOOP_VOLUME_BYTES - total_sent;
        }

        //填充递增图样并自环发送
        for (i = 0; i < chunk; i++) {
            loop_tx_buf[i] = tx_cnt++;
        }

        if (type == UART_HW_HSUART) {
            if (!uart_hw_hsuart_tx_block(loop_tx_buf, chunk)) {
                printf("[LOOP][%s] baud=%u TX dma stuck\n", loop_type_name(type), baud);
                return false;
            }
        } else {
            for (i = 0; i < chunk; i++) {
                if (!uart_hw_putbyte(UART_HW_UART1, loop_tx_buf[i])) {
                    printf("[LOOP][%s] baud=%u TX stuck\n", loop_type_name(type), baud);
                    return false;
                }
            }
        }
        WDT_CLR();

        //等回环数据收满一段
        t0 = tick_get();
        while ((got < chunk) && !tick_check_expire(t0, LOOP_CHUNK_TIMEOUT_MS)) {
            WDT_CLR();
            got += uart_hw_read(&loop_rx_buf[got], chunk - got);
        }

        //比对: 期望序列与发送序列一致(同一晶振自环, 不应有任何丢失/误码)
        for (i = 0; i < got; i++) {
            if (loop_rx_buf[i] != rx_expect) {
                mismatch++;
            }
            rx_expect++;
        }
        total_sent += chunk;
        total_recv += got;

        if (got < chunk) {
            break;                      //收不满, 提前结束该档
        }
    }

    bool pass = (total_recv == total_sent) && (mismatch == 0);

    printf("[LOOP][%s] baud=%u sent=%u recv=%u mismatch=%u  %s\n",
           loop_type_name(type), baud, total_sent, total_recv, mismatch,
           pass ? "PASS" : "FAIL");

    return pass;
}

static bool loop_phase(uart_hw_type_t type, const u32 *tbl, u8 cnt, u32 gate_baud, bool *gate_ok)
{
    u8 i;
    bool first = true;

    for (i = 0; i < cnt; i++) {
        if (first) {
            if (!uart_hw_init(type, tbl[i])) {
                printf("[LOOP][%s] init fail at baud=%u\n", loop_type_name(type), tbl[i]);
                if (tbl[i] == gate_baud) {
                    *gate_ok = false;
                }
                return false;
            }
            first = false;
        } else {
            uart_hw_set_baud(type, tbl[i]);
        }

        if (!loop_one_baud(type, tbl[i])) {
            if (tbl[i] == gate_baud) {
                *gate_ok = false;
            }
        }
        WDT_CLR();
    }

    return true;
}

bool uart_loopback_test_all(void)
{
    bool gate_ok = true;
    int i;

    printf("\n===== LOOPBACK TEST (single board) =====\n");
    printf("[LOOP] jumper PA0(TX)-PA1(RX) on THIS board, disconnect peer wires\n");
    printf("[LOOP] starting in 3s...\n");
    for (i = 3; i > 0; i--) {
        printf("[LOOP] %d...\n", i);
        delay_ms(250);
        WDT_CLR();
        delay_ms(250);
        WDT_CLR();
        delay_ms(250);
        WDT_CLR();
        delay_ms(250);
        WDT_CLR();
    }

    printf("[LOOP] --- UART1 (normal uart) ---\n");
    loop_phase(UART_HW_UART1, uart1_loop_baud_tbl, sizeof(uart1_loop_baud_tbl) / sizeof(u32),
               LOOP_BASE_UART1_BAUD, &gate_ok);
    uart_hw_deinit(UART_HW_UART1);

    printf("[LOOP] --- HSUART (high speed uart) ---\n");
    loop_phase(UART_HW_HSUART, hs_loop_baud_tbl, sizeof(hs_loop_baud_tbl) / sizeof(u32),
               LOOP_BASE_HS_BAUD, &gate_ok);
    uart_hw_deinit(UART_HW_HSUART);

    printf("[LOOP] gate(UART1@%u + HSUART@%u): %s\n",
           LOOP_BASE_UART1_BAUD, LOOP_BASE_HS_BAUD,
           gate_ok ? "OK" : "FAILED - board uart fault, do NOT continue");

    return gate_ok;
}

#endif // UART_SPEED_TEST_EN

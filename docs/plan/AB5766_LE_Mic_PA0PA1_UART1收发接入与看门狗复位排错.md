# AB5766 LE Mic：PA0/PA1 UART1 双线串口收发接入与看门狗复位排错

> 本文档是 PA0/PA1 UART1 双线串口收发功能从接入到排错的完整记录，包含两部分：
>
> - **第一部分 修改文档**：复用已有 `bsp/` 代码接入 UART1 收发（PA0=TX、PA1=RX）的改动说明。
> - **第二部分 排错文档**：接入过程中出现的“反复复位（WDT_RST）”问题、根因分析与修复。
>
> 结论：下载测试成功，另接串口助手可正常收发，问题已闭环。

---

## 目录

- [1. 目标与边界](#1-目标与边界)
- [2. 引脚与硬件约定](#2-引脚与硬件约定)
- [3. 修改文档](#3-修改文档)
  - [3.1 修改文件清单](#31-修改文件清单)
  - [3.2 各文件改动明细](#32-各文件改动明细)
  - [3.3 调用链与数据流](#33-调用链与数据流)
- [4. 排错文档](#4-排错文档)
  - [4.1 现象](#41-现象)
  - [4.2 根因分析](#42-根因分析)
  - [4.3 修复方案](#43-修复方案)
- [5. 测试方法与验证](#5-测试方法与验证)
  - [5.1 CH340 接线](#51-ch340-接线)
  - [5.2 收发测试步骤](#52-收发测试步骤)
  - [5.3 验证证据（Debug 日志）](#53-验证证据debug-日志)
- [6. 风险与后续边界](#6-风险与后续边界)

---

## 1. 目标与边界

- **目标**：使用核心板引出的 PA0、PA1 两根引脚，在 AB5766 SDK 中实现普通 UART 串口收发。
- **原则**：优先复用 `bsp/` 下已有的 [bsp_uart_transfer.c](../../bsp/bsp_uart_transfer.c) 与 [bsp_uart_transfer.h](../../bsp/bsp_uart_transfer.h)，**不重复编写** UART 寄存器初始化、crossbar 配置或中断服务函数。
- **边界**：
  - 本轮只做“双线收发/回显”能力验证，**不臆造正式命令协议**。
  - 保留 UART0 调试打印口（PB3，1.5 Mbps）不动；PA0/PA1 独立使用 UART1。
  - 不新增 UART 驱动、不新增 ISR；保留 [bsp_uart_transfer.c](../../bsp/bsp_uart_transfer.c) 现有 `AT(.com_text.isr)` 标注。

---

## 2. 引脚与硬件约定

| 项 | 值 |
|---|---|
| UART1 TX（芯片发送） | **PA0**（`GPIOA_REG` / `GPIO_PIN_0`） |
| UART1 RX（芯片接收） | **PA1**（`GPIOA_REG` / `GPIO_PIN_1`） |
| 波特率 | 115200 |
| 帧格式 | 8 数据位 / 无校验 / 1 停止位（8N1） |
| UART1 时钟 | `CLK_UART_XOSC24M`（24 MHz） |
| 电平 | 3.3 V 电平兼容，**禁止 5 V / VUSB 直接输入** |

方向定义来自 [bsp_uart_transfer.h](../../bsp/bsp_uart_transfer.h)：

```c
#define UART_TX_PORT                GPIOA_REG
#define UART_TX_PIN                 GPIO_PIN_0
#define UART_RX_PORT                GPIOA_REG
#define UART_RX_PIN                 GPIO_PIN_1
```

即：**PA0 = 芯片 TX，PA1 = 芯片 RX**。对外接线按交叉方式：芯片 PA0(TX) → 对端 RX，芯片 PA1(RX) ← 对端 TX，双方共地。

> 若核心板原理图实际要求 PA0 接收、PA1 发送，只需调整上述头文件中两个宏的引脚对应关系，并以原理图为准。

---

## 3. 修改文档

### 3.1 修改文件清单

| 文件 | 改动类型 | 作用 |
|---|---|---|
| [projects/microphone/config_ab5766_le_mic.h](../../projects/microphone/config_ab5766_le_mic.h) | 新增宏 | 增加 UART1 transfer 功能开关与波特率 |
| [bsp/bsp.h](../../bsp/bsp.h) | 新增 include | 汇总 transfer 头文件，暴露已有 BSP 接口 |
| [bsp/bsp_sys.c](../../bsp/bsp_sys.c) | 新增调用 | 启动 / 低功耗唤醒后初始化 UART1 |
| [functions/func.c](../../functions/func.c) | 新增调用 | 公共非中断循环中调用回显处理 |
| [bsp/bsp_uart_transfer.c](../../bsp/bsp_uart_transfer.c) | 最小加固 | 初始化复位软件状态 + 发送超时保护 |

### 3.2 各文件改动明细

#### 3.2.1 配置入口 [config_ab5766_le_mic.h](../../projects/microphone/config_ab5766_le_mic.h)

在 `BSP_UART_DEBUG_EN` 之后新增两个编译开关：

```c
#define BSP_UART_DEBUG_EN               GPIO_PB3                //串口打印调试使能[Baud=1500000]
#define BSP_UART_TRANSFER_EN            1                       //UART1 双线串口收发使能(PA0=TX, PA1=RX)
#define BSP_UART_TRANSFER_BAUD          115200                  //UART1 transfer 波特率[8N1]
#define BSP_ADKEY_EN                    1                       //AD按键使能[Default: PB0]
```

- `BSP_UART_TRANSFER_EN=1`：编译期打开 UART1 transfer 功能。
- `BSP_UART_TRANSFER_BAUD=115200`：波特率，按对端设备确定，默认 115200。
- 不复用 `BSP_UART_DEBUG_EN`，也不通过本功能打开 HUART。

#### 3.2.2 头文件汇总 [bsp/bsp.h](../../bsp/bsp.h)

新增一行，让系统初始化和功能循环通过既有声明调用，不在业务代码直接访问 `UART1_REG`、GPIO 寄存器、crossbar 或中断标志：

```c
#include "bsp_uart_debug.h"
#include "bsp_uart_transfer.h"
```

#### 3.2.3 系统初始化 [bsp/bsp_sys.c](../../bsp/bsp_sys.c)

**首次上电**：在 `bsp_sys_init()` 中，`mic_bias_trim_init()` **之后**调用 `uart_transfer_init()`：

```c
    if (MIC_ANA_MODE == MIC_MODE_NOT_RC) {
        mic_bias_trim_init(MIC_VDD_LVMODE, MIC_IN_TRIM_TARGET);
    }

#if BSP_UART_TRANSFER_EN
    //放在 ADC 偏置 trim 之后：避免 UART1 RX 中断干扰 mic_bias_trim_init 的采样时序,导致 trim 超时看门狗复位
    uart_transfer_init(BSP_UART_TRANSFER_BAUD);
#endif
```

**低功耗唤醒**：在 `bsp_periph_init()` **末尾**（所有外设初始化之后）调用同一函数，使唤醒后的 GPIO/crossbar/UART1 状态由同一 BSP 函数恢复：

```c
void bsp_periph_init(void)
{
#if BSP_UART_DEBUG_EN
    bsp_uart_debug_init();
#endif
#if BSP_SARADC_EN
    bsp_saradc_init();
#endif
#if BSP_KEY_EN
    bsp_key_init();
#endif
#if BSP_VBAT_DETECT_EN
    bsp_vbat_detect_init();
#endif
#if BSP_TSEN_EN
    bsp_tsensor_init();
#endif // BSP_TSEN_EN
#if BSP_UART_TRANSFER_EN
    //放在所有外设初始化之后,避免 UART1 RX 中断干扰 SARADC 等外设的采样初始化
    uart_transfer_init(BSP_UART_TRANSFER_BAUD);
#endif
}
```

> **为什么放在最后**：`uart_transfer_init()` 会打开 UART1 RX 中断。若在 ADC 相关初始化（`mic_bias_trim_init`、`bsp_saradc_init`）之前打开，PA1 悬空或接未上电的 CH340 时，RX 噪声中断会打断 ADC 采样时序，导致 trim 超时触发看门狗复位（详见 [第 4 章](#4-排错文档)）。

#### 3.2.4 公共业务循环 [functions/func.c](../../functions/func.c)

在 `func_process()` 公共非中断循环中调用已有回显处理（发射端与适配器端都经过该公共循环）：

```c
AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();

    bt_run_loop();

#if BSP_UART_TRANSFER_EN
    uart_transfer_example();
#endif
    ...
}
```

- 调用位于普通主循环，**不放入** `uart_isr()`、1 ms/5 ms 定时器 ISR 或其他中断 section。
- 保持既有 `AT(.text.func.process)` 段放置不变。

#### 3.2.5 BSP 最小加固 [bsp/bsp_uart_transfer.c](../../bsp/bsp_uart_transfer.c)

**（1）初始化时复位接收软件状态**，避免睡眠前残留半包在唤醒后与新数据拼接：

```c
void uart_transfer_init(u32 baud)
{
    uart_init_typedef uart_init_struct;
    gpio_init_typedef gpio_init_structure;

    //复位接收软件状态，避免睡眠前残留半包在唤醒后与新数据拼接
    uart_rx.len = 0;
    uart_rx.tick = 0;
    uart_rx.done = 0;

    clk_gate0_cmd(CLK_GATE0_UART1, CLK_EN);
    clk_uart_clk_set(UART1_REG, CLK_UART_XOSC24M);
    // RX / TX GPIO 与 crossbar、8N1、UART_BAUD_FIX
    gpio_func_mapping_config(UART_RX_PORT, UART_RX_PIN, GPIO_CROSSBAR_IN_UART1RX);
    gpio_func_mapping_config(UART_TX_PORT, UART_TX_PIN, GPIO_CROSSBAR_OUT_UART1TX);
    uart_init(UART1_REG, &uart_init_struct);
    uart_pic_config(UART1_REG, uart_isr, 0, UART_IT_RX, ENABLE);
    uart_cmd(UART1_REG, ENABLE);
}
```

**（2）发送回显时加超时保护与快照长度**，避免 UART1 异常时阻塞喂不上看门狗：

```c
void uart_transfer_example(void)
{
    u8 *p = uart_rx.buf;
    if (((tick_get() - uart_rx.tick) >= uart_rx.timeout) && uart_rx.len) {
        printf("-->send\n");
        //用快照长度发送,避免 ISR 在发送期间并发改 len 导致永远发不完
        u16 left = uart_rx.len;
        bool tx_ok = true;
        while (left && tx_ok) {
            uart_send_data(UART1_REG, *p++);
            //等 TX 完成,带超时保护,避免 UART1 异常时阻塞喂不上看门狗
            u32 wait = 0;
            while (uart_get_flag(UART1_REG, UART_IT_TX) != SET) {
                if (++wait > 100000) {
                    tx_ok = false;
                    break;
                }
            }
            left--;
        }
        uart_rx.len = 0;
    }
}
```

**（3）中断服务函数 `uart_isr()` 保持不变**，`AT(.com_text.isr)` 标注原样保留。

### 3.3 调用链与数据流

```
上电
 └─ bsp_sys_init()
     ├─ ... xcfg_init / pmu / saradc / key / led / charge ...
     ├─ mic_bias_trim_init()          ← 先完成 ADC 偏置 trim
     └─ uart_transfer_init(115200)    ← 再开 UART1 RX 中断
 └─ func_run() → func_mic_emit()/func_adapter()
     └─ func_process()                ← 公共非中断循环
         ├─ WDT_CLR()
         ├─ bt_run_loop()
         └─ uart_transfer_example()   ← 50ms 空闲后回显收到的数据

接收：PA1 → crossbar UART1RX → uart_isr()（AT(.com_text.isr)）→ uart_rx.buf
发送：uart_transfer_example() 空闲超时后 → uart_send_data(UART1_REG) → PA0
低功耗唤醒：sfunc_sleep_exit() → bsp_periph_init()（末尾）→ uart_transfer_init()
```

---

## 4. 排错文档

### 4.1 现象

接入 UART1 transfer 后，下载运行出现**反复复位**，串口日志不断重启，复位原因为 `WDT_RST`（看门狗复位）；而将 UART1 功能关闭后恢复正常、不再复位。日志崩溃点在 `mic_bias_trim_init` 附近。

### 4.2 根因分析

- 初始实现把 `uart_transfer_init()` 放在了 `xosc_init()` 之后、`mic_bias_trim_init()` **之前**（`bsp_sys_init`），并在 `bsp_periph_init()` 中放在 `bsp_saradc_init()` 之前。
- `uart_transfer_init()` 会执行 `uart_pic_config(UART1_REG, uart_isr, 0, UART_IT_RX, ENABLE)`，**打开 UART1 RX 中断**。
- 此时 PA1（UART1 RX）若悬空，或接到未上电/空闲的 CH340 TXD（可能被拉低），会产生 RX 噪声中断，频繁打断后续 `mic_bias_trim_init()` 内部的 ADC 采样循环。
- ADC 偏置 trim 反复被中断打断、迟迟无法收敛 → trim 超时 → 看门狗超时 → `WDT_RST()` 复位，形成“反复复位”。

**因果链**：

```
UART1 RX 中断提前打开
  → PA1 噪声/低电平触发 RX 中断
    → 打断 mic_bias_trim_init 的 ADC 采样时序
      → trim 无法收敛、超时
        → 看门狗超时 WDT_RST()
          → 反复复位
```

### 4.3 修复方案

1. **调整初始化顺序（核心修复）**：
   - `bsp_sys_init()`：把 `uart_transfer_init()` 移到 `mic_bias_trim_init()` **之后**。
   - `bsp_periph_init()`：把 `uart_transfer_init()` 移到**所有外设初始化之后**（末尾）。
   - 使 ADC 相关初始化先完成，之后才打开 UART1 RX 中断，从根源上避免中断干扰采样时序。

2. **发送路径超时保护（加固）**：
   - `uart_transfer_example()` 中等待 TX 完成加 `wait > 100000` 超时退出，避免 UART1 异常时阻塞主循环、喂不上看门狗。
   - 用快照长度 `left` 发送，避免 ISR 并发修改 `uart_rx.len` 导致发不完。

3. **唤醒后状态复位（加固）**：
   - `uart_transfer_init()` 开头清 `uart_rx.len/tick/done`，避免睡眠前残留半包在唤醒后拼接。

---

## 5. 测试方法与验证

### 5.1 CH340 接线

| CH340 模块 | 核心板 | 说明 |
|---|---|---|
| RXD | **PA0**（芯片 TX） | 交叉接：CH340 收 ← 芯片发 |
| TXD | **PA1**（芯片 RX） | 交叉接：CH340 发 → 芯片收 |
| GND | GND | 必须共地 |
| VCC | —— | **不接**，用核心板自身 3.3 V 供电 |

- 双方波特率均为 **115200**、8N1。
- CH340 电平为 3.3 V 时直接接；若为 5 V TTL，需确认电平兼容，**禁止 5 V 直接输入芯片引脚**。
- 调试日志仍在 PB3（UART0，1.5 Mbps），与 PA0/PA1 的 UART1 相互独立，可同时观察。

### 5.2 收发测试步骤

1. 按 5.1 接好 CH340 与核心板，打开串口助手（PA0/PA1 对应端口），设 115200/8N1。
2. 上电，观察 PB3 调试口输出完整启动日志（见 5.3），确认进入 `func_mic_emit` 且 `WIRELESS_CONNECTED`。
3. 在串口助手中向 PA1 发送少于 32 字节、字节间隔小于 50 ms 的数据。
4. 约 50 ms 空闲后，芯片从 PA0 原样回显相同数据（当前 `uart_transfer_example()` 语义为“收到数据 → 空闲 50 ms → 原样回显”）。
5. 发送时 PB3 调试口会打印 `-->send` 标记（`uart_transfer_example()` 中的 `printf`，输出到 UART0 调试口，用于确认回显被触发）。
6. 验证边界：超过 32 字节按既有缓存容量截断；无数据不发送；接收中断不执行阻塞发送。

> **已验证结果**：下载测试成功；在另一个串口助手中测试，收、发均可正常工作，无反复复位。

### 5.3 验证证据（Debug 日志）

以下为修复后完整启动日志（UART0 / PB3 调试口）：

```text
Hello Platform
voltage_trim_value_dump:flag:1
vrtcbg:2
vbat:13
vbg:14
chg_icoef:10
vbat_cp_det:62
startup
pthd_alg: 16154,    15e50,300
Hello AB5766: 0821003a
SDK: v0110 LIBS: v0110
WDT_RST
INBOX_WK
SW_VERSION: V0.0.1
----------------------------------------------------
comm: range[0x10c00-0x1333b], size=9k
heap: range[0x15cd1-0x16cd3], size=4k
bss : range[0x13340-0x13d7b], size=2k
bram: range[0x13d7c-0x15cd0], size=7k
----------------------------------------------------
lock wireless_mic comm
vddio:10
vddcore:20
vddbt:9
lvd:2
-->ldo_en
vddbt_capless_en:1
mic_bias_trim_init
mic trim: dc:2160mV, dc diff:910mV, res_level:15, current:403uA
mic trim: dc:1557mV, dc diff:307mV, res_level:5, current:318uA
mic trim: dc:1429mV, dc diff:179mV, res_level:4, current:286uA
mic trim: dc:1219mV, dc diff:-31mV, res_level:3, current:253uA
mic trim finish: res_level:3
SDADC & SDDAC CLOSED
bsp_sys_init
func_run
func_mic_emit
<TX_INTERVAL>         4
<CON_INTERVAL>        120
<WIRELESS_FEAT>       18306
<WIRELESS_CODEC>      4
<FREQ_BAND>           0
<RETRY>               3
<DISCON_AUTO_PWROFF>  0
<CONFIG_RSSI>         70
<WS_NAME>             LE_MIC2
pa_gain:3, pa_vsel:9, pa_mode:1
pa_gain:3, pa_vsel:9, pa_mode:1
loc: 7, 8
dc 58 48 9a cb 23 2b 96 ad 19 66 89 fd d3 e4 2d 
c9 87 4c e8 7b 52 7d da 9d 85 d2 62 04 7f 3a 58 

addr: 8b 51 13 ac 42 41 
WIRELESS_CONNECTED, 0
==>vddcore: 20->24
loc_mic_pacc_init
loc_mic_pacc_enable
mic_eq_drc_init 4
dac0_out_init
m->res_level:3
TX: pdu=(50B, 594), intv=(5000, 594), retry=(3, 3)
rx_cntl(1): 01 cf 00 80 00 00 14 00 
rx_cntl(1): 01 cf 41 80 60 00 64 00 
rx_cntl(1): 01 cf 41 86 60 00 78 00 
```

关键证据点：

| 日志行 | 含义 |
|---|---|
| `WDT_RST` / `INBOX_WK` | 记录的上一次复位原因为看门狗复位（修复前复位风暴遗留），本次唤醒原因 INBOX |
| `mic_bias_trim_init` → `mic trim finish: res_level:3` | **mic trim 成功完成**（修复前卡在此处超时复位） |
| `bsp_sys_init` → `func_run` → `func_mic_emit` | 正常进入功能状态机循环 |
| `WIRELESS_CONNECTED, 0` | 无线连接成功 |

> 说明：`WDT_RST` 是本次启动读取到的“上次复位原因”寄存器值，来自修复前的复位风暴；修复后本次启动完整跑通（trim 完成、连接成功），不再反复复位。

UART1 回显触发的调试标记为 `-->send`（由 `uart_transfer_example()` 中的 `printf` 输出到 UART0 调试口）；UART1 实际回显数据则出现在 PA0 对应串口助手中。

---

## 6. 风险与后续边界

- 现有缓存固定 **32 字节**，回显发送为阻塞等待（已加超时保护），仅适合本轮基本收发验证；正式协议需单独设计帧格式与非阻塞发送策略。
- PA0/PA1 与 `IIS_IOMAP_0`（PA0=LRCLK、PA1=BCLK）存在条件性硬件复用冲突，若后续启用 IIS 该映射，必须停用该映射或停止 UART1，不能依赖“最后一次初始化覆盖”。
- 串口信号必须使用兼容电平，VUSB/5 V 不能直接作为 UART 电平输入。
- `projects/microphone/Output/` 及工程根目录的 `effect.c/.h` 为构建/资源生成产物，不视为手写源文件。
- 本文档结论基于真实硬件构建、烧录与串口助手的收发验证，非 map/静态链接推断。

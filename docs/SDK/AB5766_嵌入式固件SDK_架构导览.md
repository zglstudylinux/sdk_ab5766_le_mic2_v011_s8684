# AB5766 嵌入式固件 SDK 架构导览（写给小白的分层指南）

> **文档定位**：入门到中级之间，**补足**"导航"(`AB5766_LE_Mic_新手开发指南.md`)与"业务实现"(`AB5766_发射端_Mic_Emit_实现详解.md` / `AB5766_接收端_Adapter_实现详解.md`)之间的空档。
>
> **读完后能**：看懂目录结构、知道每一层做什么、能照葫芦画瓢地修改小功能（新增按键、新增算法、配置新方案）。
>
> **前置知识**：C 语言基础（指针、结构体、宏）+ 一点单片机概念（中断、寄存器）。
>
> **配套文档**：
> - 入门导航：[`AB5766_LE_Mic_新手开发指南.md`](AB5766_LE_Mic_新手开发指南.md)
> - 发射端实现：[`AB5766_发射端_Mic_Emit_实现详解.md`](AB5766_发射端_Mic_Emit_实现详解.md)
> - 接收端实现：[`AB5766_接收端_Adapter_实现详解.md`](AB5766_接收端_Adapter_实现详解.md)
> - 验收：[`AB5766_按键链路测试与验收指南.md`](AB5766_按键链路测试与验收指南.md)、[`AB5766_无线麦克风_双板连接与LED测试指南.md`](AB5766_无线麦克风_双板连接与LED测试指南.md)

---

## 第一篇 开篇与全景图

### 第 1 章 · 写给小白的开场白

**你好，欢迎来到 AB5766 嵌入式固件的世界。**

如果你点开仓库，看到 `header/`、`driver/`、`bsp/`、`modules/`、`functions/`、`os/` 一堆文件夹，**目录路径基本都看不懂**，那么这份文档就是为你准备的。

我会带你：

1. **认识 SDK 的五层架构**（HAL → Driver → BSP → Module → Application）
2. **理解每一层做什么**（用真实的代码片段作为例子）
3. **看清一次按键、一帧音频在 SDK 中如何流转**（带图示）
4. **学会照葫芦画瓢改小功能**（4 个实战示例）

最后会带你了解 **OSAL 操作系统抽象层**、**STRONG/WEAK 符号覆盖**、**AT() 段属性宏**（这些名词听起来高大上，但其实就是解决具体工程问题的工具）。

> **提示**：本教程的所有代码引用都用 `文件:行号` 标注，可以直接 `Ctrl+G` 跳转到源代码阅读。

### 第 2 章 · 嵌入式 SDK 是什么？为什么要分层？

**什么是 SDK？**

SDK（Software Development Kit）是芯片厂商提供的"软件工具箱"。它包含：

- **可重用的代码库**：别人写好的音频处理、蓝牙协议栈、按键扫描等
- **抽象接口**：把"操作寄存器"包装成"调函数"，让上层不用关心硬件细节
- **构建工具**：编译器选项、链接脚本、烧录脚本

**为什么要分层？想象你在搭乐高积木：**

| 不分层 | 分层 |
|---|---|
| 业务代码直接操作寄存器 | 业务代码只调 `bsp_*_init()` 这种高层接口 |
| 换芯片要重写所有业务 | 换芯片只改 `driver/` + `bsp/` |
| 算法代码与硬件耦合 | 算法只调标准 `audio_callback_t` 回调 |
| 不可复用 | 算法可以跨产品复用 |

**分层最大收益**：**换芯片不动业务，换业务不动底层**。

AB5766 这套 SDK 也是按照这套"乐高"思想组织的。你打开 `header/include.h` 就能看到整个 SDK 的"总目录"：

```c
// header/include.h:1-24
#include "global.h"       // 基础类型
#include "xcfg.h"         // 产测配置
#include "config.h"       // 产品配置

#include "api.h"          // SDK 公共 API
#include "bsp.h"          // BSP 层头
#include "func.h"         // 应用层头
#include "driver_com.h"   // 驱动层头
#include "modules.h"      // 模块层头
```

**包含顺序即分层顺序**——这本身就是 SDK 给我们立的"规矩"。

### 第 3 章 · 一张图看懂 AB5766 SDK 的五层架构

打开 `header/include.h`，你会发现 SDK 包含四个核心层 + 一个 OSAL 横向贯穿 + 一套 STRONG/WEAK 横向覆盖机制：

```mermaid
flowchart TB
    subgraph app["应用层 functions/"]
        func_run[func_run<br/>func.c:134]
        func_mic_emit[func_mic_emit<br/>func_mic_emit.c:212]
        func_adapter[func_adapter<br/>func_adapter.c:222]
        msg_dispatch[msg_mic_emit.c / msg_adapter.c<br/>消息分派]
        func_run --> func_mic_emit
        func_run --> func_adapter
        func_mic_emit --> msg_dispatch
        func_adapter --> msg_dispatch
    end
    subgraph mod["模块层 modules/"]
        wireless[wireless/<br/>mic_proc.c 无 mic 编码 + RX 解码]
        audio[audio/<br/>EQ/DRC/soft_gain/dac0_out]
        voice[voice/<br/>echo/magic/plc 等算法]
        codec[codec/<br/>lc3s 编解码]
        wireless --> audio
        wireless --> voice
        wireless --> codec
    end
    subgraph bsp["BSP 层 bsp/"]
        bsp_sdadc[bsp_sdadc.c<br/>SDADC 板级适配]
        bsp_key[bsp_key.c<br/>按键扫描]
        bsp_sys[bsp_sys.c<br/>系统初始化入口]
        bsp_sdadc --> bsp_sys
        bsp_key --> bsp_sys
    end
    subgraph drv["驱动层 driver/"]
        driver_sdadc[driver_sdadc.c<br/>SDADC 寄存器操作]
        driver_keyscan[driver_keyscan.c<br/>KEYSCAN 寄存器操作]
        driver_clk[driver_clk.c<br/>时钟门控]
        driver_sdadc --> driver_clk
        driver_keyscan --> driver_clk
    end
    subgraph hal["HAL 层（最底层）"]
        sfr[sfr.h<br/>SFR 寄存器地址表]
        sdadc_typedef[sdadc_typedef<br/>typedef 结构体包装 SFR]
        driver_sdadc --> sdadc_typedef
        driver_keyscan --> sdadc_typedef
        sdadc_typedef --> sfr
    end
    subgraph osal["OSAL 层 os/ + libs/cpu"]
        os_thread[os_thread.h<br/>thread_alg.c thread_dec.c]
        api_msg[api_msg.h<br/>消息队列]
        api_sdadc[api_sdadc.h<br/>线程 kick 投递]
    end
    app -.调用.-> mod
    app -.调用.-> bsp
    bsp --> drv
    drv --> hal
    mod -.使用.-> osal
    bsp -.使用.-> osal
    classDef appc fill:#cfe2ff,stroke:#0d6efd
    classDef modc fill:#d1e7dd,stroke:#198754
    classDef bspc fill:#fff3cd,stroke:#ffc107
    classDef drvc fill:#f8d7da,stroke:#dc3545
    classDef halc fill:#e2e3e5,stroke:#6c757d
    classDef osalc fill:#fce4ec,stroke:#c2185b
    class func_run,func_mic_emit,func_adapter,msg_dispatch appc
    class wireless,audio,voice,codec modc
    class bsp_sdadc,bsp_key,bsp_sys bspc
    class driver_sdadc,driver_keyscan,driver_clk drvc
    class sfr,sdadc_typedef halc
    class os_thread,api_msg,api_sdadc osalc
```

**怎么读这张图？**

- **纵向箭头 = 依赖方向**：上层只能调下层，不能反过来
- **应用层** 用 `functions/` 下的状态机
- **模块层** 在 `modules/` 下，按"领域"组织（音频、语音、编解码等）
- **BSP 层** 在 `bsp/` 下，把 HAL 封装成"板级功能"
- **驱动层** 在 `driver/` 下，每个外设一个 .c
- **HAL 层** 即 `header/sfr.h` 中的寄存器地址定义 + `driver_xxx.h` 中的 `typedef` 包装
- **OSAL 层**（右侧）横向贯穿，提供消息队列、线程间异步投递

> **新手常见误区**：直接把 `bsp_key_scan()` 当作"最简单的 API"——其实它内部已经调用了 `driver_keyscan.c` + SARADC ADC + 5ms 定时器 + 消息队列 + 状态机分派，是横跨四层的"集成产物"。

### 第 4 章 · 仓库目录速览

下面这张表让你一打开仓库就知道每个文件夹是干什么的。

| 目录 | 职责 | 关键文件举例 |
|---|---|---|
| `header/` | 公共类型、宏、聚合头文件 | `typedef.h`（u8/u16/u32）、`macro.h`（AT() 宏）、`sfr.h`（寄存器地址）、`include.h`（融合头） |
| `driver/` | 寄存器级 HAL（每个外设一个 .c） | `driver_sdadc.c`、`driver_sddac.c`、`driver_uart.c`、`driver_keyscan.c`、`driver_clk.c` 等 24 个 |
| `bsp/` | 板级支持包（组合 driver） | `bsp_sys.c`、`bsp_sdadc.c`、`bsp_key.c`、`bsp_led.c`、`bsp_charge.c` 等 19 个 |
| `modules/` | 可复用协议 + 算法 | `wireless/`（D2A 协议）、`audio/`（EQ/DRC/增益）、`voice/`（7 种语音算法）、`codec/`（lc3s）、`effect/`、`device/`、`tool/` |
| `functions/` | 应用层状态机 | `func.c`（super-loop）、`func_mic_emit.c`、`func_adapter.c`、`msg_mic_emit.c`、`msg_adapter.c` |
| `os/` | 轻量级线程抽象 | `os_thread.h`、`thread_alg.c`、`thread_dec.c` |
| `projects/microphone/` | 项目入口 + 配置 + 链接脚本 | `main.c`、`config.h`、`config_ab5766_le_mic.h`、`xcfg.h`、`ram.ld`、`app.cbp`、`strong_symbol.c`、`strong_ble.c`、`port/port_key.c` |
| `libs/` | 预编译静态库（不要改） | `libbtstack.a`（BLE 协议栈）、`libplatform.a`、`libm.a`、`libc.a`、`libgcc.a` |
| `docs/` | 中文文档 | `SDK/` 5 份文档 + `plan/` 实施计划 |

**`libs/cpu/api_*.h`** 是 HAL 的"墙头"——库对外暴露的 17 个 API 头：
- `api_msg.h`（消息队列）
- `api_sdadc.h` / `api_sddac.h`（音频 ADC/DAC）
- `api_usb.h`（USB Device）
- `api_codec.h`（编解码）
- `api_pwr.h`（功耗）
- `api_flash.h`（Flash 读写）
- `api_sys.h` / `api_sysclk.h`（系统时钟）
- `api_pacc.h`（PACC 硬件加速器）
- `api_interrupt.h`（中断）
- `api_os.h` / `api_key.h`（OS/按键）
- `api_smoke.h`（冒烟测试）
- `api_update.h`（OTA）
- `api_cm.h`（CM 参数区）
- `api_list.h`（链表）

**这 17 个 .h 是 SDK 的"对外合同"**——你的应用代码只能调用这里声明的函数，不能直接访问 SFR。

---

## 第二篇 自底向上：五层逐层拆解

### 第 5 章 · HAL 层：直接和芯片寄存器对话的"指尖"

**HAL（Hardware Abstraction Layer）** 是最底层。它解决两个问题：

1. **把寄存器地址抽象成 C 结构体**——让代码可读
2. **提供一组"只做一件事"的函数**——让上层不用关心寄存器细节

**案例：`driver_sdadc.c`（SDADC = Sigma-Delta ADC，麦克风采样）**

```c
// header/sfr.h:25-30  寄存器地址定义
#define SFR_BASE   0x00000100
#define SFR0_BASE  (SFR_BASE + 0x000)
#define SFR1_BASE  (SFR_BASE + 0x100)
#define SFR2_BASE  (SFR_BASE + 0x200)
...

// driver/driver_sdadc.h:18-30  typedef 把寄存器"打包"成结构
typedef struct {
    volatile unsigned long CON;        // SDADC_CON 寄存器
    volatile unsigned long DMA_ADR;    // SDADC DMA 地址
    volatile unsigned long DMA_CNT;    // SDADC DMA 计数
    volatile unsigned long DMA_PEND;   // SDADC DMA 完成 pending
    // ... 其它字段
} sdadc_typedef;

// driver/driver_sdadc.c:194-201  HAL 函数只做一件事
AT(.com_text.sdadc.isr)
void sdadc0_cmd(SDADC_REG_TYPEDEF *sdadc, u8 en) {
    if (en) {
        sdadc->CON |= SDADC0CON_ADC0_EN;   // 打开 ADC0 通道
    } else {
        sdadc->CON &= ~SDADC0CON_ADC0_EN;  // 关闭 ADC0 通道
    }
}
```

**关键观察**：

| 特征 | 体现 |
|---|---|
| **HAL 不知 OS** | 没有 `delay`、没有 `semaphore`、没有上下文切换 |
| **HAL 不带业务** | `sdadc0_cmd` 只置/清一个 `ADC0_EN` 位，不关心是麦克风还是电子烟 |
| **HAL 不可重入** | `sdadc->CON |= BIT` 不是原子操作，调用方需自行保证临界区 |
| **HAL 一致结构** | 每个 `driver_xxx.h` 都暴露一个 `xxx_typedef *` 结构体 |

```mermaid
flowchart LR
    A[sfr.h<br/>寄存器地址常量] --> B[driver_xxx.h<br/>typedef 包装为结构体]
    B --> C[driver_xxx.c<br/>HAL 函数<br/>只读写一个字段]
    C --> D[上层调用者<br/>如 BSP / Module]
    classDef hal fill:#e2e3e5,stroke:#6c757d
    classDef up fill:#cfe2ff,stroke:#0d6efd
    class A,B,C hal
    class D up
```

> **新手误区**：在 HAL 函数里调 `printf`！这会调用 UART 驱动，可能形成循环依赖或阻塞 ISR。HAL 必须保持纯净。

### 第 6 章 · Driver 层：29 个驱动的"小工具箱"

**Driver 层** 提供"外设控制台"。本项目里 Driver 和 HAL 合并（每个 .c 既定义 typedef 又写函数），这是嵌入式常见做法。

`driver/` 下 24 个 `.c` 按功能分桶：

```mermaid
mindmap
  root((driver 层 24 个文件))
    音频
      driver_sdadc
      driver_sddac
      driver_saradc
      driver_iis
    通信
      driver_uart
      driver_hsuart
      driver_spi
      driver_iic
      driver_despi
    按键
      driver_keyscan
      driver_touch_key
    电源时钟
      driver_clk
      driver_lowpwr
      driver_charge
      driver_wdt
      driver_rtc
    控制
      driver_gpio
      driver_pwm
      driver_qdec
      driver_irtx
      driver_irrx
      driver_ledc
    辅助
      driver_tmr
```

**有哪些"知名"驱动？**

| 驱动 | 作用 | 在 SDK 中出现的位置 |
|---|---|---|
| `driver_sdadc.c` | 麦克风 ADC 采样 | `bsp_sdadc.c` 调 |
| `driver_sddac.c` | 喇叭 DAC 输出 | `bsp_sddac.c` 调 |
| `driver_keyscan.c` | 硬件按键扫描 | `bsp_key.c` 调 |
| `driver_uart.c` | 通用 UART | `bsp_uart_debug.c` 调 |
| `driver_clk.c` | 时钟门控 | 几乎所有驱动调 |

**Driver 层的"风格"：**

```c
// driver/driver_sdadc.c:194-201  典型 Driver 函数
void sdadc0_cmd(SDADC_REG_TYPEDEF *sdadc, u8 en) {
    if (en) {
        sdadc->CON |= SDADC0CON_ADC0_EN;
    } else {
        sdadc->CON &= ~SDADC0CON_ADC0_EN;
    }
}

// driver/driver_sdadc.c:234-264  典型 Driver 初始化
void sdadc_pic_config(SDADC_REG_TYPEDEF *sdadc,
                      sdadc_isr_callback cb,
                      int pr, u32 flags, u8 en) {
    sdadc->CON = (sdadc->CON & ~BIT(1)) | (pr & 0x1) << 1;
    sdadc_isr_cb = cb;                    // 保存 ISR 回调
    // ... 详细 SFR 配置
}
```

每个 Driver 的"标准三件套"：

1. **结构体 typedef**（如 `sdadc_typedef`）
2. **寄存器初始化函数**（如 `sdadc_init` / `sdadc_ana_init`）
3. **运行时操作函数**（如 `sdadc0_cmd` / `sdadc_dma_cmd` / `sdadc_get_flag`）

> **新手建议**：刚开始不要试图读懂所有 Driver 的 SFR 配置。**只关注函数名和参数即可**——能看懂"调哪个函数能做什么"就够了。

### 第 7 章 · BSP 层：把 HAL 装成"功能模块"的工程师

**BSP（Board Support Package）** 是把"裸 Driver"装成"功能模块"的关键。它做三件事：

1. **配置 HAL 时钟**
2. **注册 ISR 回调**
3. **管理 DMA 缓冲**

**教科书案例：`bsp/bsp_sdadc.c:55 bsp_sdadc_init()` 是 5 步标准流程：**

```c
// bsp/bsp_sdadc.c:55-105  完整初始化流程
void bsp_sdadc_init(void) {
    sdadc_init_typedef sdadc_cfg_init;
    sdadc_ana_cfg_typedef sdadc_ana_cfg;

    // 步骤 1: 注册算法回调（"数据采完调我"）
    sdadc_done_callback_set(sdadc_proc_cb);                    // line 60

    // 步骤 2: 开 HAL 时钟
    clk_gate0_cmd(CLK_GATE0_SDADC, CLK_EN);                   // line 62
    clk_sdadda_clk_set(CLK_SDADDA_CLK48_A);                   // line 63

    // 步骤 3: 用 HAL 接口配置参数
    sdadc_cfg_init.adc_mode = SDADC_MODE_MIC;                  // line 65
    sdadc_cfg_init.sample_rate = WIRELESS_MIC_SAMPLE_RATE_SELECT;
    sdadc_cfg_init.dig_gain = dig_gain_tbl[MIC_DIG_GAIN];
    sdadc_init(SDADC_REG, &sdadc_cfg_init);                    // line 88
    sdadc_ana_cfg.adc_mode = MIC_ANA_MODE;
    sdadc_ana_init(&sdadc_ana_cfg);                             // line 96

    // 步骤 4: 启动 DMA + 注册 ISR
    sdadc_w4_kick = true;
    sdadc_pic_config(SDADC_REG, sdadc_isr_cb, 0,
                     SDADC_FLAG_DMA_HALF_DONE | SDADC_FLAG_DMA_ALL_DONE, ENABLE);  // line 100
    sdadc_dma_cmd(SDADC_REG, (u32)sdadc_buf, SDADC_DMA_SIZE-1,
                 DMA_DATA_MODE_16BITS, ENABLE);                // line 101

    // 步骤 5: 启动时基同步
    wl_tick_time_init();                                       // line 102
}
```

```mermaid
flowchart TD
    A[bsp_sdadc_init 开始] --> B[步骤 1: sdadc_done_callback_set 注册算法回调]
    B --> C[步骤 2: clk_gate0_cmd + clk_sdadda_clk_set 开 HAL 时钟]
    C --> D[步骤 3: sdadc_init + sdadc_ana_init 配置参数]
    D --> E[步骤 4: sdadc_pic_config + sdadc_dma_cmd<br/>注册 ISR + 启动 DMA]
    E --> F[步骤 5: wl_tick_time_init 启动时基]
    F --> G[完成 - 等待 DMA 中断触发 sdadc_proc_cb]
    classDef step fill:#fff3cd,stroke:#ffc107
    class B,C,D,E,F step
```

**BSP 全家桶（19 个文件）**

| 文件 | 作用 |
|---|---|
| `bsp_sys.c` | 系统初始化总入口（`bsp_sys_init()`） |
| `bsp_key.c` | 按键扫描 + 消息队列投递 |
| `bsp_sdadc.c` / `bsp_sddac.c` | 麦克风采集 / 喇叭输出 |
| `bsp_saradc.c` / `bsp_saradc_key.c` / `bsp_saradc_vbat.c` | SARADC 通用 / 按键 / 电池电压 |
| `bsp_led.c` / `bsp_pwm.c` / `bsp_tone.c` | LED、蜂鸣器、提示音 |
| `bsp_charge.c` | 充电管理 |
| `bsp_uart_debug.c` / `bsp_uart_transfer.c` | 调试 UART / 数据传输 UART |
| `bsp_param.c` | Flash 参数读写 |
| `bsp_io_key.c` | IO 矩阵按键 |
| `bsp_irrx.c` / `bsp_irtx.c` | 红外接收 / 发射 |
| `bsp_huart.c` | 高速 UART |
| `bsp_le_dut.c` | BLE DUT 测试 |
| `bsp_iis.c` | IIS 音频总线 |

**BSP 与 Driver 的边界**：

```c
// BSP 函数（bsp_sdadc.c）
void bsp_sdadc_init(void);     // 高级、板级初始化
void bsp_sdadc_kick(void);     // 启动 DMA 采集

// Driver 函数（driver_sdadc.c）
void sdadc_init(SDADC_REG_TYPEDEF *sdadc, sdadc_init_typedef *cfg);
void sdadc_ana_init(sdadc_ana_cfg_typedef *cfg);
```

**判断标准**：

- **BSP 函数**：接收"业务级参数"（如 `WIRELESS_MIC_SAMPLE_RATE_SELECT`），内部调多个 HAL/Driver 函数
- **Driver 函数**：只操作一个外设，不关心业务

### 第 7 章附录 · LED 显示子系统：分层解耦全景（按调用链核查）

> 本附录把 LED 显示子系统当作 BSP 层的第二个教科书案例，完整梳理其**五层解耦架构**与**全部调用关系**，所有引用均精确到 `文件:行号:函数`，并经实际代码确认。文末另列三个**解耦破缺点**（死接口 / 死字段 / 引脚冲突），与 [../plan/AB5766_LE_Mic_分阶段实施计划.md](../plan/AB5766_LE_Mic_分阶段实施计划.md) 阶段 2 的实测发现互为印证。

#### A. 五层解耦架构总览

LED 子系统从上到下分为五层，层与层之间只通过明确定义的接口耦合，任意一层可被替换而不牵连他层（引脚冲突是唯一破坏这一隔离的破缺点，见 E 节）：

| 层 | 职责 | 关键文件 / 符号 | 与下一层的耦合点 |
|---|---|---|---|
| ① **配置层** | 运行时决定「用不用灯、灯在哪个 IO、闪什么图样」 | [xcfg.h:50-81](../../projects/microphone/xcfg.h) `xcfg_cb` 字段 | 被 ②③④读取 |
| ② **编译期开关层** | 编译期决定「编不编 LED、编不编数码管、UART 占哪个脚」 | [config_ab5766_le_mic.h:24/30/31](../../projects/microphone/config_ab5766_le_mic.h) | 包裹 ③④⑤的 `#if` |
| ③ **BSP LED 驱动层** | 状态机 + 图样消耗器：把「图样数据」翻译成「亮灭动作」 | [bsp_led.c](../../bsp/bsp_led.c) / [bsp_led.h](../../bsp/bsp_led.h) | 调 ④的 `gpio_*` |
| ④ **GPIO HAL/Driver 层** | 寄存器级：把「亮灭动作」落到具体引脚电平 | [driver_gpio.c:15](../../driver/driver_gpio.c) `gpio_init` 等 | 操作 SFR |
| ⑤ **业务调用层 + 定时器调度层** | 决定「什么时候设什么图样」「什么时候扫描」 | [wireless_proc.c:319](../../modules/wireless/wireless_proc.c) / [func_*.c](../../functions/) / [bsp_sys.c:61](../../bsp/bsp_sys.c) / [bsp_charge.c:110](../../bsp/bsp_charge.c) | 调 ③的公开接口 |

```mermaid
flowchart TD
    subgraph L1["① 配置层 xcfg_cb（运行时 / Flash 加载）"]
        A1["bled_disp_en / bled_io_sel<br/>rled_disp_en / rled_io_sel<br/>led_*_config_en + 4×led_cfg_t<br/>xcfg.h:50-81"]
    end
    subgraph L2["② 编译期开关层 config_ab5766_le_mic.h"]
        B1["BSP_LED_EN=1 :30<br/>BSP_DLED_EN=0 :31<br/>BSP_UART_DEBUG_EN=GPIO_PB3 :24"]
    end
    subgraph L3["③ BSP LED 驱动层 bsp_led.c/h"]
        C1["led_cb_t 状态机<br/>led_set_sta_p 只写 bled_sta :197<br/>led_scan 只消耗 bled_sta :246"]
        C2["led_init / bled_set_on/off<br/>rled_set_on/off / led_charge_on/off"]
    end
    subgraph L4["④ GPIO HAL/Driver 层 driver_gpio.c"]
        D1["gpio_init :15<br/>gpio_set_bits / reset_bits / set_de"]
    end
    subgraph L5["⑤ 业务调用层 + 定时器调度层"]
        E1["wireless_mic_proc :319 切 idle/connected"]
        E2["func_mic_emit_init :37 / func_adapter_init :58 预设 idle"]
        E3["bsp_charge_process :110 驱动红灯"]
        E4["usr_tmr5ms_thread_do :61 每 50ms 调 led_scan"]
        E5["func_pwroff :407 关机 bled_set_off"]
    end

    L1 --> L3
    L2 -->|"#if 包裹"| L3
    L2 -->|"#if 包裹"| L4
    L3 --> L4
    L5 -->|"调公开接口"| L3

    classDef cfg fill:#cfe8ff,stroke:#0d6efd
    classDef sw fill:#fff3cd,stroke:#ffc107
    classDef drv fill:#d1e7dd,stroke:#198754
    classDef hal fill:#f8d7da,stroke:#dc3545
    classDef biz fill:#e2d9f3,stroke:#6f42c1
    class A1 cfg
    class B1 sw
    class C1,C2 drv
    class D1 hal
    class E1,E2,E3,E4,E5 biz
```

#### B. 各层详解（精确到文件:函数:行号）

**① 配置层 `xcfg_cb` —— 数据，不是代码**

LED 的「用不用、在哪个脚、闪什么图样」全部以位域形式存于运行时配置结构 `xcfg_cb_t`，由 ToolKit 界面写入 `wireless_mic_emit.setting`、经 `prebuild.bat` 生成 `xcfg.bin`，运行时由 `xcfg_init()` 加载：

- 使能与 IO 选择（[xcfg.h:50-53](../../projects/microphone/xcfg.h)）：
  - `bled_disp_en`（:50，蓝灯使能）、`bled_io_sel`（:51，蓝灯 IO，PA0=160 / PB3=179 / PB4=180…）
  - `rled_disp_en`（:52，红灯使能）、`rled_io_sel`（:53，红灯 IO）
- 四套状态图样的「是否自定义 + 图样本体」（[xcfg.h:54-81](../../projects/microphone/xcfg.h)）：每个 `led_cfg_t{redpat,bluepat,unit,cycle}` 子结构前都有一个 `*_config_en` 开关位（`led_pwron_config_en`/`led_pwroff_config_en`/`led_btpair_config_en`/`led_btconn_config_en`）。
- 充电相关（影响红灯）：`charge_en`（[xcfg.h:26](../../projects/microphone/xcfg.h)）、`charge_working_while_charging`（[xcfg.h:30](../../projects/microphone/xcfg.h)）。
- 加载点：[bsp_sys.c:202](../../bsp/bsp_sys.c) `bsp_sys_init()` 首行 `xcfg_init(&xcfg_cb, sizeof(xcfg_cb))`，先于一切 LED 初始化。
- 工具侧来源：[wireless_mic_emit.setting:79-81](../../projects/microphone/Output/bin/Settings/wireless_mic_emit.setting)（`bled_disp_en`/`bled_io_sel`/`rled_disp_en`/`rled_io_sel`）、[xcfg.xm:64-80](../../projects/microphone/Output/bin/xcfg.xm)（`BLED_DISP_EN`/`BLED_IO_SEL`/`RLED_DISP_EN`/`RLED_IO_SEL` 及四套 `config(LED,…)`）。

> **要点**：配置层是纯数据。改灯的「行为」不需要改代码，只改 ToolKit 配置即可——这是 LED 子系统最核心的解耦点。

**② 编译期开关层 —— 决定哪些代码参与编译**

- `BSP_LED_EN=1`（[config_ab5766_le_mic.h:30](../../projects/microphone/config_ab5766_le_mic.h)）：整个 [bsp_led.c](../../bsp/bsp_led.c) 的总开关，`#if BSP_LED_EN` 包裹（[bsp_led.c:5](../../bsp/bsp_led.c)）。
- `BSP_DLED_EN=0`（[config_ab5766_le_mic.h:31](../../projects/microphone/config_ab5766_le_mic.h)）：数码管/双灯扫描器 `dled_*` 一套不编译（[bsp_led.c:261-348](../../bsp/bsp_led.c) 整段被 `#if BSP_DLED_EN` 排除）。当 `BSP_DLED_EN=0` 时，`led_scan` 才由 50ms 周期驱动；若 `BSP_DLED_EN=1` 则改由 `dled_scan_10ms` 接管（见 D 节调度）。
- `BSP_UART_DEBUG_EN=GPIO_PB3`（[config_ab5766_le_mic.h:24](../../projects/microphone/config_ab5766_le_mic.h)）：**与 LED 隐式冲突的编译期占用**——PB3 被编译期锁定为 UART0 TX 调试脚，蓝灯再配 PB3 即触发 E 节破缺。
- IO 端口/引脚宏：`GPIO_PA0=0xA0`、`GPIO_PB3=0xB3`、`GPIO_PB4=0xB4`（[config_define.h](../../projects/microphone/config_define.h)），`GPIO_PORT_GET(x)`（高 4 位判 A/B 口）、`GPIO_PIN_GET(x)`（低 4 位算位掩码）。
- ③层对配置的展开：[bsp_led.h:6-9](../../bsp/bsp_led.h) `BSP_BLED_PORT=GPIO_PORT_GET(xcfg_cb.bled_io_sel)`、`BSP_BLED_PIN=GPIO_PIN_GET(...)`、红灯同理——**每次访问都实时读 `xcfg_cb`**，故改 IO 无需重编译。

**③ BSP LED 驱动层 `bsp_led.c/h` —— 状态机 + 图样消耗器**

这是解耦的中枢。核心数据结构两个，均在 [bsp_led.h](../../bsp/bsp_led.h)：

- `led_cfg_t{redpat,bluepat,unit,cycle}`（[bsp_led.h:12-17](../../bsp/bsp_led.h)）：**图样是数据**——一个 8 位位图（`bluepat`/`redpat`，bit=亮）、一个 50ms 单位（`unit`）、一个间隔周期（`cycle`）。四份默认 const 表在 [bsp_led.c:9-16](../../bsp/bsp_led.c)（`led_cfg_poweron/poweroff/pairing/connected`）。
- `led_cb_t{rled_sta,bled_sta,unit,period,offset:init,circle,busy,lowbat,charge_flag,led2_flag}`（[bsp_led.h:19-34](../../bsp/bsp_led.h)）：运行态。**`rled_sta` 是死字段**（见 E 节）。

驱动函数及其行号级职责：

| 函数（文件:行号） | 职责 / 关键行为 |
|---|---|
| `led_init` [bsp_led.c:36](../../bsp/bsp_led.c) | `memset led_cb`；若蓝红 IO 相同则 `led2_flag=1`（共脚双色，L40-44）；对蓝灯、红灯各调一次 `led_cfg_init` 把 IO 配成数字输出 GPIO |
| `led_cfg_init` [bsp_led.c:19](../../bsp/bsp_led.c) | `gpio_init` 设 `OUTPUT+FEN_GPIO+FDIR_SELF+DIGITAL`——**覆盖 UART TX 交叉开关的根因点**（E 节） |
| `led2_port_out_mode` [bsp_led.c:55](../../bsp/bsp_led.c) | 共脚时靠 `gpio_set_de` 切模拟/数字模式推双色 |
| `bled_set_on/off` [bsp_led.c:62/77](../../bsp/bsp_led.c) | L64/L79 `if(!xcfg_cb.bled_disp_en) return`（禁用安全保护）；独立脚时 `gpio_set_bits` 高电平点亮 / `gpio_reset_bits` 低电平熄灭；共脚时靠模式切换 |
| `rled_set_on/off` [bsp_led.c:91/102](../../bsp/bsp_led.c) | 高电平点亮（`gpio_set_bits`）/ `gpio_reset_bits` 熄灭；`static`，仅本文件与 `led_charge_*`/`dled_scan` 用 |
| `led_charge_on/off` [bsp_led.c:112/122](../../bsp/bsp_led.c) | `on`：`charge_flag=true`，非共脚先 `bled_set_off` 再 `rled_set_on`；`off`：`rled_set_off` + `charge_flag=false` |
| `led_power_up/down` [bsp_led.c:129/140](../../bsp/bsp_led.c) | 按 `led_*_config_en` 选 xcfg 字段或 const 表，调 `led_set_sta`——**全仓无调用方，死接口**（E 节） |
| `led_bt_idle` [bsp_led.c:151](../../bsp/bsp_led.c) | 配对态图样（未连接） |
| `led_w4_end` [bsp_led.c:162](../../bsp/bsp_led.c) | 等 `led_cb.busy`（图样播完） |
| `led_bt_connected` [bsp_led.c:171](../../bsp/bsp_led.c) | 已连接图样 |
| `led_set_sta_normal` [bsp_led.c:182](../../bsp/bsp_led.c) | `→ led_set_sta_p(cfg, &led_cb)` |
| `led_set_sta_p` [bsp_led.c:189](../../bsp/bsp_led.c) | **核心写入**：L197 `s->bled_sta = sta.bluepat`（**只写 bled_sta，从不写 rled_sta**）；L201-206 `circle = unit*8 + period`（`cycle==0xff` 时 `circle=0x7fff` 永久）；L208-209 算 `offset/init` 对齐 tick |
| `led_scan` [bsp_led.c:214](../../bsp/bsp_led.c) | **核心消耗**：L218 禁用保护；L229 `if(led_cb.charge_flag) return`（充电早退）；L246 `if(s->bled_sta & BIT(bcnt))` 位图消耗 → 调 `bled_set_on/off`（**只读 bled_sta，从不读 rled_sta、从不调 rled_set_***） |
| `led_set_sta` 宏 [bsp_led.h:36](../../bsp/bsp_led.h) | `#define led_set_sta(cfg) led_set_sta_normal(cfg)` 间接转发 |

**④ GPIO HAL/Driver 层 `driver_gpio.c`**

[driver_gpio.c:15](../../driver/driver_gpio.c) `gpio_init(gpiox, cfg)` 逐位配 `dir/de/fen/fdir/pupd/drv`——这是把 `fen`（功能使能）从「外设交叉开关」改写成「普通 GPIO」的地方，也是 PB3 卡死链路的物理执行点。`gpio_set_bits`（置高，点亮）、`gpio_reset_bits`（置低，熄灭）、`gpio_set_de`（数字/模拟使能，共脚用）是③层唯一直接触碰的三个 GPIO 接口。

**⑤ 业务调用层 + 定时器调度层**

LED 的「何时设何图样、何时扫描」完全由业务层驱动，③层自身是无源的状态机：

- **定时器调度（扫描的节拍源）**：[strong_symbol.c:54-58](../../projects/microphone/strong_symbol.c) `usr_tmr5ms_thread_callback() → usr_tmr5ms_thread_do()` 是 5ms 线程的 STRONG 入口；[bsp_sys.c:61-97](../../bsp/bsp_sys.c) `usr_tmr5ms_thread_do()` 在 L71-77 `#if (BSP_LED_EN && !BSP_DLED_EN)` 下以 `tmr5ms_cnt%10==0` 即**每 50ms** 调一次 `led_scan(tick_get())`。
- **初始化链**：[bsp_sys.c:200-267](../../bsp/bsp_sys.c) `bsp_sys_init()` 顺序 `xcfg_init`(L202) → … → `led_init()`(L224) → `bsp_charge_init()`(L231) → `printf("%s\n",__func__)`(L264)。唤醒后外设重初始化走 [bsp_sys.c:269-290](../../bsp/bsp_sys.c) `bsp_periph_init()`（L272 `bsp_uart_debug_init`），由 [func_lowpwr.c:263-291](../../functions/func_lowpwr.c) `sfunc_sleep_exit()` L274 调用。
- **角色进入预设**：[func_mic_emit.c:37-49](../../functions/func_mic_emit.c) `func_mic_emit_init()` L46-48、[func_adapter.c:58-69](../../functions/func_adapter.c) `func_adapter_init()` L66-68，进入角色即 `led_bt_idle()`。
- **运行期唯一随连接态切 LED 的点**：[wireless_proc.c:319-340](../../modules/wireless/wireless_proc.c) `wireless_mic_proc()` L322 比较 `wireless_mic.connected_sta != sys_cb.disp_sta`，L332-338 `#if BSP_LED_EN` 下 `disp_sta==0 → led_bt_idle()`、否则 `led_bt_connected()`。其上游是 [func_mic_emit.c:143-172](../../functions/func_mic_emit.c) `func_mic_emit_process()` L156 与 [func_adapter.c:166-230](../../functions/func_adapter.c) `func_adapter_process()` L189 调 `wireless_mic_proc()`。
- **充电红灯链**：[func.c:43-68](../../functions/func.c) `func_process()` L50-65 `#if BSP_CHARGE_EN` 下按 `charge_working_while_charging` 调 `bsp_charge_process()`；[bsp_charge.c:110-169](../../bsp/bsp_charge.c) `bsp_charge_process()` L120 `charge_get_status()`，L124-128 状态变化时 `>= CHARGE_STA_ON_CON_CURR` 调 `led_charge_on()`、否则 `led_charge_off()`——**红灯唯一驱动**。
- **关机**：[func_lowpwr.c:407-417](../../functions/func_lowpwr.c) `func_pwroff()` L414-416 直接 `bled_set_off()`（不走 `led_power_down` 死接口）。

#### C. led_scan 50ms 闪烁状态机时序

```mermaid
sequenceDiagram
    participant TMR as 5ms 硬件定时器
    participant CB as usr_tmr5ms_thread_callback<br/>strong_symbol.c:54
    participant DO as usr_tmr5ms_thread_do<br/>bsp_sys.c:61
    participant SCAN as led_scan<br/>bsp_led.c:214
    participant CB2 as led_cb_t
    participant GPIO as bled_set_on/off<br/>bsp_led.c:62/77

    Note over TMR: 每 5ms 触发一次
    TMR->>CB: 中断回调
    CB->>DO: usr_tmr5ms_thread_do()
    DO->>DO: tmr5ms_cnt++ (bsp_sys.c:63)
    alt tmr5ms_cnt % 10 == 0（每 50ms）
        DO->>SCAN: led_scan(tick_get()) (bsp_sys.c:75)
        SCAN->>CB2: if(!bled_disp_en) return (L218)
        SCAN->>CB2: if(unit==0||circle==0) return (L225)
        SCAN->>CB2: if(charge_flag) return (L229 充电早退)
        SCAN->>CB2: tick/50 算 offset (L241-242)
        alt offset < unit*8 且 offset%unit==0
            SCAN->>CB2: bcnt = offset/unit (L245)
            alt bled_sta & BIT(bcnt) == 1
                SCAN->>GPIO: bled_set_on() (L247)
            else
                SCAN->>GPIO: bled_set_off() (L249)
            end
        else offset >= unit*8
            SCAN->>CB2: busy=false, 一次性图样 unit=0 (L253-257)
        end
    end
```

#### D. 闪烁算法原理：位图消耗 + 圆环对齐

`led_cfg_t` 把一次图样压缩成 4 字节，`led_scan` 用「圆环 + 位图」还原成时序亮灭：

1. **圆环长度** `circle`（[bsp_led.c:205](../../bsp/bsp_led.c)）：`circle = unit*8 + period`。前 `unit*8` 段是「图样播放区」（8 个 `unit` 时隙，对应 `bluepat` 的 8 个 bit），后面 `period` 段是「灭灯间隔」。`cycle==0xff` 时 `circle=0x7fff` 表示永久循环（[bsp_led.c:201-203](../../bsp/bsp_led.c)）。
2. **相位对齐**（[bsp_led.c:208-209](../../bsp/bsp_led.c)）：`offset = (tick/50 + 1) % circle`，把全局 tick 对齐到圆环起点，保证多次 `led_set_sta` 切换图样时相位连续、不跳变。
3. **位图消耗**（[bsp_led.c:243-251](../../bsp/bsp_led.c)）：`bcnt = offset/unit`（0~7），`if (bled_sta & BIT(bcnt))` 决定亮灭——即 `bluepat` 的第 `bcnt` 位为 1 则该时隙亮。
4. **举例**：`led_cfg_pairing = {0x00, 0x0f, 2, 2}`（[bsp_led.c:14](../../bsp/bsp_led.c)）→ `bluepat=0x0f`=二进制`00001111`、`unit=2`(100ms)、`circle=2*8+2=18`(900ms)。前 8×100ms=800ms 中，`bcnt` 0~3 位为 1（亮 400ms），4~7 位为 0（灭 400ms），再灭 100ms，循环——即「蓝灯亮 400ms 灭 500ms」。

#### E. 解耦设计要点

1. **图样是数据，不是代码**：`led_cfg_t` 与 `led_cb_t` 分离，`led_set_sta_p` 只搬数据进状态机，`led_scan` 只按数据消耗。新增一种闪法只需新增一份 `led_cfg_t`，不动扫描器。
2. **`led_set_sta_p` 接 `led_cb_t*` 参数**（[bsp_led.c:189](../../bsp/bsp_led.c)）：`led_set_sta_normal` 再转一手传 `&led_cb`（[bsp_led.c:184](../../bsp/bsp_led.c)）。这一层间接允许将来存在多个 `led_cb_t` 实例（多组灯各扫各的），虽当前只用全局 `led_cb`。
3. **`led_set_sta` 宏间接**（[bsp_led.h:36](../../bsp/bsp_led.h)）：`#define led_set_sta(cfg) led_set_sta_normal(cfg)`，业务层写 `led_set_sta(…)` 而非直指函数名，便于日后切换实现。
4. **`charge_flag` 早退**（[bsp_led.c:229](../../bsp/bsp_led.c)）：`led_scan` 一进来 `if(led_cb.charge_flag) return`，把「充电恒亮红灯」从「闪烁扫描」中完全解耦——充电时闪烁扫描直接停摆，红灯由 `led_charge_on/off` 直接管恒亮/恒灭，两条驱动路径互不干扰。
5. **`led2_flag` 共脚双色**（[bsp_led.c:40-44](../../bsp/bsp_led.c)）：`bled_io_sel==rled_io_sel` 时单引脚靠 `led2_port_out_mode` 切模拟/数字模式推双色。当前板 PB3(蓝)≠PB4(红) 故 `led2_flag=0`，走独立脚路径。
6. **禁用安全保护**：`bled_set_on/off`、`led_scan` 头部都判 `xcfg_cb.bled_disp_en`（[bsp_led.c:64/79/218](../../bsp/bsp_led.c)），配置层关灯后③层不操作 GPIO，避免「配了灯但 IO 没接」时误动引脚。

#### F. 三个解耦破缺点（实测已确认）

破缺点即「五层隔离被绕过」的地方，前两项是历史遗留的死代码，第三项是引脚资源冲突。

**破缺 1 · `led_power_up/led_power_down` 是死接口**

[bsp_led.c:129/140](../../bsp/bsp_led.c) 两个函数在全仓无任何调用方——开机不调 `led_power_up`，关机走 [func_lowpwr.c:414-416](../../functions/func_lowpwr.c) `func_pwroff()` 直接 `bled_set_off()`。即 `xcfg_cb.led_pwron_config_en`/`led_pwroff_config_en` 及对应图样 `led_poweron/led_poweroff`（[xcfg.h:54-67](../../projects/microphone/xcfg.h)）**实际不生效**。ToolKit 界面里「开机/关机闪灯」配置项是空操作。若要恢复，需在 `bsp_sys_init` 或 `func_pwroff` 中补调用点。

**破缺 2 · `rled_sta` 是死字段 → 红灯无闪烁机制**

- `led_set_sta_p`（[bsp_led.c:197](../../bsp/bsp_led.c)）只写 `s->bled_sta = sta.bluepat`，**从不写 `s->rled_sta`**；`rled_sta`（[bsp_led.h:20](../../bsp/bsp_led.h)）在全仓无任何写入点。
- `led_scan`（[bsp_led.c:246](../../bsp/bsp_led.c)）只读 `s->bled_sta & BIT(bcnt)` 调 `bled_set_on/off`，**从不读 `rled_sta`、从不调 `rled_set_on/rled_set_off`**。
- 后果：所有 `led_bt_idle`/`led_bt_connected`/`led_power_up`/`led_power_down` 设的图样（哪怕 `redpat` 非 0）只影响蓝灯，**红灯纹丝不动**。红灯唯一驱动是充电链的 `led_charge_on/off`（恒亮/恒灭），**无闪烁**。

> 若后续需红灯闪烁（充电异常、低电告警），须新增驱动：在 `led_set_sta_p` 写 `s->rled_sta`，并在 `led_scan` 的 `charge_flag` 早退分支外另开红灯位图消耗分支（或单独加红灯扫描入口）。属新功能，需单独评审，见 [../plan/AB5766_LE_Mic_分阶段实施计划.md](../plan/AB5766_LE_Mic_分阶段实施计划.md) 阶段 2 B 节、6.4 风险表「红灯无闪烁机制」。

**破缺 3 · `led_init` 的 `gpio_init` 覆盖 UART TX 交叉开关 → PB3 蓝灯卡死**

这是唯一**跨层破坏隔离**的破缺：③层的 `led_cfg_init` 经④层 `gpio_init` 把引脚从②层 `BSP_UART_DEBUG_EN` 建立的「UART 外设功能」改写成「普通 GPIO」，导致 UART0 TX 物理通路失效。

```mermaid
flowchart TD
    P1["编译期 config_ab5766_le_mic.h:24<br/>BSP_UART_DEBUG_EN = GPIO_PB3"]
    P2["启动早期 strong_symbol.c:66<br/>uart_debug_init_callback()"]
    P3["bsp_uart_debug.c:8 bsp_uart_debug_init()<br/>L27-28 交叉开关映射 UART0RX/TX → PB3<br/>gpio_func_mapping_config(..., GPIO_CROSSBAR_OUT_UART0TX)"]
    P4["bsp_sys.c:202 bsp_sys_init()<br/>xcfg_init() 加载 bled_io_sel=179(PB3)"]
    P5["bsp_sys.c:213 pmu_init()<br/>打印 lock wireless_mic comm … vddbt_capless_en:1"]
    P6["bsp_sys.c:224 led_init()<br/>bsp_led.c:47 led_cfg_init(BSP_BLED_PIN,...)"]
    P7["bsp_led.c:19 led_cfg_init<br/>gpio_init 设 FEN_GPIO + FDIR_SELF + DIGITAL"]
    P8["driver_gpio.c:15 gpio_init<br/>逐位重写 fen → 清掉 UART TX 交叉开关映射"]
    P9["driver_uart.c:150 uart_baud_config<br/>while ((uartx->con & UARTxCON_TXPND) == 0)"]
    P10["bsp_sys.c:264 printf('bsp_sys_init')"]
    DEAD["putchar 忙等待 TXPND 不翻转<br/>系统卡死在 vddbt_capless_en:1 之后"]

    P1 --> P2 --> P3
    P4 --> P5 --> P6 --> P7 --> P8
    P8 -->|"TX 物理引脚已非 UART 外设引脚"| P9
    P10 --> P9 --> DEAD

    classDef sw fill:#fff3cd,stroke:#ffc107
    classDef uart fill:#cfe8ff,stroke:#0d6efd
    classDef led fill:#d1e7dd,stroke:#198754
    classDef hal fill:#f8d7da,stroke:#dc3545
    classDef dead fill:#343a40,stroke:#000,color:#fff
    class P1 sw
    class P2,P3,P9 uart
    class P4,P5,P6,P7 led
    class P8 hal
    class P10,DEAD dead
```

**为何日志正好停在 `vddbt_capless_en:1`**：`bsp_sys_init()` 顺序是 `pmu_init()`（打印 `lock wireless_mic comm` … `vddbt_capless_en:1`，全部在 `led_init()` 之前完成）→ `led_init()`（L224，破坏 PB3 的 UART TX 映射，本身无打印）→ … → `printf("%s\n",__func__)`（L264，打印 `bsp_sys_init`）。PMU 末行打印完后紧接着 `led_init()` 破坏引脚，下一处 printf（L264）的 putchar 经 UART0 发送时 `TXPND` 无法翻转（[driver_uart.c:150](../../driver/driver_uart.c) 忙等），死锁，故 `bsp_sys_init` 这行不再出现，与实测一致。

**解决路径**（二选一，均不改③④层代码）：

1. **蓝灯不配 PB3**，改用其他空闲引脚——实测改到 PA0（`bled_io_sel=160`）即恢复，仅改运行时配置无需重编译。
2. **若必须用 PB3 作蓝灯**，先把编译期 `BSP_UART_DEBUG_EN` 从 `GPIO_PB3` 改到 PA0/PA1/PA4 等并重编译，让出 PB3。

> 该破缺的根因是**②层与③层对同一物理引脚没有仲裁**——`BSP_UART_DEBUG_EN`（编译期）和 `bled_io_sel`（运行时）分属两个配置空间，互不感知。彻底修法是在 `led_init` 调 `led_cfg_init` 前加一道「目标 IO 与 `BSP_UART_DEBUG_EN` 相同则拒配」的断言，把隐式冲突显式化。当前以「文档约束 + ToolKit 不选 PB3」规避。

#### G. 一句话总结

LED 子系统把「行为（图样）」做成 `xcfg_cb` 里的纯数据、把「时序（扫描）」交给 50ms 定时器、把「电平」下沉到 GPIO Driver，三层各司其职——这是它**解耦得最好**的部分；而 `led_power_*` 死接口、`rled_sta` 死字段、PB3 引脚冲突，则是它**解耦被破坏**的三处明证，修改 LED 行为前务必先对照本附录确认未踩这三点。

### 第 8 章 · Module 层：算法的"小工厂"

**Module 层** 是 SDK 的"业务工具箱"。每个 Module 通常是一个"独立问题域"：

| 子目录 | 业务 | 典型 API |
|---|---|---|
| `modules/wireless/` | 无线麦协议（D2A = Device To Adapter） | `bsp_ble_init()`、`mic_emit_init()`、`adapter_init()` |
| `modules/audio/` | 音频处理（DAC、EQ、增益） | `dac0_out_init()`、`soft_gain_init()` |
| `modules/voice/` | 7 种语音算法 | `echo_audio_init()`、`magic_audio_init()` |
| `modules/codec/` | 编解码 | `lc3s_enc()` / `lc3s_dec()` |
| `modules/effect/` | 音效表管理 | `mic_effect_load()` |
| `modules/device/` | 设备去抖 | `dev_init()`、`dev_online_filter()` |
| `modules/tool/` | 工具集（调音、调试） | `toolkit_process()` |

**Module 层**采用统一的 **"init/proc/reset" 三段式 API**：

```mermaid
flowchart LR
    A[mic_emit_init<br/>一次性配置] --> B[连接成功后触发]
    B --> C[mic_emit_process<br/>每音频帧调用]
    C --> D[持续运行]
    D --> E[断开时 mic_emit_reset 清理]
    classDef init fill:#cfe2ff,stroke:#0d6efd
    classDef proc fill:#fff3cd,stroke:#ffc107
    classDef reset fill:#f8d7da,stroke:#dc3545
    class A init
    class B,C,D proc
    class E reset
```

**这套设计的妙处**：Module 内部所有算法都被统一管理。打开 `modules/wireless/mic_proc.c:597-675`，你看到的是这样：

```c
void mic_emit_init(void) {
    memset(&mic_enc, 0x00, sizeof(mic_enc));           // 602
    mic_enc.first_pkt = true;                          // 603
    param_mic_mute_level_read(&mic_enc.mute_en);       // 605

#if WIRELESS_MIC_32K_EN
    src_init(0, 32000, 48000);                        // 629  32k → 48k 重采样
#endif
#if WIRELESS_MIC_ECHO_EN
    echo_audio_init(0, samples, 1);                   // 637
#endif
#if WIRELESS_MIC_MAGIC_EN
    magic_audio_init(0, samples);                     // 641
#endif
// ... 其它算法 init
    bsp_sdadc_init();                                  // 669
}
```

**每个算法都用 `#if XXX_EN` 宏守护**——关闭特性就不参与编译，节省 Flash 空间。

### 第 9 章 · Application 层：状态机 + super-loop + 消息机制

**Application 层** 是 SDK 的"大脑"。它做四件事：

1. **决定当前工作模式**（发射端 / 接收端 / 低电）
2. **轮询各个模块**（状态机主循环）
3. **接收消息**（按键、低电等事件）
4. **分发消息**（执行具体动作）

**核心：`functions/func.c:134 func_run()` 是永不返回的 super-loop。**

```c
// functions/func.c:134-178  顶层状态机
void func_run(void) {
    if (wireless_role_is_adapter()) {       // 行 138
        func_cb.sta = FUNC_ADAPTER;          // 接收端
    } else {
        func_cb.sta = FUNC_MIC_EMIT;         // 发射端
    }

    while (1) {                              // 死循环
        func_enter();                        // 进入新功能前初始化
        switch (func_cb.sta) {
            case FUNC_MIC_EMIT:  func_mic_emit(); break;   // 行 153
            case FUNC_ADAPTER:   func_adapter();  break;   // 行 158
            case FUNC_PWROFF:    func_pwroff();   break;   // 行 170
            // ... 其他模式
        }
    }
}
```

**进入模式前 = `func_enter()`**：

```c
// functions/func.c:115-125
void func_enter(void) {
    bsp_param_sync();                                   // 同步参数到 Flash
    lowpwr_sleep_delay_reset();                         // 重置睡眠延时
    lowpwr_pwroff_delay_reset();                        // 重置关机延时
    key_set_msg_tbl(func_info[func_cb.sta].msg_tbl);    // 装入该模式的按键表
}
```

注意 `key_set_msg_tbl(...)` —— **不同模式用不同的按键表**！发射端有 PP/K1/K2/K3 四键，接收端只需要 PP 键。

**模式内主循环（以发射端为例）：**

```c
// functions/func_mic_emit.c:212-227
void func_mic_emit(void) {
    func_mic_emit_enter();                  // 初始化（清队列、bsp_ble_init）
    while (func_cb.sta == FUNC_MIC_EMIT) {
        func_mic_emit_process();            // 状态机推进 + 协议栈 tick
        u16 msg = msg_dequeue();           // 取出消息
        if (msg != MSG_NO) {
            func_mic_emit_message(msg);     // 分派消息
        }
    }
    func_mic_emit_exit();                   // 退出清理
}
```

**状态机切换示例**：

```mermaid
stateDiagram-v2
    [*] --> FUNC_MIC_EMIT
    FUNC_MIC_EMIT --> FUNC_PWROFF: MSG_PWR_HOLD 超时
    FUNC_MIC_EMIT --> FUNC_PWROFF: EVT_LPWR_OFF
    FUNC_PWROFF --> [*]: 关机完成
    FUNC_MIC_EMIT --> FUNC_LE_DUT: FUNC_LE_DUT_EN 模式
    note right of FUNC_MIC_EMIT
      发射端/接收端都通过
      func_cb.sta 切换
    end note
```

**func_mic_emit_message 消息分派**（`functions/msg_mic_emit.c:5-135`）：

- `MSG_VOL_UP` → `soft_gain_up()` + 同步给接收端
- `MSG_VOL_DOWN` → `soft_gain_down()` + 同步给接收端
- `MSG_PWR_HOLD` → 开始关机计时
- `MSG_ECHO_LEVEL_UP` → `echo_delay_level_up()` + 同步给接收端
- ...

`func_mic_emit_message` 是一个 `switch(msg)` 结构，每个 case 分派一个动作。这种设计叫 **"消息驱动"**。

### 第 10 章 · OSAL 层：两个"伪线程" + kick 宏投递

**OSAL（Operating System Abstraction Layer）** 是嵌入式 SDK 的"伪操作系统"。AB5766 是个 RISC-V 芯片，**没有 RTOS**，但通过 `os/` 目录模拟出"线程"概念。

**伪线程三件套**：

```c
// os/os_thread.h:5-19  消息枚举 + kick 宏
enum {
    MSG_MIC_DEC0 = 0,         // 接收端解码线程消息（第 0 通道）
    MSG_MIC_DEC1,             // 接收端解码线程消息（第 1 通道）
    MSG_MIC_AINS4_32K,        // 算法线程消息
};

void mic_dec_kick_cb(u8 msg);              // 接收端解码 kick 回调
void mic_alg_kick_cb(u8 msg);              // 算法 kick 回调

#define kick_dec_prio_trans(idx)           mic_dec_kick_cb(MSG_MIC_DEC0 + (idx))
#define robotization_mic_proc_kick_start()  mic_alg_kick_cb(MSG_MIC_ROBOTIZATION)
#define ains4_32k_mic_proc_kick_start()     mic_alg_kick_cb(MSG_MIC_AINS4_32K)
```

**两个"线程"是什么？**

```c
// os/thread_dec.c:4-20  接收端解码线程
AT(.com_text.adapter.proc.thread)
void thread_dec_proc_msg_cb(u32 msg) {
    switch (msg) {
        case MSG_MIC_DEC0:
            mic_dec_prco_cb(0);    // 通道 0 解码
            break;
        case MSG_MIC_DEC1:
            mic_dec_prco_cb(1);    // 通道 1 解码
            break;
    }
}

// os/thread_alg.c:4-31  算法线程
AT(.com_text.thread)
void thread_alg_proc_msg_cb(u32 msg) {
    switch (msg) {
        case MSG_MIC_AINS4_32K:
            ains4_32k_mic_proc_cb();
            break;
#if ROBOTIZATION_EN
        case MSG_MIC_ROBOTIZATION:
            robotization_mic_proc_cb();
            break;
#endif
    }
}
```

**为什么需要"伪线程"？**

假设 BLE 中断里调用 `mic_dec_prco_cb()`（这个函数要 LC3S 解码 + PCM 处理 + DAC 输出，耗时 1-2ms）。**中断里跑这么久会阻塞 BLE 调度，让连接断开**。

`os/` 的伪线程机制：

```mermaid
sequenceDiagram
    participant ISR as BLE ISR
    participant KICK as kick_dec_prio_trans
    participant LIB as 调度库
    participant THREAD as thread_dec_proc_msg_cb
    participant DEC as mic_dec_prco_cb

    ISR->>KICK: mic_dec_kick_cb(MSG_MIC_DEC0)
    Note over ISR: ISR 立即返回
    KICK->>LIB: 投递到线程
    Note over LIB: 等 BLE 空闲时调度
    LIB->>THREAD: 调用 thread_dec_proc_msg_cb
    THREAD->>DEC: mic_dec_prco_cb(0)
    Note over DEC: 耗时 1-2ms 的解码
    DEC-->>THREAD: 完成
    THREAD-->>LIB: 返回
```

**核心思想**：**ISR 只做"清 pending + kick"，重工作交给线程**。

> **小白理解**：把这想象成"医院分诊"。ISR 是急诊分诊台（立即响应），线程是门诊医生（真正看病）。这样急诊不会卡死。

---

## 第三篇 抽象与接口：代码里的"合同"

### 第 11 章 · 5 种统一"合同"设计模式

AB5766 SDK 里反复使用 **5 种设计模式**。掌握它们，你就掌握了 SDK 的"语法"。

```mermaid
flowchart LR
    P1["1. 统一回调签名<br/>audio_callback_t"] --> EX1["定义在 libs/cpu/api_msg.h"]
    P2["2. init/proc/reset 三段式"] --> EX2["mic_emit_init/proc/reset"]
    P3["3. STRONG/WEAK 符号覆盖"] --> EX3["strong_symbol.c 覆盖库 WEAK"]
    P4["4. 全局单例 cb 结构体"] --> EX4["sys_cb / xcfg_cb / func_cb"]
    P5["5. AT 段属性宏"] --> EX5["AT 段属性宏 控制代码位置"]
    classDef pat fill:#cfe2ff,stroke:#0d6efd
    classDef ex fill:#fff3cd,stroke:#ffc107
    class P1,P2,P3,P4,P5 pat
    class EX1,EX2,EX3,EX4,EX5 ex
```

**模式 1：统一回调签名 `audio_callback_t`**

```c
// functions/func_mic_emit.h:11
typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *params);
```

所有音频算法都按这个签名注册回调，避免"接口不一"问题。

**模式 2：init/proc/reset 三段式**

| 阶段 | 调用时机 | 作用 |
|---|---|---|
| `init()` | BLE 连接成功后 | 初始化算法、分配缓冲 |
| `proc()` | 每帧音频 | 算法处理 1 帧 PCM |
| `reset()` | BLE 断开时 | 释放缓冲、清状态 |

**模式 3：STRONG/WEAK**

```c
// projects/microphone/strong_symbol.c:54-58
STRONG void usr_tmr5ms_thread_callback(void) {
    usr_tmr5ms_thread_do();        // 库里的 WEAK 默认空实现被覆盖
}
```

库函数声明为 `WEAK`（弱符号），项目用 `STRONG` 重写即可"无缝替换"。

**模式 4：全局单例**

| 单例 | 位置 | 用途 |
|---|---|---|
| `sys_cb` | `bsp/bsp_sys.c:8` | 系统状态（pwroff、mic_alg_en、disp_sta） |
| `xcfg_cb` | `bsp/bsp_sys.c:7` | 产测配置（70+ 字段） |
| `func_cb` | `functions/func.c:4` | 当前/上次工作模式 |
| `wireless_mic` | `modules/wireless/wireless.c:37` | 蓝牙连接状态 |
| `mic_enc` / `mic_dec` | `modules/wireless/mic_proc.c:46-50` | 编码/解码上下文 |

**模式 5：AT() 段属性宏**

```c
// header/macro.h:9
#define AT(x)  __attribute__((section(STR(x))))

// 使用示例
AT(.com_text.sdadc.isr) static void sdadc_isr_cb(void) { ... }
AT(.buf.sdadc) static int16_t sdadc_buf[SDADC_DMA_SIZE];
```

`AT(.xxx)` 把代码/数据放到指定内存段。比如 `AT(.com_text.isr)` 表示"放在 comm 段的 ISR 段"。

### 第 12 章 · 单例与全局变量：SDK 的"共享白板"

**嵌入式为什么大量用全局结构体？**

| 优点 | 说明 |
|---|---|
| 节省栈 | 嵌入式栈通常只有 1-4KB |
| 跨层共享 | 不用层层传指针 |
| 零开销 | 无 malloc/free |
| 易于静态分析 | 编译期确定地址 |

**5 个核心单例详解**

| 单例 | 字段数 | 主要字段 | 生命周期 |
|---|---|---|---|
| `sys_cb` | 多个 | `pwroff`、`mic_alg_en`、`disp_sta` | 整个程序 |
| `xcfg_cb` | 80+ | `wireless_adapter_en`、`wireless_mic_emit_en`、各种 GPIO/电压 | 产测后不变 |
| `func_cb` | 2 | `sta`、`last` | 整个程序 |
| `wireless_mic` | 4-7 | `change_flag`、`connected_sta`、`change_sta[2]` | BLE 生命周期 |
| `mic_enc` / `mic_dec` | 5-10 | `mute_en`、`buffer[]`、`pcm[2]` | BLE 生命周期 |

**`xcfg_cb` 是"产测烧入"的——Flash 中的 71 字段**，由配置工具写入；运行时由 `bsp_sys_init()` 加载到 RAM，覆盖编译期宏。

### 第 13 章 · STRONG/WEAK 符号覆盖：项目定制 SDK 的秘诀

**为什么需要 STRONG/WEAK？**

SDK 库提供"通用默认实现"，但每个产品都要做"客制化"。STRONG/WEAK 提供"项目可覆盖库默认"的能力。

```mermaid
flowchart LR
    A[libs/libplatform.a<br/>库默认实现 WEAK] -->|链接时被覆盖| B[app.rv32]
    B --> C[STRONG usr_tmr5ms_thread_callback<br/>projects/microphone/strong_symbol.c:54]
    B --> D[STRONG os_spiflash_addr_check<br/>strong_symbol.c:33]
    B --> E[STRONG pmu_cfg_vddio/vddbt/vddcore<br/>strong_symbol.c:83-95]
    A2[libs/ble/libbtstack.a<br/>BLE 默认 WEAK] -->|链接时被覆盖| B
    B --> F[STRONG ble_t2r_v8_*<br/>projects/microphone/strong_ble.c:128]
    classDef lib fill:#e2e3e5,stroke:#6c757d
    classDef app fill:#cfe2ff,stroke:#0d6efd
    class A,A2 lib
    class B,C,D,E,F app
```

**经典例子 1：5ms 心跳钩子**

```c
// 库端（libs/libplatform.a）声明为 WEAK
WEAK void usr_tmr5ms_thread_callback(void) { /* 默认空 */ }

// 项目端覆盖（strong_symbol.c:54-58）
STRONG void usr_tmr5ms_thread_callback(void) {
    usr_tmr5ms_thread_do();        // 调 BSP 的 5ms tick 钩子
}
```

**经典例子 2：SPI Flash 保护**

```c
// strong_symbol.c:33-51  限制 SPI Flash 写入范围
STRONG bool os_spiflash_addr_check(u32 addr, uint len) {
    if ((addr >= CM_START) && ((addr + len) <= (CM_START + CM_SIZE))) {
        return true;                    // CM 区允许
    }
#if AB_FOT_EN
    if ((addr >= FOT_PACK_START) && ((addr + len) <= (FOT_PACK_START + FOT_PACK_SIZE))) {
        return true;                    // FOT 区允许
    }
#endif
    printf("-->os_spiflash_addr_err:0x%x\n", addr);
    return false;                       // 其它区域禁止
}
```

**经典例子 3：电压配置回调**

```c
// strong_symbol.c:83-95
uint8_t pmu_cfg_vddio(void) { return xcfg_cb.vddio_sel; }
uint8_t pmu_cfg_vddbt(void) { return xcfg_cb.vddbt_sel; }
uint8_t pmu_cfg_vddcore(void) { return xcfg_cb.vddcore_sel; }
```

库要配置 PMU 电压，但库又不知道项目选什么电压——所以通过回调让项目自己决定。

**STRONG/WEAK 使用规则**：

| 规则 | 例子 |
|---|---|
| 库里**默认**实现用 `WEAK` | `WEAK void xxx(void) { }` |
| 项目**要覆盖**时用 `STRONG` | `STRONG void xxx(void) { ... }` |
| 一份项目只能覆盖一次 | 重复覆盖会导致链接错误 |
| 跨模块覆盖很常见 | `strong_symbol.c`（平台）+ `strong_ble.c`（BLE） |

---

## 第四篇 数据流与控制流：让信号在 SDK 里跑一遍

### 第 14 章 · 音频数据流：麦克风 PCM 到 RF 发送的完整路径

跟踪一帧 PCM 数据的生命路径，理解 SDK 是怎么"端到端"协作的。

```mermaid
sequenceDiagram
    participant MIC as MEMS 麦克风
    participant DMA as SDADC DMA
    participant BUF as sdadc_buf
    participant ISR as sdadc_isr_cb
    participant THREAD as sdadc_proc_cb
    participant ENC as mic_enc_proc_cb
    participant ALG as 算法链
    participant CODEC as lc3s_enc
    participant TX as TX 队列
    participant BLE as BLE 库
    participant RF as RF PHY

    MIC->>DMA: 模拟信号 → 数字采样
    DMA->>BUF: 写入半/全帧<br/>bsp_sdadc.c:14
    BUF->>ISR: DMA 半满/全满触发
    activate ISR
    ISR->>ISR: 清 pending + 保存 tick
    ISR->>THREAD: kick 投递 (sdadc_done_proc_kick)
    deactivate ISR
    activate THREAD
    THREAD->>ENC: mic_enc_proc_cb(pcm, samples)<br/>bsp_sdadc.c:45
    activate ENC
    ENC->>ALG: 算法链处理<br/>mut/eq/drc/echo/magic
    ALG->>CODEC: lc3s_enc(pcm, mic_enc.buffer, samples)<br/>mic_proc.c:359
    CODEC->>TX: wireless_d2a_put_tx_frame<br/>wireless_txrx_single.c:308
    deactivate ENC
    TX->>BLE: set_txbuf_cb(ptr, len)<br/>wireless_txrx_single.c:301
    BLE->>RF: 物理发送
    deactivate THREAD
```

**10 步详解**

| 步 | 文件:行 | 动作 |
|---|---|---|
| 1 | `bsp_sdadc.c:14` | `sdadc_buf[SDADC_DMA_SIZE] AT(.buf.sdadc)` DMA 缓冲 |
| 2 | `driver_sdadc.c:194` | `sdadc0_cmd` 打开 ADC0 |
| 3 | `bsp_sdadc.c:100` | `sdadc_pic_config` 注册 ISR |
| 4 | `bsp_sdadc.c:27-37` | `sdadc_isr_cb` ISR：清 pending + `sdadc_done_proc_kick` 投递 |
| 5 | `bsp_sdadc.c:39-53` | `sdadc_proc_cb` 算法线程回调：从 `sdadc_buf` 取 PCM |
| 6 | `bsp_sdadc.c:45,51` | 调 `mic_enc_proc_cb(rbuf, samples)` |
| 7 | `mic_proc.c:277-378` | `mic_enc_proc_cb`：mute + 算法链 + LC3S 编码 |
| 8 | `mic_proc.c:359` | `lc3s_enc` 编码 |
| 9 | `mic_proc.c:363` | `wireless_d2a_put_tx_frame` 入 TX 队列 |
| 10 | `wireless_txrx_single.c:301` | `wireless_d2a_set_txbuf_cb` 库取走 buffer 发送 |

**关键观察**：

- **DMA 缓冲**绕过 CPU，由硬件自动搬运
- **算法处理**在"线程"里跑（不在 ISR 中），避免阻塞 BLE
- **LC3S 编码**完成后立刻入 TX 队列，再由 BLE 库取出发送

### 第 15 章 · 控制流：按下 VOL+ 后的完整链路

跟踪一次按键事件的生命路径。

```mermaid
sequenceDiagram
    participant T as 5ms 定时器
    participant BS as bsp_sys.c
    participant KEY as bsp_key_scan
    participant KSP as key_status_process
    participant KGET as bsp_key_get_msg
    participant Q as msg_queue
    participant FM as func_mic_emit loop
    participant FMM as func_mic_emit_message
    participant SG as soft_gain_up
    participant BLE as BLE 通道

    T->>BS: 5ms 心跳
    activate BS
    BS->>KEY: bsp_key_scan(0)
    deactivate BS
    activate KEY
    KEY->>KSP: 抖动 + 状态机
    KSP->>KGET: 查 mic_emit_key_msg_tbl
    KGET->>Q: msg_enqueue(MSG_VOL_UP)
    deactivate KEY
    activate FM
    FM->>Q: msg_dequeue()
    FM->>FMM: func_mic_emit_message(msg)
    deactivate FM
    activate FMM
    FMM->>SG: soft_gain_up()
    SG->>BLE: wireless_tx_soft_gain_level
    deactivate FMM
```

**关键文件:行号**

| 步 | 文件:行 | 动作 |
|---|---|---|
| 1 | `bsp_sys.c:60-97` | `usr_tmr5ms_thread_do` 5ms tick |
| 2 | `bsp_sys.c:79-81` | `bsp_key_scan(0)` |
| 3 | `bsp_key.c:126` | `key_status_process` 区分短/长/双击 |
| 4 | `bsp_key.c:130` | `bsp_key_get_msg` 查 `mic_emit_key_msg_tbl` |
| 5 | `bsp_key.c:131` | `msg_enqueue(MSG_VOL_UP)` |
| 6 | `func_mic_emit.c:219` | `msg_dequeue()` |
| 7 | `func_mic_emit.c:221` | `func_mic_emit_message(msg)` |
| 8 | `msg_mic_emit.c:59` | `soft_gain_up()` |
| 9 | `msg_mic_emit.c:65` | `wireless_tx_soft_gain_level` 同步 |

**关键观察**：

- **按键事件**：5ms 扫描 → 消抖 → 查表 → 入队
- **状态机**：从队列取消息 → 分派 → 执行
- **跨设备同步**：动作执行后通过 BLE 私有命令同步给接收端

### 第 16 章 · 线程间通信：kick 宏与消息队列

**为什么需要线程间通信？**

BLE 中断里不能跑 LC3S 解码（耗时 1-2ms）；但 LC3S 解码又需要**尽快**跑（避免音乐断流）。所以需要"中断 kick 出来，到线程里跑"。

```mermaid
flowchart LR
    A[BLE ISR<br/>不能阻塞] -->|kick_dec_prio_trans idx| B[os_thread.h 宏]
    B -->|调用 mic_dec_kick_cb| C[调度库]
    C -->|空闲时调用| D[thread_dec_proc_msg_cb]
    D -->|switch msg| E[mic_dec_prco_cb 0]
    D -->|switch msg| F[mic_dec_prco_cb 1]
    E -->|共享 buffer| G[mic_dec.pcm 0]
    F -->|共享 buffer| G[mic_dec.pcm 1]
    classDef isr fill:#f8d7da,stroke:#dc3545
    classDef th fill:#cfe2ff,stroke:#0d6efd
    class A,C isr
    class B,D,E,F,G th
```

**共享 buffer 模式**

`bsp_sdadc.c:14 sdadc_buf[SDADC_DMA_SIZE]` 是全局共享：

```c
// bsp_sdadc.c:14
int16_t sdadc_buf[SDADC_DMA_SIZE] AT(.buf.sdadc);
```

ISR 写入 → 线程读取 → 处理。这就是"跨线程共享数据"。

**线程间通信的两条规则**：

1. **ISR 永远只 kick**（投递消息），不直接处理
2. **线程共享 buffer**，但要注意**临界区**（用 `GLOBAL_INT_DISABLE` 保护）

```c
// 关键代码：BLE TX 帧缓冲（wireless_txrx_single.c:197-228）
void txpkt_put_frame(txpkt_ctl_t *txpkt, u8 *ptr, u8 size) {
    GLOBAL_INT_DISABLE();   // 关中断
    // ... memcpy 拷贝数据
    GLOBAL_INT_RESTORE();   // 开中断
}
```

---

## 第五篇 配置与构建：把它点亮

### 第 17 章 · 三层配置体系：config.h → config_xxx.h → xcfg

AB5766 有**三层配置**，承担不同角色：

```mermaid
flowchart LR
    A[config.h<br/>选产品方案] --> B[config_ab5766_le_mic.h<br/>当前产品特性]
    B --> C[config_define.h<br/>SDK 通用定义]
    A --> D[config_extra.h<br/>派生/校验]
    B --> D
    C --> D
    E[Output/bin/xcfg.xm<br/>产测 bin] -->|riscv32-elf-xmaker 转换| F[xcfg_cb<br/>运行时配置]
    F --> G[bsp_sys_init 加载]
    classDef src fill:#cfe2ff,stroke:#0d6efd
    classDef mid fill:#fff3cd,stroke:#ffc107
    classDef run fill:#d1e7dd,stroke:#198754
    class A,B,C,D src
    class E,F mid
    class G run
```

**详细说明**

| 文件 | 编译期/运行期 | 主要内容 |
|---|---|---|
| `config.h` | 编译期 | 选择 `USER_CONFIG = CONFIG_AB5766_LE_MIC` |
| `config_ab5766_le_mic.h` | 编译期 | 特性开关（`WIRELESS_MIC_ECHO_EN=0`）、时钟、GPIO |
| `config_define.h` | 编译期 | SDK 通用定义（Flash 容量、采样率、编解码索引） |
| `config_extra.h` | 编译期 | 派生校验（`FRAME_SIZE_MIN`、`WIRELESS_CON_INTERVAL`） |
| `xcfg.h` | 编译期结构体 + 运行时数据 | 71 个产测字段（`wireless_adapter_en`、`le_name`） |
| `xcfg.xm` | 产测期 | 配置工具生成的 bin，写入 Flash |

**关键了解**：

- **编译期宏**控制代码是否参与编译（如 `WIRELESS_MIC_ECHO_EN` 决定 `echo_audio_init` 是否被调用）
- **xcfg_cb** 是产测烧入的**运行时**配置，可以覆盖编译期默认值

**示例：开关一个特性**

假设你想打开 ECHO 音效：

```c
// projects/microphone/config_ab5766_le_mic.h:101
#define WIRELESS_MIC_ECHO_EN  1    // 0 → 1
```

**重新编译就行**——所有 `#if WIRELESS_MIC_ECHO_EN` 守护的代码会自动参与编译。

### 第 18 章 · xcfg 与产测流程：Flash 里的"调音台"

**xcfg_cb 是怎么"装进"固件的？**

```mermaid
flowchart LR
    A[配置工具<br/>GUI 输入] -->|生成 xcfg.xm| B[Output/bin/xcfg.xm]
    B -->|prebuild.bat 调用<br/>riscv32-elf-xmaker -b| C[Output/bin/xcfg.bin + xcfg.h]
    C -->|编译时 include| D[app.rv32]
    D -->|烧录到 Flash| E[Flash 0x1E000-0x20000 区域]
    E -->|运行时 bsp_sys_init 加载| F[xcfg_cb RAM 副本]
    F -->|后续 wifi_xxx_init 用| G[wireless_pwr_ctr / led / etc]
    classDef tool fill:#cfe2ff,stroke:#0d6efd
    classDef bin fill:#fff3cd,stroke:#ffc107
    classDef flash fill:#d1e7dd,stroke:#198754
    class A tool
    class B,C,D bin
    class E,F,G flash
```

**关键代码**

```c
// bsp/bsp_sys.c:202  加载 xcfg_cb
if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {
    printf("xcfg init error\n");
}

// projects/microphone/xcfg.h:7-40  xcfg_cb_t 字段示例
typedef struct {
    uint8_t     wireless_adapter_en;   // 决定角色
    uint8_t     wireless_mic_emit_en;
    char        le_name[32];           // 蓝牙广播名
    uint8_t     le_addr[6];            // 蓝牙地址
    uint8_t     rf_tx_pwr;             // 发射功率
    uint8_t     dac_sel;               // 单端/差分
    uint16_t    pwron_press_time;      // 长按开机时间
    uint16_t    pwroff_press_time;     // 长按关机时间
    // ... 60+ 字段
} xcfg_cb_t;
```

**关键了解**：

- `wireless_adapter_en=1` → 烧成接收端
- `wireless_mic_emit_en=1` → 烧成发射端
- 同一份固件烧到不同板子靠 xcfg 切换角色

### 第 19 章 · 构建系统：app.cbp + ram.ld + AT() 段名机制

**构建流程**

```mermaid
flowchart TD
    A[app.cbp + 源码] --> B[prebuild.bat]
    B --> B1[riscv32-elf-xmaker -b res.xm<br/>生成 res.h]
    B --> B2[riscv32-elf-xmaker -b xcfg.xm<br/>生成 xcfg.h]
    B --> B3[复制 effect.c/h 到工程]
    A --> C[riscv32-v3 编译]
    C --> D[链接 ram.ld + libs/*.a]
    D --> E[app.rv32]
    E --> F[postbuild.bat]
    F --> F1[riscv32-elf-objcopy -O binary<br/>生成 app.bin]
    F --> F2[riscv32-elf-xmaker -b<br/>生成 app.dcf 下载镜像]
    F --> F3[如果存在 C:\upload\upload.bat<br/>调用烧录]
    classDef src fill:#cfe2ff,stroke:#0d6efd
    classDef bin fill:#fff3cd,stroke:#ffc107
    classDef out fill:#d1e7dd,stroke:#198754
    class A,B,B1,B2,B3,C src
    class D,E bin
    class F,F1,F2,F3 out
```

**关键文件**

| 文件 | 作用 |
|---|---|
| `projects/microphone/app.cbp` | Code::Blocks 工程 |
| `projects/microphone/ram.ld` | 链接脚本（内存分段） |
| `Output/bin/prebuild.bat` | 编译前预处理 |
| `Output/bin/postbuild.bat` | 编译后处理（生成 .bin / .dcf） |

**`AT()` 段名机制**

`header/macro.h:9` 定义了 `AT(x)` 宏：

```c
#define AT(x)  __attribute__((section(STR(x))))
```

**作用**：把函数/变量放到指定内存段。

| 段名 | 物理位置 | 用途 |
|---|---|---|
| `AT(.text.mic_emit.proc.sdadc)` | `comm_emit` 区域 | 仅发射端跑的代码 |
| `AT(.text.adapter.proc.sdadc)` | `comm_adapter` 区域 | 仅接收端跑的代码 |
| `AT(.com_text.sdadc.isr)` | `comm` 区域 | ISR 代码 |
| `AT(.buf.sdadc)` | `.data` 区域 | SDADC 音频缓冲 |

**关键了解**：`__comm_emit_vma` / `__comm_adapter_vma` 共享同一 RAM 区域（运行时只装入当前角色那份）。

### 第 20 章 · 角色双烧：一块固件同时支持发射端与接收端

**这是 AB5766 SDK 最巧妙的设计**——同一份固件烧录到不同板子，自动跑发射端或接收端。

```mermaid
flowchart TB
    A[Flash 0x10017800<br/>发射端代码<br/>__comm_emit_lma] -->|memcpy| B[RAM 0x10017800<br/>__comm_emit_vma]
    C[Flash 0x10017800<br/>接收端代码<br/>__comm_apapter_lma] -->|memcpy| B
    B --> D[同 RAM region<br/>__adapter_ram_vma = __emit_ram_vma]
    F[启动时] --> G[wireless_mic_role_init<br/>wireless_proc.c:5]
    G --> H{xcfg_cb.wireless_adapter_en?}
    H -->|1| I[memcpy comm_adapter]
    H -->|0| J[memcpy comm_emit]
    I --> K[func_cb.sta = FUNC_ADAPTER]
    J --> L[func_cb.sta = FUNC_MIC_EMIT]
    classDef flash fill:#fff3cd,stroke:#ffc107
    classDef ram fill:#cfe2ff,stroke:#0d6efd
    classDef run fill:#d1e7dd,stroke:#198754
    class A,C flash
    class B,D ram
    class F,G,H,I,J,K,L run
```

**关键代码**

```c
// modules/wireless/wireless_proc.c:20-32  链接脚本符号
extern u32 __comm_adapter_vma, __comm_apapter_lma, __comm_apapter_size;
extern u32 __comm_emit_vma,    __comm_emit_lma,    __comm_emit_size;

// modules/wireless/wireless_proc.c:24-32  运行时拷贝
void wireless_mic_load_code(void) {
    load_code_wireless_mic();                   // 库函数
    if (xcfg_cb.wireless_adapter_en) {
        memcpy(&__comm_adapter_vma, &__comm_apapter_lma, (u32)&__comm_apapter_size);
    } else {
        memcpy(&__comm_emit_vma,    &__comm_emit_lma,    (u32)&__comm_emit_size);
    }
}
```

**收益**：

- **节省 Flash 空间**：两份代码只用一份 RAM
- **一份固件多种角色**：无需烧两次
- **运行时切换**：换 xcfg 即可

**新手注意**：如果你看到 `__comm_emit_*` 符号或 `AT(.text.mic_emit.proc.*)` 段名，就知道这是发射端专用代码。

---

## 第六篇 设计哲学与实战示例

### 第 21 章 · 高内聚低耦合：依赖方向、接口抽象、回调机制

**依赖方向纪律**：

```mermaid
flowchart LR
    APP[应用层 functions/] -->|只调| BSP[BSP 层 bsp/]
    APP -->|只调| MOD[模块层 modules/]
    BSP -->|只调| DRV[驱动层 driver/]
    DRV -->|只读| HAL[HAL 层 sfr.h]
    MOD -->|可调用| APP
    MOD -->|只调| BSP
    MOD -->|只调| DRV
    classDef up fill:#cfe2ff,stroke:#0d6efd
    classDef mid fill:#fff3cd,stroke:#ffc107
    classDef low fill:#e2e3e5,stroke:#6c757d
    class APP up
    class BSP,MOD mid
    class DRV,HAL low
```

**箭头规则**：

- ✅ 上层能调下层
- ❌ 下层不能调上层
- ✅ 同层之间只通过回调通信，不直接 import

**证据 1：模块层只调 BSP/Driver，绝不调应用层**

```bash
# modules/wireless/mic_proc.c 不 include 任何 functions/ 头
$ grep -r "include.*func" modules/wireless/
# （无输出）
```

**证据 2：Driver 层不调 BSP/Module**

```bash
# driver/driver_sdadc.c 只 include 它需要的头
$ grep "include" driver/driver_sdadc.c
#include "include.h"
#include "driver_sdadc.h"
#include "driver_sdadc_reg.h"
#include "driver_com.h"
```

**证据 3：HAL 不调任何上层**

```c
// driver/driver_sdadc.c 只有 SFR 读写，不会调函数
void sdadc0_cmd(SDADC_REG_TYPEDEF *sdadc, u8 en) {
    if (en) sdadc->CON |= SDADC0CON_ADC0_EN;
    else    sdadc->CON &= ~SDADC0CON_ADC0_EN;
}
```

**抽象与回调机制**

| 抽象 | 文件 | 作用 |
|---|---|---|
| `audio_callback_t` | `functions/func_mic_emit.h:11` | 统一音频回调签名 |
| `sdadc_done_callback` | `libs/cpu/api_sdadc.h:29` | SDADC 完成回调 |
| `wireless_mic_tag` | `modules/wireless/wireless.c:37` | BLE 状态结构 |
| `mic_enc_t` / `mic_dec_t` | `modules/wireless/mic_proc.c:13-44` | 编/解码上下文 |

**3 类通信机制**

1. **全局单例**（共享白板）：`sys_cb`、`xcfg_cb`、`func_cb`
2. **回调函数**（事件驱动）：`sdadc_done_callback_set(...)`
3. **消息队列**（异步通信）：`msg_queue` + `func_mic_emit_message`

### 第 22 章 · 示例 A：新增一个按键 + 消息链路（实测成功案例）

**目标**：在 mic emit 发射端，实现「K1 长按后松手」触发 `MSG_TEST_X`，串口打印 `Test X triggered`。本章按实际板卡实测通过流程记录，包含一次真实踩坑与修复。

> 选 K1 长按抬起的原因：当前开发板只有 `KEY_ID_PP`、`KEY_ID_K1`、`KEY_ID_K2` 三个有效按键；`KEY_ID_K2` 的「长按抬起」列已用于 `MSG_CHANGE_MAGIC`，故选 K1。事件列为 `KEY_LONG_UP`（长按后**松手**触发，索引 3），与 K2 的 MAGIC 一致。

#### 步骤 1：添加消息枚举（关键：值必须 < 0x100）

```c
// functions/msg.h
// 按键消息表是 u8 数组，经 bsp_key.c 的 key_set_msg_tbl 按字节拷贝，
// 再由 bsp_key_get_msg 按 u8 取值返回。因此经按键表分派的消息，
// 其枚举值必须落在 0x00~0xff，否则写入/读取会被截断成低 8 位。
// 放在 MSG_VOL_MUTE 之后（紧接 MSG_VOL_MUTE，自动取下一个值，远小于 0x100）。
enum{
    MSG_NO                  = 0,
    ...
    MSG_VOL_UP,
    MSG_VOL_DOWN,
    MSG_VOL_MUTE,
    MSG_TEST_X,                     //实测新增消息，值 = 11，满足 < 0x100

    MSG_SYS_1S = 0x700,
    ...
};
```

#### 步骤 2：修改按键消息表（mic emit 非 K12 表）

```c
// projects/microphone/port/port_key.c
// 注意：项目未定义 WIRELESS_MIC_K12_KEY_EN，编译走 #else 的非 K12 表。
// KEY_ID_K1 的「长按抬起」列（索引 3，KEY_LONG_UP）填 MSG_TEST_X。
#if WIRELESS_MIC_K12_KEY_EN
... // K12 表略
#else
const u8 mic_emit_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
        //单击,      长按下,  HOLD,    长按抬起,      双击,   三击    四击    五击
    [KEY_ID_PP] ={MSG_VOL_MUTE, MSG_NO, MSG_PWR_HOLD, MSG_NO, MSG_NO, MSG_NO, MSG_NO, MSG_NO},
    [KEY_ID_K1] ={MSG_VOL_UP,  MSG_NO, MSG_NO,      MSG_TEST_X, MSG_NO, MSG_NO, MSG_NO, MSG_NO},
    [KEY_ID_K2] ={MSG_VOL_DOWN,MSG_NO, MSG_NO,      MSG_CHANGE_MAGIC, MSG_NO, MSG_NO, MSG_NO, MSG_NO},
    [KEY_ID_K3] ={MSG_NO,      MSG_NO, MSG_NO,      MSG_NO, MSG_NO, MSG_NO, MSG_NO, MSG_NO},
};
#endif
```

#### 步骤 3：处理消息

```c
// functions/msg_mic_emit.c  // 在 func_mic_emit_message 中添加 case
case MSG_TEST_X:
    printf("Test X triggered\n");
    break;
```

#### 步骤 4：构建并测试

```bash
# 1. 用 Code::Blocks 打开 projects/microphone/app.cbp，构建 Debug 目标
# 2. 烧录 Output/bin/app.dcf
# 3. 测试 - 长按 K1 再松手，观察 UART 串口打印
```

实测：长按 K1 后松手，UART 打印 `Test X triggered`，链路打通。

```mermaid
flowchart LR
    A[硬件按键 K1] --> B[bsp_key_scan<br/>ADC/IO 消抖]
    B --> C[key_status_process<br/>识别 KEY_LONG_UP]
    C --> D[port_key.c 查表<br/>K1 长按抬起 → MSG_TEST_X]
    D --> E[msg_enqueue<br/>bsp_key.c:131]
    E --> F[func_mic_emit_message<br/>switch case]
    F --> G[printf Test X triggered]
    classDef hw fill:#e2e3e5,stroke:#6c757d
    classDef bsp fill:#fff3cd,stroke:#ffc107
    classDef app fill:#cfe2ff,stroke:#0d6efd
    class A,B,C hw
    class D,E,F,G app
```

#### 实测踩坑：消息值 ≥ 0x100 被截断，长按抬起不打印

最初把新增消息放在 `MSG_SYS_MAX = 0x7ff` 之后，命名为 `EVT_TEST_X`，结果长按抬起**完全不打印**，而同表里的 `MSG_VOL_MUTE`、`MSG_VOL_UP` 等都正常。

**根因**：`EVT_TEST_X` 紧跟 `0x7ff`，枚举值 = `0x800`（2048），超出 `u8` 范围。

链路断点逐层分析：

1. [port_key.c](projects/microphone/port/port_key.c) 的 `mic_emit_key_msg_tbl` 元素类型是 `u8`。
2. [bsp_key.c:40-53](bsp/bsp_key.c#L40-L53) `key_set_msg_tbl` 用 `memcpy` 按**字节**拷贝整表，`EVT_TEST_X(0x800)` 写入后只剩低 8 位 = `0x00`。
3. [bsp_key.c:55-67](bsp/bsp_key.c#L55-L67) `bsp_key_get_msg` 从 `u8` 元素取值返回，得到的还是 `0x00`。
4. [bsp_key.c:129-132](bsp/bsp_key.c#L129-L132) 仍执行 `msg_enqueue(0)`。
5. [func_mic_emit.c:219-222](functions/func_mic_emit.c#L219-L222) `msg = msg_dequeue()` 得到 `MSG_NO`，`if (msg != MSG_NO)` 不成立 → `func_mic_emit_message` **根本没被调用**，所以不打印。

而 `MSG_VOL_MUTE`(10)、`MSG_VOL_UP`(8) 等都 < 0x100，放 `u8` 表不丢值，因此正常。

**修复**：把测试消息挪到 `MSG_VOL_MUTE` 之后（值 = 11，满足 `< 0x100`），改名 `MSG_TEST_X`，并删除原来位于 `0x800` 的 `EVT_TEST_X`。三处同步：`msg.h` 枚举、`port_key.c` 按键表、`msg_mic_emit.c` case。

**易错点**：

- **经按键表分派的消息，枚举值必须 `< 0x100`**——这是本次踩坑的核心，`port_key.c` 表是 `u8`，`bsp_key.c` 按字节拷贝/读取。放在 `MSG_SYS_MAX = 0x7ff` 之后的 `EVT_*` 不满足此条件，不能进按键表。
- `port_key.c` 的 `KEY_MSG_MAX_IDX` 必须覆盖所有事件类型
- `msg.h` 枚举不能重复
- 按键消息表内的 `MSG_*` 必须在 `functions/msg.h` 中定义
- 注意当前板卡走非 K12 按键表（`WIRELESS_MIC_K12_KEY_EN` 未定义），且只有 PP/K1/K2 三个有效键，无 K3

### 第 23 章 · 示例 B：新增调试打印

**两种风格**：

```c
// 风格 1：直接用 printf（自动路由到 bsp_uart_debug）
// bsp_uart_debug 已在 bsp_sys.c:271 初始化
printf("value=%d state=%d\n", val, state);

// 风格 2：项目内 TRACE 宏（参考 func_mic_emit.c:6-14）
#define TRACE_EN  1
#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#define TRACE_R(...) print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif
```

**配置开关**：

```c
// projects/microphone/config_ab5766_le_mic.h:24
#define BSP_UART_DEBUG_EN  GPIO_PA4    // 改 0 关闭
```

**注意**：

- `printf` 在 ISR 中调用会阻塞（不要在 ISR 里用）
- 高频打印会拖慢主循环（避免 1ms 一次）
- 调试完记得关闭 `BSP_UART_DEBUG_EN`

### 第 24 章 · 示例 C：新增音效算法

**目标**：在 `modules/voice/` 加一个 `my_effect.c`，实现"音量减半"。

#### 步骤 1：创建 effect 文件

```c
// modules/voice/my_effect.c
#include "include.h"

static int my_effect_inited = 0;

AT(.com_text.voice.init)
void my_effect_init(u8 sample_rate, u16 samples) {
    my_effect_inited = 1;
    printf("my_effect init: %d Hz, %d samples\n", sample_rate, samples);
}

AT(.text.mic_emit.proc)
void my_effect_process(s16 *pcm, u16 samples) {
    if (!my_effect_inited) return;
    for (u16 i = 0; i < samples; i++) {
        pcm[i] = pcm[i] >> 1;        // 音量减半
    }
}
```

#### 步骤 2：插入算法链

```c
// modules/wireless/mic_proc.c:277-378  在 mic_enc_proc_cb 中
#if WIRELESS_MIC_MY_EFFECT_EN
    my_effect_process(pcm, samples);                  // 选个位置插入
#endif
```

#### 步骤 3：初始化

```c
// modules/wireless/mic_proc.c:597-675  在 mic_emit_init 中
#if WIRELESS_MIC_MY_EFFECT_EN
    my_effect_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif
```

#### 步骤 4：特性开关宏

```c
// projects/microphone/config_ab5766_le_mic.h
#define WIRELESS_MIC_MY_EFFECT_EN  1
```

```mermaid
flowchart LR
    A[SDADC PCM] --> B[算法链]
    B --> C[ains4]
    C --> D[mic_eq_drc]
    D --> E[my_effect_process<br/>音量减半]
    E --> F[lc3s_enc]
    F --> G[BLE TX]
    classDef alg fill:#d1e7dd,stroke:#198754
    classDef new fill:#fff3cd,stroke:#ffc107
    class A,B,C,D,F,G alg
    class E new
```

### 第 25 章 · 示例 D：配置一个新产品方案

**目标**：新增 `CONFIG_MY_PRODUCT = 4`，定义新产品配置。

#### 步骤 1：复制现有配置头

```bash
cp projects/microphone/config_ab5766_le_mic.h \
   projects/microphone/config_my_product.h
```

#### 步骤 2：修改特性开关

```c
// projects/microphone/config_my_product.h
#define WIRELESS_MIC_ECHO_EN  1       // 新产品开 ECHO
#define WIRELESS_MIC_MAGIC_EN  1       // 新产品开 MAGIC
#define SYS_CLK_SEL  SYS_48M           // 新产品用 48M
```

#### 步骤 3：注册到 config.h

```c
// projects/microphone/config.h:13
#define CONFIG_MY_PRODUCT  4

// projects/microphone/config.h:19-27
#if (USER_CONFIG == CONFIG_AB5766_LE_MIC)
    #include "config_ab5766_le_mic.h"
#elif (USER_CONFIG == CONFIG_MY_PRODUCT)
    #include "config_my_product.h"     // 新增
#endif
```

#### 步骤 4：调整按键表和 xcfg

```c
// projects/microphone/port/port_key.c
// 调整 mic_emit_key_msg_tbl 以匹配新硬件按键
```

```bash
# 调整 xcfg_cb 字段
# 烧录新配置到 Flash
```

#### 步骤 5：调整内存布局

```c
// projects/microphone/ram.ld
// 如果新硬件有更大 Flash / RAM，调整 MEMORY 段
```

```mermaid
flowchart LR
    A[config.h 添加 CONFIG_MY_PRODUCT] --> B[config_my_product.h 定义特性]
    B --> C[port_key.c 调整按键表]
    C --> D[xcfg 配置工具生成新 xm]
    D --> E[prebuild.bat 生成 xcfg.h]
    E --> F[改动 ram.ld 内存布局]
    F --> G[重新编译烧录]
    classDef cfg fill:#cfe2ff,stroke:#0d6efd
    classDef key fill:#fff3cd,stroke:#ffc107
    classDef run fill:#d1e7dd,stroke:#198754
    class A,B cfg
    class C,D key
    class E,F,G run
```

---

## 第七篇 收尾

### 第 26 章 · 常见陷阱 + 调试技巧 + 配套阅读路线

#### 8-10 个常见陷阱

| 陷阱 | 症状 | 解决方案 |
|---|---|---|
| **双烧 RAM 溢出** | 编译报 `region 'comm_emit' overflowed` | 减小 `WIRELESS_MIC_ECHO_EN` 等特性 |
| **xcfg 与 config 冲突** | 运行时行为与编译期宏不符 | 检查 `xcfg_cb` 是否被正确加载 |
| **AT 段名拼错** | 链接报 `undefined reference` | 核对 `ram.ld` 中的 section 收集 |
| **ISR 中调 printf** | 调试 UART 阻塞，BLE 断连 | 改为 `kick` 给线程 |
| **STRONG 函数未链接** | 5ms tick 不工作 | 检查 `strong_symbol.c` 是否有 `STRONG` 关键字 |
| **`sys_cb` 未初始化** | 系统状态乱 | 在 `bsp_sys_init` 中 memset |
| **消息队列溢出** | 按键无响应 | 队列满时及时 `msg_queue_clear` |
| **算法线程饿死** | 实时音频卡顿 | 减少算法 / 调高主频 |
| **DAC 调速失效** | 播放速度漂移 | 检查 `dac0_play_sync_fifocnt` 阈值 |
| **BLE 连接快速断开** | 接收不到数据 | 检查 `cfg_wireless_role` 和 `xcfg_cb.wireless_adapter_en` |

#### 调试技巧

1. **Printf 大法**：先打开 `BSP_UART_DEBUG_EN=1`，在关键函数加 `printf()`
2. **状态机跟踪**：用 `printf("state: %d\n", func_cb.sta)` 看状态变化
3. **内存使用**：`printf("heap: %d\n`, &__heap_size - &__heap_start)`
4. **PER 监控**：开 `WIRELESS_DUMP_EN=1` 看丢包率
5. **逻辑分析仪**：抓 SDADC/DAC 进出的 PCM 数据
6. **打断点**：在 IDE 中断到 `mic_enc_proc_cb` 看 PCM 是否正确

#### 推荐阅读顺序

```mermaid
flowchart LR
    A[1. AB5766_LE_Mic_新手开发指南.md] --> B[2. 本文档<br/>架构导览]
    B --> C[3. AB5766_发射端_Mic_Emit_实现详解.md]
    B --> D[4. AB5766_接收端_Adapter_实现详解.md]
    C --> E[5. AB5766_按键链路测试与验收指南.md]
    D --> E
    E --> F[6. AB5766_无线麦克风_双板连接与LED测试指南.md]
    classDef lvl fill:#cfe2ff,stroke:#0d6efd
    classDef mid fill:#fff3cd,stroke:#ffc107
    classDef adv fill:#d1e7dd,stroke:#198754
    class A lvl
    class B,C,D mid
    class E,F adv
```

**阅读路径建议**：

1. **先看 1（导航）**：知道有哪些文档，了解仓库长什么样
2. **再看 2（本文档）**：建立"五层架构 + 抽象模式 + 角色双烧"的整体认知
3. **然后 3 或 4**：选择一个角色（发射端/接收端），深入了解实际代码
4. **最后 5、6**：双板烧录测试，验证理解

#### 总结

**学完本文档，你应该能回答这些问题**：

- ✅ AB5766 SDK 的五层是什么？每一层做什么？
- ✅ HAL 和 Driver 的区别是什么？
- ✅ BSP 层在做什么？为什么需要 BSP？
- ✅ Module 层为什么用 init/proc/reset 三段式？
- ✅ Application 层用什么方式处理消息？
- ✅ OSAL 层用什么机制做"伪线程"？
- ✅ 同一个固件怎么分别烧成发射端和接收端？
- ✅ STRONG/WEAK 怎么覆盖库的默认实现？
- ✅ AT() 段名怎么影响代码位置？
- ✅ 一次按键和一帧音频在 SDK 中怎么流转？

**如果你都能回答，恭喜你——你已经从"零基础"进阶到"中级"了**。

继续探索的入口：

- **想深入算法**：`modules/voice/echo.c`（典型 voice 算法）
- **想深入协议**：`modules/wireless/wireless_cmd.c`（蓝牙私有命令）
- **想深入构建**：`projects/microphone/ram.ld`（链接脚本）
- **想实战演练**：编译并烧录一份固件观察 LED 闪烁

祝你在嵌入式 SDK 的世界里玩得开心！🎉

---

（本文档基于 `projects/microphone/config_ab5766_le_mic.h` 当前默认配置 + Code::Blocks 工程配置。所有文件:行号引用为仓库当时快照；如有变动请以实际代码为准。）

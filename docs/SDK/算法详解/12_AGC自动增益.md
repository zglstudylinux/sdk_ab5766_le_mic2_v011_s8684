# 12 AGC 自动增益

> 本文梳理 AGC（Automatic Gain Control，自动增益控制）模块的配置宏、调用点、内核声明、段预留与包装源码缺失状况。所有结论来自源码实际引用关系（文件、函数、行号）。

> **核心诚实标注（三重边界）**：
> 1. **包装源码 `agc.c`/`agc.h` 未纳入仓库**——全仓 `Glob **/agc*` 无匹配，[modules.h:12](../../../modules/modules.h#L12) `//#include "voice/agc.h"` 已注释。即包装层函数 `agc_audio_init`/`agc_audio_input` **既无定义也无声明**（Grep `**/*.h` 无匹配），仓库中只有调用点，无实现。
> 2. **内核声明在 `api_alg.h:104-105` 可见**（`agcInit_do`/`AgcProcess_do`），但**全仓无任何调用点**——包装层应当内部维护 `agc_cb_t` 实例并调用内核，但包装源码缺失，无法证实 `agc_audio_input` 内部确实调了 `AgcProcess_do`。即**不能把 `agc_audio_input` 的调用等同于 `AgcProcess_do` 的执行**，中间隔着一个缺失的包装层。
> 3. **map.txt 无 `agc.o`**——Grep `agc.o|agc_audio|agcInit|AgcProcess|.text.agc|.buf.agc` 全文无匹配（仅 `rf_agc_enable` 是射频 AGC，与本算法无关）。即无 AGC 翻译单元参与链接，连三段全 0 条目都没有（与 ROBOTIZATION 同）。

> **当前实测状态（CLAUDE.md）**：AGC 当前 `EN=0`，源码注释标注"调试中"。CLAUDE.md「测试纪律与算法边界」明确："AGC 当前源码标注'调试中'，可能存在实现/链接风险，应放在算法测试最后，先做构建/链接验证再决定是否刷写。"本文只记录有据可查的调用点、内核声明、段预留，不推测内部增益算法。

> **Soft Gain ≠ AGC**：Soft Gain（见 [06_EQ_DRC_PACC框架与SoftGain.md](06_EQ_DRC_PACC框架与SoftGain.md)）是经 PACC CS 链的等级化增益，当前 `EQ_DRC_EN` 内启用，有 `MSG_VOL_UP/DOWN` 控制面；AGC 是独立的自动增益反馈环，当前 `EN=0` 不编译，无控制面。两者机制不同，**Soft Gain 结果不能替代 AGC 结论**。

---

## 1. 配置宏与编译状态

### 1.1 使能宏链（当前关闭，标注调试中）

[config_ab5766_le_mic.h:109-110](../../../projects/microphone/config_ab5766_le_mic.h#L109-L110)、[165](../../../projects/microphone/config_ab5766_le_mic.h#L165)：

```c
#define WIRELESS_MIC_AGC_EN                     0               //发射端AGC算法使能 (调试中)
#define WIRELESS_MIC_AGC_DELAY                  WIRELESS_MIC_AGC_EN * 360
...
//AGC算法
#define AGC_EN                                  (WIRELESS_MIC_AGC_EN)
```

| 宏 | 定义 | 当前值 | 作用 |
|---|---|---|---|
| `WIRELESS_MIC_AGC_EN` | `[config:109]` | `0` | 发射端 AGC 算法使能（顶层开关，注释"调试中"） |
| `WIRELESS_MIC_AGC_DELAY` | `WIRELESS_MIC_AGC_EN*360` `[config:110]` | `0` | AGC 运算时间预算（us） |
| `AGC_EN` | `(WIRELESS_MIC_AGC_EN)` `[config:165]` | `0` | 模块内/段 `#if` 包裹用的实际门控 |

`AGC_EN` 在仓库中**无任何 `#if AGC_EN` 引用点**（与 `ROBOTIZATION_EN` 在 thread_alg.c:23 有 `#if ROBOTIZATION_EN` 不同）。即 `AGC_EN` 定义了但当前无消费点，可能是历史遗留或预留给未纳入仓库的 `agc.c` 内部使用。如实记录：`AGC_EN` 定义存在但无 `#if` 引用。

### 1.2 时间预算

`WIRELESS_MIC_AGC_DELAY` 在 `cfg_wireless_d2a_enc_us`（[wireless.c:19-20](../../../modules/wireless/wireless.c#L19-L20)）的 TX 编码时间预算求和中：

```c
uint16_t cfg_wireless_d2a_enc_us = (WIRELESS_MIC_ENC_MAX_US + ... + WIRELESS_MIC_AGC_DELAY
                                    + WIRELESS_MIC_SRC_DELAY + WIRELESS_MIC_DNR_FRE_DELAY);
```

当前 `EN=0`，`AGC_DELAY=0`，对时间预算无贡献。`360` 是标定的单帧 AGC 运算预算（us）。

### 1.3 当前编译状态（不编译，且无翻译单元）

`mic_proc.c:339`（process）与 `658`（init）的 `#if WIRELESS_MIC_AGC_EN` 块在 `EN=0` 下不编译。

**与 ECHO/ROOM_REVERB 的关键差异**：ECHO/ROOM_REVERB 虽 `EN=0` 不编译，但 `map.txt` 仍记录了 `room_reverb.o`/`echo.o` 三段全 0 条目（翻译单元参与链接只是内容为空）。**AGC 连这样的条目都没有**——`map.txt` Grep `agc` 无算法符号，即根本不存在 `agc.o` 翻译单元参与链接。这与"包装源码未纳入仓库"直接对应：没有 `.c` 源文件，编译器不产生 `.o`，链接器自然无条目。

### 1.4 无 stub 机制

[strong_ble.c](../../../projects/microphone/strong_ble.c) Grep `agc|AGC` 全文**无匹配**。[strong_symbol.c](../../../projects/microphone/strong_symbol.c) Grep `agc|AGC` 全文**无匹配**。即 `#if !AGC_EN`/`#if !WIRELESS_MIC_AGC_EN` 下**无任何 AGC 空 stub**。

| 模块 | `#if !*_EN` stub 数 | stub 覆盖函数 |
|---|---|---|
| ECHO | 6 | `echo_delay_level_*` + `echo_audio_mute_set` |
| MAGIC | 4 | `magic_effect_*` |
| ROOM_REVERB | 1 | `room_reverb_audio_mute_set` |
| **AGC** | **0** | **无** |

AGC 无 stub 的原因：
1. **无控制面消息**（见第 5 节）：无 `MSG_AGC_*` 驱动 level/mute 函数，故无需 level stub。
2. **调用点自包裹**：`mic_proc.c:339`/`658` 的调用都在 `#if WIRELESS_MIC_AGC_EN` 内，`EN=0` 时调用本身不编译，无需 stub 兜底链接。

---

## 2. 调用点与内核声明（包装层缺失，签名不可展开）

### 2.1 调用点：包装层函数（init + 每帧 process）

AGC 在 TX 侧有两个调用点，都是包装层函数（`agc_audio_init`/`agc_audio_input`），**实现不可见**。

**init**（[mic_proc.c:657-659](../../../modules/wireless/mic_proc.c#L657-L659)）：

```c
#if WIRELESS_MIC_AGC_EN
    agc_audio_init(0, WIRELESS_MIC_SAMPLES_SELECT);
#endif
```

位于 TX `mic_emit_init` 算法 init 序列中：
- `ains4_32k_mic_init`（[632-634](../../../modules/wireless/mic_proc.c#L632-L634)）→ `echo_audio_init`（[636-638](../../../modules/wireless/mic_proc.c#L636-L638)）→ `magic_audio_init`（[640-642](../../../modules/wireless/mic_proc.c#L640-L642)）→ `robotization_mic_init`（[644-646](../../../modules/wireless/mic_proc.c#L644-L646)）→ `room_reverb_audio_init`（[648-651](../../../modules/wireless/mic_proc.c#L648-L651)）→ `mic_eq_drc_init`（[653-655](../../../modules/wireless/mic_proc.c#L653-L655)）→ **`agc_audio_init`（657-659）** → `dnr_fre_mic_init`（[661-663](../../../modules/wireless/mic_proc.c#L661-L663)）→ `dac0_out_init`（[665-667](../../../modules/wireless/mic_proc.c#L665-L667)）。

init 序列在 EQ_DRC 之后、DNR_FRE 之前。

**参数签名推断**（与同类 init 对比）：

```c
agc_audio_init(u8 idx, u16 samples);
//             0    120
```

- `idx=0`：链路索引，与 `ains4_32k_mic_init(0, samples, 1)`、`dnr_fre_mic_init(0, samples, 0)` 同模式。
- `samples=120`（`WIRELESS_MIC_SAMPLES_SELECT`）：每帧点数。

注意 AGC init 只传 2 参数（`idx, samples`），无 `ch_mode`——与 `ains4_32k_mic_init(0, samples, 1)` 的 3 参数不同。但包装源码缺失，无法确认参数个数是否完整。

**process**（[mic_proc.c:338-340](../../../modules/wireless/mic_proc.c#L338-L340)）：

```c
#if WIRELESS_MIC_AGC_EN
        agc_audio_input(pcm, WIRELESS_MIC_SAMPLES_SELECT, 0, NULL);
#endif
```

链路位置（[mic_proc.c:334-345](../../../modules/wireless/mic_proc.c#L334-L345)）：

```
room_reverb_audio_process(pcm,120) [335]  #if WIRELESS_MIC_ROOM_REVERB_EN
agc_audio_input(pcm,120,0,NULL)    [339]  #if WIRELESS_MIC_AGC_EN   ← 本模块
bsp_huart_dump_audio(pcm,120)      [343]  #if DUMP_HUART
```

AGC 在 ROOM_REVERB 之后、DUMP_HUART 之前。

**参数签名推断**（与同类 audio_input 对比）：

```c
agc_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
//             pcm  120          0          NULL
```

- 与 `ains4_32k_mic_audio_input((u8 *)pcm, samples, 1, NULL)`、`robotization_mic_audio_input(pcm, samples, 1, NULL)`、`dnr_fre_mic_audio_input((u8 *)pcm, samples, 1, NULL)` 同签名模式（ptr, samples, ch_mode, params）。
- `ch_mode=0`：与 `dnr_fre_mic_audio_input(...,0,NULL)` 一致传 0（其他多数传 1）。`ch_mode` 语义不可展开——包装源码缺失。

### 2.2 内核声明：agcInit_do / AgcProcess_do

[api_alg.h:90-105](../../../libs/api_alg.h#L90-L105)：

```c
///AGC 算法结构体以及相关API声明
typedef struct {
    u32 sampleHz;           //语音采样率
    u8  frameMs;             //一帧语音长度，以毫秒为单位
    u16 frameSh;            //一帧short的个数， 默认一帧10ms
    u16 frameSh_true;
    s16 CompressiondB;
    s16 TargetdBfs;
    u8  limiterEnable;		//是否打开限压器
    u8  frame_2p5ms_en;
    u8	inbits24_en;
    u8  smooth_en;
    u8  agcMode;			//AGC模式
} agc_cb_t;
void agcInit_do(agc_cb_t* agc_cb);
void AgcProcess_do(s16 *input, s32 pcmLen);
```

| 项目 | 来源 | 签名 | 调用点 |
|---|---|---|---|
| `agc_cb_t` 结构体 | [api_alg.h:91-103](../../../libs/api_alg.h#L91-L103) | 11 字段（采样率/帧长/压缩比/目标电平/限压器/平滑/模式等） | 结构体本身可读 |
| `agcInit_do(agc_cb_t* agc_cb)` | [api_alg.h:104](../../../libs/api_alg.h#L104) | 单参（结构体指针） | **全仓无调用点** |
| `AgcProcess_do(s16 *input, s32 pcmLen)` | [api_alg.h:105](../../../libs/api_alg.h#L105) | 逐帧（input 指针 + pcmLen） | **全仓无调用点** |

### 2.3 关键纠偏：调用点与内核签名不符，中间隔包装层

| 侧 | 函数 | 签名 | 来源 |
|---|---|---|---|
| **调用点（包装层）** | `agc_audio_init(0, 120)` | `(u8 idx, u16 samples)` | mic_proc.c:658 |
| **调用点（包装层）** | `agc_audio_input(pcm, 120, 0, NULL)` | `(u8*, u32, int, void*)` | mic_proc.c:339 |
| **内核** | `agcInit_do(agc_cb_t*)` | `(agc_cb_t*)` | api_alg.h:104 |
| **内核** | `AgcProcess_do(s16*, s32)` | `(s16*, s32)` | api_alg.h:105 |

包装层 `agc_audio_init`/`agc_audio_input` 与内核 `agcInit_do`/`AgcProcess_do` **名称与签名均不同**。合理推断：包装层内部维护一个静态 `agc_cb_t` 实例，`agc_audio_init` 填字段后调 `agcInit_do`，`agc_audio_input` 调 `AgcProcess_do`——**但包装源码缺失，无法证实此推断**。

> **诚实边界**：不能把 `agc_audio_input`（mic_proc.c:339）的调用等同于 `AgcProcess_do`（api_alg.h:105）的执行。中间隔着一个缺失的包装层 `agc.c`。当前仓库**无法证实** AGC 内核 `AgcProcess_do` 被实际调用。

### 2.4 exit：无定义，且 reset 不调

全仓 Grep `agc.*exit` 或类似命名无匹配。`mic_emit_reset`（[mic_proc.c:677-692](../../../modules/wireless/mic_proc.c#L677-L692)）只调 `bsp_sdadc_exit`/`wireless_cmd_reset`/`dac0_out_exit`，**不调任何 agc exit 函数**。与 ECHO/MAGIC/ROOM_REVERB/ROBOTIZATION 一致：TX 音效模块均无逐会话 reset/exit 语义。

---

## 3. 无异步线程分发

[os_thread.h](../../../os/os_thread.h) Grep `agc|AGC` **无匹配**。[thread_alg.c](../../../os/thread_alg.c) Grep `agc|AGC` **无匹配**。

即 AGC：
- **无 kick 宏**（对比 AINS4_32K 的 `ains4_32k_mic_proc_kick_start()`、ROBOTIZATION 的 `robotization_mic_proc_kick_start()`）。
- **无线程分发 case 分支**（对比 `thread_alg_proc_msg_cb` 中 `case MSG_MIC_AINS4_32K`/`case MSG_MIC_ROBOTIZATION`）。

这暗示 AGC 设计为**同步每帧处理**——`agc_audio_input` 在 `mic_enc_proc_cb` 帧内直接完成处理并就地写回，不异步攒帧、不 kick 低优先级线程。与 ECHO/MAGIC/ROOM_REVERB 的同步逐点/逐帧模式一致，与 AINS4_32K/ROBOTIZATION 的异步乒乓模式不同。

> **但同步性不可断言**：包装源码缺失，`agc_audio_input` 内部是否真的同步处理未知。只记录"无线程分发基础设施"。

---

## 4. 段放置与资源预留

### 4.1 ram.ld 段预留

[ram.ld:249-254](../../../projects/microphone/ram.ld#L249-L254)（代码段，`#if WIRELESS_MIC_AGC_EN`）：

```ld
#if WIRELESS_MIC_AGC_EN
        __code_start_agc = .;
        *(.text.agc_proc*)
        *(.rodata.agc*)
        __code_end_agc = .;
#endif
```

[ram.ld:334-337](../../../projects/microphone/ram.ld#L334-L337)（缓冲段，`#if WIRELESS_MIC_AGC_EN`）：

```ld
#if WIRELESS_MIC_AGC_EN
        __buf_start_agc = .;
        *(.buf.agc*)
        __buf_end_agc = .;
#endif
```

- **代码段**：收集 `.text.agc_proc*`（处理函数）与 `.rodata.agc*`（只读数据/表）。
- **缓冲段**：收集 `.buf.agc*`（状态/缓冲，预期为 `agc_cb_t` 实例 + 内部延迟线）。
- 两段都在 `> comm_emit AT > flash` 区域内（与 ROBOTIZATION/ROOM_REVERB 等同组，TX 发射端公共段）。
- **当前 EN=0，整个 `#if` 块不预处理**，`__code_start_agc`/`__buf_start_agc` 等符号不定义，段不收集。`map.txt` 无 `.text.agc_proc`/`.buf.agc` 段条目（Grep 证实）。

### 4.2 与同类模块段预留对比

| 模块 | 代码段 | 数据段 | `#if` 包裹 | 当前状态 |
|---|---|---|---|---|
| AINS4_32K | `.text.ains4_32k_proc` | `.buf.ains4_32k` | 是 | EN=1，链接有体积 |
| ECHO | `.text.echo_proc` | `.buf.echo`/`.mram_echo` | 是 | EN=0，段预留但不收集，有 .o 三段全 0 |
| ROOM_REVERB | `.text.room_reverb_*` | `.buf.room_reverb`/`.mram_reverb` | 是 | EN=0，段预留但不收集，有 .o 三段全 0 |
| ROBOTIZATION | `.text.robotization_proc` | `.buf.robotization` | 是 | EN=0，段预留但不收集，**无 .o** |
| **AGC** | `.text.agc_proc` | `.buf.agc` | 是 | EN=0，段预留但不收集，**无 .o** |

AGC 的段预留命名模式（`.text.agc_proc`/`.buf.agc`）与 AINS4_32K（`.text.ains4_32k_proc`）、ECHO（`.text.echo_proc`）一致，说明设计上预期有 `agc.c` 用 `AT(.text.agc_proc)`/`AT(.buf.agc)` 标注函数与变量——只是该源码未纳入仓库。

> **无 MRAM 段**：与 ECHO/ROOM_REVERB 有 `.mram_echo`/`.mram_reverb`（24 KB 延迟线）不同，AGC 在 ram.ld 中**无 `.mram_agc` 段预留**。说明 AGC 不预期使用 MRAM 大延迟线（AGC 是增益反馈环，非长延迟混响，缓冲需求小），缓冲预期落在普通 RAM `.buf.agc`。这是可观测的段设计差异。

---

## 5. 控制面与按键映射（均无）

### 5.1 无控制面消息

`functions/` Grep `agc|AGC` 全文**无匹配**。[msg_mic_emit.c](../../../functions/msg_mic_emit.c) 中无 `MSG_AGC_*` 类消息，无 level 切换、无 mute 控制。

对比 TX 音效的控制面覆盖：

| 模块 | 控制面消息 | 按键映射 | 在线调试 |
|---|---|---|---|
| ECHO | `MSG_ECHO_LEVEL_UP/DOWN` | K12/K2 双击 | `CFG_ECHO` ✅ |
| MAGIC | `MSG_CHANGE_MAGIC` | K2 长按抬起 / K3 单击 | `CFG_MAGIC` ✅ |
| Soft Gain | `MSG_VOL_UP/DOWN` | K2 单击/长按 | （经 EQ_DRC） |
| Mute | `MSG_VOL_MUTE` | K1 | — |
| ROOM_REVERB | 无 | 无 | 无 |
| ROBOTIZATION | 无 | 无 | 无 |
| **AGC** | **无** | **无** | **无** |

AGC 与 ROOM_REVERB/ROBOTIZATION 一样是"三无"模块（无控制面消息、无按键、无在线调试）。AGC 作为"自动"增益控制，设计上本就不需要用户手动调等级（增益由算法自动反馈），无控制面与"自动"语义一致——但包装源码缺失，无法证实算法是否真的自动反馈。

### 5.2 无按键映射

[port_key.c](../../../projects/microphone/port/port_key.c) Grep `agc|AGC` 全文无匹配。非 K12 表（当前生效）与 K12 表均无 `MSG_AGC_*` 映射。用户无法通过按键开关/调整 AGC。

### 5.3 无在线调试回调

[effect_table.c:4-11](../../../modules/effect/effect_table.c#L4-L11) 的 `effect_update_callback_tbl` 只有 `CFG_MIC_EQ`/`CFG_MIC_DRC`/`CFG_ECHO`/`CFG_MAGIC`/`CFG_MIX_EQ`/`CFG_MIX_DRC` 六项，**无 `CFG_AGC`**。`toolkit_effect.c` 无 agc 回调函数。即无 UART 在线调参入口。

---

## 6. 完整调用链（TX 侧，当前不编译）

```mermaid
sequenceDiagram
    participant INIT as mic_emit_init
    participant WRAP_INIT as agc_audio_init<br/>(包装层, 源码缺失)
    participant KERN_INIT as agcInit_do<br/>(预编译内核 api_alg.h:104)
    participant LOOP as mic_enc_proc_cb 每帧
    participant WRAP_PROC as agc_audio_input<br/>(包装层, 源码缺失)
    participant KERN_PROC as AgcProcess_do<br/>(预编译内核 api_alg.h:105)
    Note over INIT,WRAP_INIT: #if WIRELESS_MIC_AGC_EN (=0 当前不编译)
    Note over WRAP_INIT,KERN_PROC: 包装源码 agc.c 未纳入仓库<br/>agc_audio_init/agc_audio_input 实现不可见
    Note over KERN_INIT,KERN_PROC: 内核声明在 api_alg.h 可见<br/>但全仓无调用点 (无法证实被包装层调用)
    INIT->>WRAP_INIT: agc_audio_init(0, 120) [mic_proc.c:658]
    Note over WRAP_INIT: 推断: 内部维护 agc_cb_t 实例, 填字段后调 agcInit_do<br/>(但源码缺失, 不可证实)
    WRAP_INIT->>KERN_INIT: (推断) agcInit_do(&agc_cb)
    KERN_INIT-->>WRAP_INIT: 内核初始化增益环状态
    loop 每帧 120 点
        LOOP->>WRAP_PROC: agc_audio_input(pcm, 120, 0, NULL) [mic_proc.c:339]
        Note over WRAP_PROC: 推断: 内部调 AgcProcess_do 就地写回<br/>(但源码缺失, 不可证实)
        WRAP_PROC->>KERN_PROC: (推断) AgcProcess_do(pcm, 120)
        KERN_PROC-->>WRAP_PROC: 就地写回 pcm (自动增益)
    end
    Note over INIT,LOOP: mic_emit_reset 不调任何 agc exit; 无逐会话 reset
```

---

## 7. 预编译库边界

| 项目 | 来源 | 可记录 | 不可推测 |
|---|---|---|---|
| `agc_cb_t` 结构体 | [api_alg.h:91-103](../../../libs/api_alg.h#L91-L103) | 11 字段（`sampleHz`/`frameMs`/`frameSh`/`CompressiondB`/`TargetdBfs`/`limiterEnable`/`smooth_en`/`agcMode` 等）、字段名与厂商注释 | 各字段如何被内核解释为增益环参数、`CompressiondB`/`TargetdBfs` 的定点格式与 dB 换算 |
| `agcInit_do(agc_cb_t*)` | [api_alg.h:104](../../../libs/api_alg.h#L104) | API 签名、**无调用点** | 内部增益环初始化（attack/release 系数、平滑器状态） |
| `AgcProcess_do(s16*, s32)` | [api_alg.h:105](../../../libs/api_alg.h#L105) | 逐帧处理 API、`input` 指针 + `pcmLen`、**无调用点** | 内部电平检测/增益计算/平滑/限幅的具体数学 |
| `agc_audio_init`/`agc_audio_input` | mic_proc.c:658/339（调用点） | 调用点、参数（`0,120` / `pcm,120,0,NULL`） | **定义点不可见**，与内核 `agcInit_do`/`AgcProcess_do` 的调用关系不可证实 |

> **AGC 与 ROBOTIZATION 的内核声明差异**：
> - ROBOTIZATION：`api_alg.h` Grep `robotiz` **无匹配**——连内核声明都没有，是"源码 + 内核声明 + 链接符号"三重缺失。
> - AGC：`api_alg.h:104-105` **有内核声明**（`agcInit_do`/`AgcProcess_do`），但**全仓无调用点**——是"包装源码 + 调用点间接性 + 链接符号"缺失，但内核 API 声明可见。
>
> 即 AGC 比 ROBOTIZATION 多一层可读性：内核 `agc_cb_t` 结构体与 `agcInit_do`/`AgcProcess_do` 签名可记录，但**不能证实它们被包装层调用**。

### 7.1 不可推测项

由于包装源码缺失 + 内核无调用点，以下均**不可推测**：
- `agc_audio_init` 内部是否维护静态 `agc_cb_t` 实例、如何填字段（`sampleHz=48000`？`frameMs=10`？`CompressiondB`/`TargetdBfs` 取值）。
- `agc_audio_input` 是否同步调 `AgcProcess_do` 就地写回，还是异步攒帧（无线程分发基础设施暗示同步，但不可断言）。
- AGC 内部增益环数学（电平检测 RMS/peak、增益计算曲线、attack/release 时间常数、平滑器、限幅器）。
- `agcMode` 的取值与含义（厂商注释仅"AGC 模式"）。
- `frame_2p5ms_en`/`inbits24_en`/`smooth_en` 的具体效果（字段名可推测用途但非数学证明）。

---

## 8. 知识点小结

1. **包装源码缺失（核心事实）**：`agc.c`/`agc.h` 未纳入仓库（Glob 无匹配，[modules.h:12](../../../modules/modules.h#L12) include 被注释）。包装层 `agc_audio_init`/`agc_audio_input` **既无定义也无声明**，只有调用点。不推测内部实现。
2. **内核声明可见但无调用点**：`agcInit_do`/`AgcProcess_do` 在 [api_alg.h:104-105](../../../libs/api_alg.h#L104-L105) 可读，但全仓无调用点。不能把 `agc_audio_input` 调用等同于 `AgcProcess_do` 执行——中间隔缺失的包装层。
3. **当前 EN=0 不编译**：`WIRELESS_MIC_AGC_EN=0`（[config:109](../../../projects/microphone/config_ab5766_le_mic.h#L109)），调用点 `#if` 不编译，无 undefined reference。
4. **"调试中"标注**：config 注释"发射端AGC算法使能 (调试中)"。CLAUDE.md 测试纪律：先构建/链接验证再刷写，不预设原理。
5. **map.txt 无 agc.o**：Grep `agc` 无算法符号（仅 `rf_agc_enable` 射频 AGC 无关），连三段全 0 条目都没有（与 ROBOTIZATION 同，与 ECHO/ROOM_REVERB 有 .o 三段全 0 不同）。
6. **无 stub**：strong_ble.c/strong_symbol.c 无 `#if !AGC_EN` stub（与 ROBOTIZATION 同，因调用点自包裹 + 无控制面，无需 stub）。
7. **两个调用点**：`agc_audio_init(0,120)`（[mic_proc.c:658](../../../modules/wireless/mic_proc.c#L658)）、`agc_audio_input(pcm,120,0,NULL)`（[mic_proc.c:339](../../../modules/wireless/mic_proc.c#L339)）。参数签名从调用点推断（idx,samples / ptr,samples,ch_mode,params）。
8. **链路位置**：process 在 ROOM_REVERB（[335](../../../modules/wireless/mic_proc.c#L335)）后、DUMP_HUART（[343](../../../modules/wireless/mic_proc.c#L343)）前；init 在 EQ_DRC（[655](../../../modules/wireless/mic_proc.c#L655)）后、DNR_FRE（[662](../../../modules/wireless/mic_proc.c#L662)）前。
9. **无异步线程分发**：os_thread.h/thread_alg.c Grep `agc` 无匹配。无 kick 宏、无线程 case 分支。暗示同步每帧处理，但包装源码缺失不可断言同步性。
10. **`agc_cb_t` 结构体可读**：[api_alg.h:91-103](../../../libs/api_alg.h#L91-L103)，11 字段（采样率/帧长/压缩比/目标电平/限压器/平滑/模式等），字段名与注释可记录，但内部增益环数学不可推测。
11. **无控制面/无按键/无在线调试**：msg_mic_emit.c 无 `MSG_AGC_*`；port_key.c 无映射；effect_table.c 无 `CFG_AGC`。是"三无"模块（与 ROOM_REVERB/ROBOTIZATION 同）。AGC"自动"语义与无控制面一致，但包装源码缺失无法证实自动反馈。
12. **ram.ld 段 `#if` 包裹**：`.text.agc_proc`/`.rodata.agc`/`.buf.agc`（[ram.ld:249-254](../../../projects/microphone/ram.ld#L249-L254)、[334-337](../../../projects/microphone/ram.ld#L334-L337)），归属 `comm_emit` 段组。当前 EN=0 不收集。无 `.mram_agc`（不预期 MRAM 延迟线）。
13. **exit 无定义且 reset 不调**：无 `agc_*_exit` 函数，`mic_emit_reset` 不调。与 ECHO/MAGIC/ROOM_REVERB/ROBOTIZATION 一致，TX 音效无逐会话 reset。
14. **`AGC_EN` 无 `#if` 引用点**：`AGC_EN`（[config:165](../../../projects/microphone/config_ab5766_le_mic.h#L165)）定义但仓库内无 `#if AGC_EN` 消费点（与 `ROBOTIZATION_EN` 在 thread_alg.c:23 有引用不同）。可能是历史遗留或预留给未纳入仓库的 agc.c。
15. **Soft Gain ≠ AGC**：Soft Gain 经 PACC CS 链等级化增益（当前启用，有 `MSG_VOL_*`），AGC 是独立自动增益反馈环（当前 `EN=0`，无控制面）。机制不同，Soft Gain 结果不能替代 AGC 结论。
16. **启用前必补源码**：`EN=1` 必然 undefined reference（`agc_audio_init`/`agc_audio_input` 无定义），需先补 `agc.c`。CLAUDE.md 要求 AGC 放算法测试最后、先构建/链接验证——构建验证第一步即失败，除非先补源码。

---

## 9. 延伸阅读

- [00_算法总览与全局调用关系.md](00_算法总览与全局调用关系.md)（TX 上行算法链全貌、AGC 在链中位置、"包装源码未纳入仓库"总述）
- [02_TX上行采集与编码骨架.md](02_TX上行采集与编码骨架.md)（`mic_emit_init`/`mic_enc_proc_cb` 完整 TX 链、AGC 调用点）
- [06_EQ_DRC_PACC框架与SoftGain.md](06_EQ_DRC_PACC框架与SoftGain.md)（Soft Gain 对照：同为"增益"类，但 Soft Gain 经 PACC 等级化、有控制面、当前启用；AGC 独立自动反馈、无控制面、当前 EN=0 缺失。Soft Gain ≠ AGC）
- [10_ROBOTIZATION机器人音效.md](10_ROBOTIZATION机器人音效.md)（ROBOTIZATION 对照：同为"三无"+源码缺失+无 stub+无 .o；差异在 AGC 有 api_alg.h 内核声明而 ROBOTIZATION 连内核声明都无）
- [07_ECHO混响.md](07_ECHO混响.md)（ECHO 对照：同为 TX 音效、链路相邻区域，但 ECHO 有包装源码、有控制面/stub、有 MRAM 延迟线；AGC 三无+无 MRAM）

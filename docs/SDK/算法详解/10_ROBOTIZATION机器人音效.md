# 10 ROBOTIZATION 机器人音效

> 本文梳理 ROBOTIZATION（机器人音效）模块的配置宏、调用点、异步线程分发、段预留与包装源码缺失状况。所有结论来自源码实际引用关系（文件、函数、行号）。

> **核心诚实标注**：ROBOTIZATION 的**包装层源码（`robotization.c`/`robotization.h`）未纳入仓库**——全仓 `Glob **/robotization*` 无匹配，`api_alg.h` Grep `robotiz` 无匹配，`map.txt` Grep `robotiz` 无任何符号。即三个被调函数 `robotization_mic_init`/`robotization_mic_audio_input`/`robotization_mic_proc_cb` 在仓库中**只有调用点，无定义**。当前 `WIRELESS_MIC_ROBOTIZATION_EN=0`，调用点不编译，链接器不会报 undefined；但若直接改 `EN=1` 而不补包装源码，必然链接失败。本文只记录有据可查的调用点、参数签名（从调用点推断）、段预留与线程分发，不推测内部算法。

> **当前实测状态（CLAUDE.md）**：ROBOTIZATION 当前 `EN=0`，源码注释标注"调试中"。CLAUDE.md 测试纪律明确：源码标注"调试中"的算法应先做构建/链接验证再决定是否刷写，不能预设算法原理。

---

## 1. 配置宏与编译状态

### 1.1 使能宏链（当前关闭，且标注调试中）

[config_ab5766_le_mic.h:111](../../../projects/microphone/config_ab5766_le_mic.h#L111)、[152](../../../projects/microphone/config_ab5766_le_mic.h#L152)：

```c
#define WIRELESS_MIC_ROBOTIZATION_EN            0               //发射端是否开启 机器人音效 (调试中)
...
#define ROBOTIZATION_EN                         (WIRELESS_MIC_ROBOTIZATION_EN)
```

| 宏 | 定义 | 当前值 | 作用 |
|---|---|---|---|
| `WIRELESS_MIC_ROBOTIZATION_EN` | `[config:111]` | `0` | 发射端是否开启机器人音效（顶层开关，注释"调试中"） |
| `ROBOTIZATION_EN` | `(WIRELESS_MIC_ROBOTIZATION_EN)` `[config:152]` | `0` | 模块内/线程分发 `#if` 包裹用的实际门控 |

**注意：ROBOTIZATION 没有 `WIRELESS_MIC_ROBOTIZATION_DELAY` 宏**。对比同类 TX 音效：ECHO 无独立 DELAY、MAGIC 有 `WIRELESS_MIC_MAGIC_DELAY`、ROOM_REVERB 有 `WIRELESS_MIC_ROOM_REVERB_DELAY`、AGC 有 `WIRELESS_MIC_AGC_DELAY`、ALLPASS 有 `WIRELESS_ALLPASS_FILTER_CHANGE_DELAY`。ROBOTIZATION 缺 DELAY 预算宏，与"调试中"状态一致——时间预算尚未标定或无需单独标定。记录此缺失，不臆断原因。

### 1.2 当前编译状态（不编译，且无翻译单元）

`mic_proc.c:330-332`（process）与 `644-646`（init）的 `#if WIRELESS_MIC_ROBOTIZATION_EN` 块在 `EN=0` 下不编译。`thread_alg.c:23-27` 的 `#if ROBOTIZATION_EN` 分支也不编译。

**与 ECHO/ROOM_REVERB 的关键差异**：ECHO/ROOM_REVERB 虽 `EN=0` 不编译，但 `map.txt` 仍记录了 `room_reverb.o`/`echo.o` 的三段全 0 条目（[map.txt:1125-1127](../../../projects/microphone/Output/bin/map.txt#L1125-L1127) `room_reverb.o` .text/.data/.bss 全 0），说明这些翻译单元的 `.o` 文件参与了链接只是内容为空。**ROBOTIZATION 连这样的条目都没有**——`map.txt` Grep `robotiz` 全文无匹配，即根本不存在 `robotization.o` 翻译单元参与链接。这与"包装源码未纳入仓库"直接对应：没有 `.c` 源文件，编译器不会产生 `.o`，链接器自然无条目。

### 1.3 无 stub 机制

[strong_ble.c](../../../projects/microphone/strong_ble.c) Grep `robotization|ROBOTIZATION` 全文**无匹配**。即 `#if !ROBOTIZATION_EN` 下**没有任何 ROBOTIZATION 空 stub**。

| 模块 | `#if !*_EN` stub 数 | stub 覆盖函数 |
|---|---|---|
| ECHO | 6 | `echo_delay_level_set/down/up/get/is_max_min` + `echo_audio_mute_set` |
| MAGIC | 4 | `magic_effect_level_set` + `magic_audio_mute_set` + `magic_effect_level_change` + `magic_effect_level_get` |
| ROOM_REVERB | 1 | `room_reverb_audio_mute_set` |
| **ROBOTIZATION** | **0** | **无** |

ROBOTIZATION 无 stub 的原因有二：
1. **无控制面消息**（见第 5 节）：没有 `MSG_ROBOTIZATION_*` 消息驱动 level 函数，故无需 level stub（与 ROOM_REVERB 无 level stub 同理）。
2. **调用点自包裹**：`mic_proc.c:331`/`645` 与 `thread_alg.c:25` 的调用都在 `#if WIRELESS_MIC_ROBOTIZATION_EN`/`#if ROBOTIZATION_EN` 内，`EN=0` 时调用本身不编译，无需 stub 兜底链接。

但需注意：若未来补了控制面消息（如 `MSG_ROBOTIZATION_LEVEL`）在 `msg_mic_emit.c` 中调用某 robotization level 函数，而该函数不在 `#if` 内，则 `EN=0` 时会链接失败——届时才需补 stub。当前不存在此情况。

---

## 2. 调用点与参数签名（从调用点推断，实现不可见）

由于包装源码缺失，三个被调函数的**实现体不可见**，只能从调用点反推参数签名与调用模式。

### 2.1 init：mic_emit_init 内

[mic_proc.c:644-646](../../../modules/wireless/mic_proc.c#L644-L646)：

```c
#if WIRELESS_MIC_ROBOTIZATION_EN
    robotization_mic_init(0, WIRELESS_MIC_SAMPLES_SELECT, 1);
#endif
```

调用位置在 TX 上行算法 init 序列中：
- `ains4_32k_mic_init`（[632-634](../../../modules/wireless/mic_proc.c#L632-L634)）→ `echo_audio_init`（[636-638](../../../modules/wireless/mic_proc.c#L636-L638)）→ `magic_audio_init`（[640-642](../../../modules/wireless/mic_proc.c#L640-L642)）→ **`robotization_mic_init`（644-646）** → `room_reverb_audio_init`（[648-651](../../../modules/wireless/mic_proc.c#L648-L651)）→ `mic_eq_drc_init`（[653-655](../../../modules/wireless/mic_proc.c#L653-L655)）→ `agc_audio_init`（[657-659](../../../modules/wireless/mic_proc.c#L657-L659)）→ ...

init 序列在 MAGIC 之后、ROOM_REVERB 之前。

**参数签名推断**（与同类 init 对比）：

```c
robotization_mic_init(u8 idx, u16 samples, int ch_mode);
//                     0       120                1
```

- `idx=0`：链路索引，与 `ains4_32k_mic_init(0, samples, 1)`、`dnr_fre_mic_init(0, samples, 0)`、`mic_eq_drc_init(0, samples, 1)` 同模式。
- `samples=120`（`WIRELESS_MIC_SAMPLES_SELECT`）：每帧点数。
- `ch_mode=1`：声道模式。与 `ains4_32k_mic_init(...,1)`、`mic_eq_drc_init(...,1)` 一致（传 1），`dnr_fre_mic_init(...,0)` 传 0。`ch_mode` 的具体语义（1=立体声？1=左声道？）不可展开——包装源码缺失。

### 2.2 process：每帧 mic_enc_proc_cb

[mic_proc.c:330-332](../../../modules/wireless/mic_proc.c#L330-L332)：

```c
#if WIRELESS_MIC_ROBOTIZATION_EN
        robotization_mic_audio_input(pcm, WIRELESS_MIC_SAMPLES_SELECT, 1, NULL);
#endif
```

链路位置（[mic_proc.c:326-340](../../../modules/wireless/mic_proc.c#L326-L340)）：

```
magic_audio_process(pcm,120)      [327]  #if WIRELESS_MIC_MAGIC_EN
robotization_mic_audio_input(...)  [331]  #if WIRELESS_MIC_ROBOTIZATION_EN   ← 本模块
room_reverb_audio_process(pcm,120) [335]  #if WIRELESS_MIC_ROOM_REVERB_EN
agc_audio_input(...)               [339]  #if WIRELESS_MIC_AGC_EN
```

ROBOTIZATION 在 MAGIC 之后、ROOM_REVERB 之前。

**参数签名推断**（与同类 audio_input 对比）：

```c
robotization_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params);
//                            pcm  120          1          NULL
```

- 与 `ains4_32k_mic_audio_input((u8 *)pcm, samples, 1, NULL)`、`dnr_fre_mic_audio_input((u8 *)pcm, samples, 1, NULL)`、`agc_audio_input(pcm, samples, 0, NULL)` 同签名模式（ptr, samples, ch_mode, params）。
- `params=NULL`：回调/扩展参数，当前不传。

> **同步 vs 异步不可断言**：`ains4_32k_mic_audio_input`（[ains4_32k.c:59](../../../modules/voice/ains4_32k.c#L59)）是**异步**的——它内部用乒乓 `tog_buf` 攒帧，攒满后 `ains4_32k_mic_proc_kick_start()`（[75](../../../modules/voice/ains4_32k.c#L75)）kick 低优先级线程做实际算法，`audio_input` 本身只做缓存搬运。ROBOTIZATION 的 `robotization_mic_audio_input` 是否同样异步？**不可断言**——包装源码缺失，无法看其内部是否用 tog_buf、是否 kick。但第 3 节的 kick 宏与线程分发分支存在，暗示设计上**预留了异步路径**，只是 kick 宏当前无调用点（见第 3 节纠偏）。

### 2.3 exit：无定义，且 reset 不调

全仓 Grep `robotization.*exit` 或类似命名无匹配。`mic_emit_reset`（[mic_proc.c:677-692](../../../modules/wireless/mic_proc.c#L677-L692)）不调任何 robotization exit 函数。与 ECHO/MAGIC/ROOM_REVERB 一致：TX 音效模块均无逐会话 reset/exit 语义。

---

## 3. 异步线程分发（预留但 kick 无调用点）

ROBOTIZATION 在线程分发层有完整预留，与 AINS4_32K 对称，但**关键差异：kick 宏定义存在，但全仓无任何 `.c` 调用此 kick 宏**。

### 3.1 消息定义与 kick 宏

[os_thread.h:6-17](../../../os/os_thread.h#L6-L17)：

```c
enum {
    MSG_MIC_DEC0 = 0,
    MSG_MIC_DEC1,
    MSG_MIC_AINS4_32K,
};                          // 注意：MSG_MIC_ROBOTIZATION 不在此 enum

void mic_dec_kick_cb(u8 msg);
void mic_alg_kick_cb(u8 msg);
#define kick_dec_prio_trans(idx)                   mic_dec_kick_cb(MSG_MIC_DEC0 + (idx))

#define robotization_mic_proc_kick_start()         mic_alg_kick_cb(MSG_MIC_ROBOTIZATION)
#define ains4_32k_mic_proc_kick_start()            mic_alg_kick_cb(MSG_MIC_AINS4_32K)
```

- **`MSG_MIC_ROBOTIZATION` 不在 enum 中**：`MSG_MIC_DEC0=0`/`MSG_MIC_DEC1=1`/`MSG_MIC_AINS4_32K=2`，enum 到 2 结束，没有 `MSG_MIC_ROBOTIZATION=3`。`robotization_mic_proc_kick_start()` 宏把 `MSG_MIC_ROBOTIZATION`（一个未在 enum 显式赋值的符号）传给 `mic_alg_kick_cb`。这意味着 `MSG_MIC_ROBOTIZATION` 要么在别处 `#define`，要么依赖 enum 隐式延续（若在 enum 内加 `MSG_MIC_ROBOTIZATION,` 会自动 =3）。**全仓 Grep `MSG_MIC_ROBOTIZATION` 只在 os_thread.h:16（宏定义处）与 thread_alg.c:24（case 分支）出现**，未见 `#define MSG_MIC_ROBOTIZATION` 显式定义——它很可能是某个被注释或被 `#if ROBOTIZATION_EN` 包裹的 enum 项（当前不编译故 Grep 不到定义点）。如实记录：`MSG_MIC_ROBOTIZATION` 的数值定义点当前不可见，但其作为 case 标签在 `thread_alg.c:24` 被引用。
- **kick 宏定义存在**：`robotization_mic_proc_kick_start()` = `mic_alg_kick_cb(MSG_MIC_ROBOTIZATION)`（[os_thread.h:16](../../../os/os_thread.h#L16)）。

### 3.2 线程分发分支

[thread_alg.c:4-31](../../../os/thread_alg.c#L4-L31)：

```c
AT(.com_text.thread)
void thread_alg_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case MSG_MIC_AINS4_32K:
            ains4_32k_mic_proc_cb();
            break;

#if ROBOTIZATION_EN
        case MSG_MIC_ROBOTIZATION:
            robotization_mic_proc_cb();
            break;
#endif
        default:
            break;
    }
}
```

- `#if ROBOTIZATION_EN` 包裹（[23](../../../os/thread_alg.c#L23)）：当前 `EN=0` 不编译此 case。
- `robotization_mic_proc_cb()`（[25](../../../os/thread_alg.c#L25)）：无参异步处理回调，与 `ains4_32k_mic_proc_cb()` 同模式。**实现不可见**（包装源码缺失）。

### 3.3 关键纠偏：kick 宏无调用点（与 AINS4_32K/DNR_FRE 对比）

全仓 Grep `robotization_mic_proc_kick_start` 调用点：

| 模块 | kick 宏 | 宏定义处 | 实际调用处 |
|---|---|---|---|
| AINS4_32K | `ains4_32k_mic_proc_kick_start()` | [os_thread.h:17](../../../os/os_thread.h#L17) | [ains4_32k.c:75](../../../modules/voice/ains4_32k.c#L75) ✅ |
| DNR_FRE | `dnr_fre_mic_proc_kick_start()` | （DNR_FRE 自己的头） | [dnr_fre.c:73](../../../modules/voice/dnr_fre.c#L73) ✅ |
| ROBOTIZATION | `robotization_mic_proc_kick_start()` | [os_thread.h:16](../../../os/os_thread.h#L16) | **无** ❌ |

即 `robotization_mic_proc_kick_start()` 宏定义了，但**没有任何 `.c` 文件调用它**。对比 AINS4_32K：`ains4_32k_mic_audio_input`（[ains4_32k.c:71-75](../../../modules/voice/ains4_32k.c#L71-L75)）在乒乓 buf 攒满帧时调 `ains4_32k_mic_proc_kick_start()` 触发异步处理。ROBOTIZATION 的 kick 宏无人调用，意味着即使 `EN=1` 且补了包装源码，若包装层内部不调 `robotization_mic_proc_kick_start()`，异步线程 `thread_alg_proc_msg_cb` 永远收不到 `MSG_MIC_ROBOTIZATION`，`robotization_mic_proc_cb` 永不执行——异步路径断线。

> **诚实边界**：这条"kick 无调用点"是当前仓库的**可观测事实**，但它可能是因为**包装源码缺失**（若 robotization.c 存在，其 `audio_input` 内部应当像 ains4_32k.c:75 那样调 kick 宏，只是该源码不在仓库里所以 Grep 不到）。即"kick 无调用点"与"包装源码缺失"可能是**同一事实的两个侧面**——源码不在仓库，自然其内部的 kick 调用也 Grep 不到。不据此断言"异步路径设计断线"，只记录"当前仓库内无法证实 kick 被调用"。

---

## 4. 段放置与资源预留

### 4.1 ram.ld 段预留

[ram.ld:242-247](../../../projects/microphone/ram.ld#L242-L247)（代码段）：

```ld
#if WIRELESS_MIC_ROBOTIZATION_EN
        __code_start_robotization = .;
        *(.text.robotization_proc*)
        *(.rodata.robotization*)
        __code_end_robotization = .;
#endif
```

[ram.ld:308-312](../../../projects/microphone/ram.ld#L308-L312)（数据段）：

```ld
#if WIRELESS_MIC_ROBOTIZATION_EN
        __buf_start_robotization = .;
        *(.buf.robotization*)
        __buf_end_robotization = .;
#endif
```

- **代码段**：收集 `.text.robotization_proc*`（处理函数）与 `.rodata.robotization*`（只读数据/表）。
- **数据段**：收集 `.buf.robotization*`（状态/缓冲）。
- 两段都在 `> comm_emit AT > flash`（[ram.ld:267](../../../projects/microphone/ram.ld#L267)）区域内，即 ROBOTIZATION 归属 `comm_emit` 段组（TX 发射端公共段），与 AINS4_32K/ECHO/MAGIC/ROOM_REVERB/AGC 同组。
- **当前 EN=0，整个 `#if` 块不预处理**，`__code_start_robotization`/`__buf_start_robotization` 等符号不定义，段不收集。`map.txt` 无 `.text.robotization_proc`/`.buf.robotization` 段条目（Grep 证实）。

### 4.2 与同类模块段预留对比

| 模块 | 代码段 | 数据段 | 当前状态 |
|---|---|---|---|
| AINS4_32K | `.text.ains4_32k_proc` 等 | `.buf.ains4_32k` | EN=1，链接有体积 |
| ECHO | `.text.echo_proc` 等 | `.buf.echo`/`.mram_echo` | EN=0，段预留但不收集 |
| MAGIC | `.text.pitch_shift2_*` | `.buf.pitch_shift.cache` | EN=1，链接有体积 |
| ROOM_REVERB | `.text.room_reverb_*` | `.buf.room_reverb`/`.mram_reverb` | EN=0，段预留但不收集 |
| **ROBOTIZATION** | `.text.robotization_proc` | `.buf.robotization` | **EN=0，段预留但不收集，且无 .o 翻译单元** |

ROBOTIZATION 的段预留命名模式（`.text.robotization_proc`/`.buf.robotization`）与 AINS4_32K（`.text.ains4_32k_proc`/`.buf.ains4_32k`）、ECHO（`.text.echo_proc`）一致，说明设计上预期有 `robotization.c` 用 `AT(.text.robotization_proc)`/`AT(.buf.robotization)` 标注函数与变量——只是该源码未纳入仓库。

> **无 MRAM 段**：与 ECHO/ROOM_REVERB 有 `.mram_echo`/`.mram_reverb`（[map.txt:2788-2794](../../../projects/microphone/Output/bin/map.txt#L2788-L2794)）不同，ROBOTIZATION 在 ram.ld 中**无 `.mram_robotization` 段预留**。说明该模块不预期使用 MRAM 延迟线（与 ECHO/ROOM_REVERB 这类需大延迟线的混响模块不同），缓冲预期落在普通 RAM `.buf.robotization`。这是可观测的段设计差异，不据此断言算法类型。

---

## 5. 控制面与按键映射（均无）

### 5.1 无控制面消息

`functions/` 目录 Grep `robotization|ROBOTIZATION|MSG_ROBOT` 全文**无匹配**。[msg_mic_emit.c](../../../functions/msg_mic_emit.c) 中无 `MSG_ROBOTIZATION_*` 类消息，无 level 切换、无 mute 控制。

对比 TX 音效的控制面覆盖：

| 模块 | 控制面消息 | 按键映射 | 在线调试 |
|---|---|---|---|
| ECHO | `MSG_ECHO_LEVEL_UP/DOWN` | K12/K2 双击 | `CFG_ECHO` ✅ |
| MAGIC | `MSG_CHANGE_MAGIC` | K2 长按抬起 / K3 单击 | `CFG_MAGIC` ✅ |
| Soft Gain | `MSG_VOL_UP/DOWN` | K2 单击/长按 | （经 EQ_DRC） |
| Mute | `MSG_VOL_MUTE` | K1 | — |
| ROOM_REVERB | 无 | 无 | 无 |
| **ROBOTIZATION** | **无** | **无** | **无** |

ROBOTIZATION 与 ROOM_REVERB 一样是"三无"模块（无控制面消息、无按键、无在线调试）。但 ROOM_REVERB 至少有包装源码（room_reverb.c 在仓库，只是 EN=0 不编译），ROBOTIZATION 连包装源码都没有。这是更深的"未完成"状态。

### 5.2 无按键映射

[port_key.c](../../../projects/microphone/port/port_key.c) Grep `robotization|ROBOT|MSG_ROBOT` 全文无匹配。非 K12 表（当前生效）与 K12 表均无 `MSG_ROBOTIZATION_*` 映射。用户无法通过按键切换/开关 ROBOTIZATION。

### 5.3 无在线调试回调

[effect_table.c:4-11](../../../modules/effect/effect_table.c#L4-L11) 的 `effect_update_callback_tbl` 只有 `CFG_MIC_EQ`/`CFG_MIC_DRC`/`CFG_ECHO`/`CFG_MAGIC`/`CFG_MIX_EQ`/`CFG_MIX_DRC` 六项，**无 `CFG_ROBOTIZATION`**。`toolkit_effect.c` 无 robotization 回调函数。即无 UART 在线调参入口。

---

## 6. 完整调用链（TX 侧，当前不编译）

```mermaid
sequenceDiagram
    participant INIT as mic_emit_init
    participant WRAP as robotization_mic_init<br/>(包装层, 源码缺失)
    participant LOOP as mic_enc_proc_cb 每帧
    participant INPUT as robotization_mic_audio_input<br/>(包装层, 源码缺失)
    participant KICK as robotization_mic_proc_kick_start<br/>(宏定义, 无调用点)
    participant THREAD as thread_alg_proc_msg_cb
    participant CB as robotization_mic_proc_cb<br/>(包装层, 源码缺失)
    Note over INIT,WRAP: #if WIRELESS_MIC_ROBOTIZATION_EN (=0 当前不编译)
    Note over WRAP,CB: 包装源码 robotization.c 未纳入仓库, 三个函数实现不可见
    INIT->>WRAP: robotization_mic_init(0, 120, 1) [mic_proc.c:645]
    Note over WRAP: 实现不可见, 参数语义不可展开
    loop 每帧 120 点
        LOOP->>INPUT: robotization_mic_audio_input(pcm, 120, 1, NULL) [mic_proc.c:331]
        Note over INPUT: 实现不可见; 是否用 tog_buf 异步攒帧未知
        Note over KICK: kick 宏定义于 os_thread.h:16<br/>但全仓无 .c 调用此宏
        Note over KICK: (若包装层内部调 kick) → mic_alg_kick_cb(MSG_MIC_ROBOTIZATION)
        Note over THREAD: #if ROBOTIZATION_EN case 分支当前不编译
        THREAD->>CB: robotization_mic_proc_cb() (异步, 实现不可见)
    end
    Note over INIT,LOOP: mic_emit_reset 不调任何 robotization exit; 无逐会话 reset
```

---

## 7. 包装源码缺失边界（核心诚实标注）

### 7.1 三重证据证实源码缺失

| 检查 | 工具 | 结果 | 含义 |
|---|---|---|---|
| 源文件 | `Glob **/robotization*` | No files found | 无 `robotization.c`/`robotization.h` |
| 内核声明 | `Grep robotiz` in `libs/api_alg.h` | No matches | `api_alg.h` 无 robotization 内核 API（与 `ains4_init`/`pitch_shift2_init`/`reverb_buf_init` 不同） |
| 链接符号 | `Grep robotiz` in `map.txt` | No matches | 无任何 robotization 符号，连未链接 `.o` 三段全 0 条目都没有 |

对比已纳入仓库的同类模块：

| 模块 | 包装源码 | 内核声明（api_alg.h） | map.txt 符号 |
|---|---|---|---|
| AINS4_32K | `modules/voice/ains4_32k.c` ✅ | `ains4_init`/`ains4_process` ✅ | 链接有体积 ✅ |
| PLC | `modules/voice/plc_soft.c` ✅ | `plc_soft_init_do`/`process_do` ✅ | 链接有体积 ✅ |
| ECHO | `modules/voice/echo.c` ✅ | `echo_process_16bit` 等 ✅ | EN=0 三段全 0 但有条目 |
| ROOM_REVERB | `modules/voice/room_reverb.c` ✅ | `room_reverb_process`/`reverb_buf_init` ✅ | EN=0 三段全 0 但有条目 |
| MAGIC | `modules/voice/magic.c` ✅ | `pitch_shift2_init`/`process` ✅ | 链接有体积 ✅ |
| DNR_FRE | `modules/voice/dnr_fre.c` ✅ | （其内核在别处） | EN=0 但有 .o |
| **ROBOTIZATION** | **无** ❌ | **无** ❌ | **无** ❌ |

ROBOTIZATION 是 TX 算法中**唯一三重缺失**（源码 + 内核声明 + 链接符号全无）的模块。

### 7.2 启用前的必要条件

若要使能 ROBOTIZATION（`WIRELESS_MIC_ROBOTIZATION_EN: 0 → 1`），仅改宏**必然链接失败**，因为：

1. `mic_proc.c:645` `robotization_mic_init(0, 120, 1)` —— undefined reference。
2. `mic_proc.c:331` `robotization_mic_audio_input(pcm, 120, 1, NULL)` —— undefined reference。
3. `thread_alg.c:25` `robotization_mic_proc_cb()` —— undefined reference（`#if ROBOTIZATION_EN` 会编译此 case）。
4. `MSG_MIC_ROBOTIZATION` 符号需有定义（enum 项或 `#define`），否则 `os_thread.h:16` 宏与 `thread_alg.c:24` case 标签报未定义。

即需先补 `robotization.c`（定义三个函数 + 可能的 `AT(.text.robotization_proc)`/`AT(.buf.robotization)` 段标注 + 可能的 `MSG_MIC_ROBOTIZATION` enum 项补全 + 内部 kick 调用），才能 `EN=1` 构建。

> **CLAUDE.md 测试纪律直接对应**：CLAUDE.md「测试纪律与算法边界」明确"AGC 当前源码标注'调试中'，可能存在实现/链接风险，应放在算法测试最后，先做构建/链接验证再决定是否刷写"。ROBOTIZATION 同样标注"调试中"（[config:111](../../../projects/microphone/config_ab5766_le_mic.h#L111)），且源码根本不在仓库，链接风险比 AGC 更确定（AGC 至少 `agc_audio_init`/`agc_audio_input` 的调用点在，只是包装源码状态待查；ROBOTIZATION 是明确缺失）。P7 测试卡片要求"先构建/链接验证，不能预设算法原理"——对 ROBOTIZATION 而言，构建验证的第一步就会失败（undefined reference），除非先补源码。

### 7.3 不可推测项

由于三重缺失，以下均**不可推测**：
- `robotization_mic_init` 内部如何初始化（是否有 `memset` 状态、是否调某内核 init、参数 `ch_mode=1` 如何解释）。
- `robotization_mic_audio_input` 是否异步（是否用 `tog_buf` 攒帧、是否调 kick 宏、`PROCESS_OUT_SAMPLES`/`FRAME_LEN` 取值）。
- `robotization_mic_proc_cb` 内部算法（机器人音效的数学原理——是移频？是环形调制 ring modulation？是频谱整形？是 LPC 残差激励？）。"机器人音效"是功能命名（厂商注释），不等于已验证的算法类型。
- 是否有 mute/level/参数表（MAGIC 有 `magic_effect_tbl`、ECHO 有 `echo_delay_level`，ROBOTIZATION 是否有类似表不可见）。
- 缓冲大小（文件头无 `buf:` 注释可引用，因无源文件）。

---

## 8. 知识点小结

1. **三重缺失（核心事实）**：ROBOTIZATION 包装源码（`robotization.c`）、`api_alg.h` 内核声明、`map.txt` 链接符号**全无**。是 TX 算法中唯一三重缺失的模块。不推测内部实现。
2. **当前 EN=0 不编译**：`WIRELESS_MIC_ROBOTIZATION_EN=0`（[config:111](../../../projects/microphone/config_ab5766_le_mic.h#L111)），调用点 `#if` 不编译，无 undefined reference。
3. **"调试中"标注**：config 注释"机器人音效 (调试中)"。CLAUDE.md 测试纪律：先构建/链接验证再刷写，不预设原理。
4. **无 DELAY 宏**：ROBOTIZATION 无 `WIRELESS_MIC_ROBOTIZATION_DELAY` 预算宏（与 MAGIC/ROOM_REVERB/AGC/ALLPASS 不同），与调试中状态一致。
5. **三个调用点**：`robotization_mic_init`（[mic_proc.c:645](../../../modules/wireless/mic_proc.c#L645)）、`robotization_mic_audio_input`（[mic_proc.c:331](../../../modules/wireless/mic_proc.c#L331)）、`robotization_mic_proc_cb`（[thread_alg.c:25](../../../os/thread_alg.c#L25)）。参数签名从调用点推断（idx,samples,ch_mode / ptr,samples,ch_mode,params / void）。
6. **链路位置**：process 在 MAGIC（[327](../../../modules/wireless/mic_proc.c#L327)）后、ROOM_REVERB（[335](../../../modules/wireless/mic_proc.c#L335)）前；init 同序（[641](../../../modules/wireless/mic_proc.c#L641) 后、[649](../../../modules/wireless/mic_proc.c#L649) 前）。
7. **异步线程预留**：kick 宏 `robotization_mic_proc_kick_start()`（[os_thread.h:16](../../../os/os_thread.h#L16)）+ 线程分发 `case MSG_MIC_ROBOTIZATION`（[thread_alg.c:23-27](../../../os/thread_alg.c#L23-L27)）。设计上预留异步路径。
8. **kick 宏无调用点**：全仓无 `.c` 调用 `robotization_mic_proc_kick_start()`（对比 AINS4_32K 在 [ains4_32k.c:75](../../../modules/voice/ains4_32k.c#L75) 实际调用、DNR_FRE 在 [dnr_fre.c:73](../../../modules/voice/dnr_fre.c#L73) 实际调用）。可能与包装源码缺失同因（源码不在则其内部 kick 调用也 Grep 不到）。
9. **`MSG_MIC_ROBOTIZATION` 定义点不可见**：不在 os_thread.h enum 内（enum 到 `MSG_MIC_AINS4_32K=2` 结束），全仓无 `#define MSG_MIC_ROBOTIZATION`。可能在被 `#if` 包裹的 enum 项中（当前不编译）。作为 case 标签在 thread_alg.c:24 被引用。
10. **无 stub**：strong_ble.c 无 `#if !ROBOTIZATION_EN` stub（对比 ECHO 6 个、MAGIC 4 个、ROOM_REVERB 1 个）。因调用点自包裹 + 无控制面，无需 stub 兜底。
11. **无控制面/无按键/无在线调试**：msg_mic_emit.c 无 `MSG_ROBOTIZATION_*`；port_key.c 无映射；effect_table.c 无 `CFG_ROBOTIZATION`。是"三无"模块（与 ROOM_REVERB 同，但 ROOM_REVERB 有包装源码）。
12. **ram.ld 段预留存在**：`.text.robotization_proc`/`.rodata.robotization`/`.buf.robotization`（[ram.ld:242-247](../../../projects/microphone/ram.ld#L242-L247)、[308-312](../../../projects/microphone/ram.ld#L308-L312)），归属 `comm_emit` 段组。当前 EN=0 不收集。无 `.mram_robotization`（不预期 MRAM 延迟线）。
13. **exit 无定义且 reset 不调**：无 `robotization_*_exit` 函数，`mic_emit_reset` 不调。与 ECHO/MAGIC/ROOM_REVERB 一致，TX 音效无逐会话 reset。
14. **启用前必补源码**：`EN=1` 必然 undefined reference（三个函数 + `MSG_MIC_ROBOTIZATION` 符号），需先补 `robotization.c`。P7 测试卡片构建验证第一步即失败，除非先补源码。

---

## 9. 延伸阅读

- [00_算法总览与全局调用关系.md](00_算法总览与全局调用关系.md)（TX 上行算法链全貌、ROBOTIZATION 在链中位置、"包装源码未纳入仓库"总述）
- [02_TX上行采集与编码骨架.md](02_TX上行采集与编码骨架.md)（`mic_emit_init`/`mic_enc_proc_cb` 完整 TX 链、ROBOTIZATION 调用点）
- [03_AINS4_32K降噪.md](03_AINS4_32K降噪.md)（AINS4_32K 对照：同样有 kick 宏 + 异步线程分发，但 AINS4_32K 包装源码在仓库、kick 实际被调用；ROBOTIZATION 源码缺失、kick 无调用点）
- [09_MAGIC魔音变调.md](09_MAGIC魔音变调.md)（MAGIC 对照：同为 TX 音效、链路相邻（MAGIC→ROBOTIZATION），但 MAGIC 源码在仓库且 EN=1 启用、有控制面与在线调试；ROBOTIZATION 三重缺失）
- [08_ROOM_REVERB房间混响.md](08_ROOM_REVERB房间混响.md)（ROOM_REVERB 对照：同为"三无"模块——无控制面/无按键/无在线调试，但 ROOM_REVERB 有包装源码只是 EN=0；ROBOTIZATION 连源码都没有）
- [12_AGC自动增益.md](12_AGC自动增益.md)（AGC 对照：同样标注"调试中"，但 AGC 的调用点与 stub 状态需单独核实）

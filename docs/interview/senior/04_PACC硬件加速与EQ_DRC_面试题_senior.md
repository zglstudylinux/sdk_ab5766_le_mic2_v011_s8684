# 04 PACC 硬件加速与 EQ/DRC 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 音频算法方向
> 核心考察：PACC CS 链表模型、PACC0 复用、Soft Gain 门控耦合、Q 值定点、资源加载

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，TX/RX 共用 PACC0 硬件加速器。
- 架构特征：PACC 用 CS（Control State）链表描述音频处理链，支持 EQ/DRC/MDRC/FSH/FIR，数据格式支持 S16_Q15/S32_Q15/S32_Q23 等。
- 本卷重点：CS 链表与 Q 格式递进、PACC0 按位图分时复用、Soft Gain 与 PACC 的双重门控、effect.bin 资源加载。
- 核心源码入口：
  - `libs/cpu/api_pacc.h`
  - `modules/effect/mic_effect.c:19-103,164-187`
  - `modules/audio/mic_eq_drc.c:19-48`
  - `modules/audio/soft_gain.c:110-128`
  - `modules/tool/toolkit_effect.c`

---

## 1. 概念理解题

### 题目 4.1 PACC 与 CS 链表模型（★★★★ / 概念理解）

**题目**：什么是 PACC？CS（Control State）链表模型与"函数调用栈"有什么本质区别？

**考察点**：
- 主考点：硬件加速器抽象模型；
- 副考点：CS 与函数调用的数据流差异；
- 区分点：能否说出 CS 是"数据流图节点"而非"函数调用"。

**参考答案**：
PACC 是芯片内置的音频算法硬件加速器，通过 CS 链表描述处理链：每个 CS 是一个处理节点（EQ/DRC/FSH 等），包含输入/输出缓冲、格式、参数、指向下一个 CS 的指针。`pacc_ctl_proc()` 把链表交给 PACC 控制器，硬件按顺序执行，无需 CPU 逐点调用函数。这与函数调用栈的区别：CS 是数据流驱动的静态描述，函数调用是控制流驱动的动态调用。

**评估标准**：
- ⭐⭐⭐ 能对比数据流 vs 控制流；
- ⭐⭐ 能说出"链表"；
- ⭐ 完全不知道。

---

### 题目 4.2 PACC 数据格式（★★★ / 概念理解）

**题目**：列出 PACC EQ/DRC 支持的数据格式（`S16_Q15_SAT`、`S32_Q23_SAT`、`S32_Q15`、`S32_Q23`），并说明各自的位宽与小数位数。

**考察点**：
- 主考点：定点 Q 格式；
- 副考点：EQ/DRC 内部精度需求；
- 区分点：能否区分"饱和"与"非饱和"版本。

**参考答案**：
| 格式 | 位宽 | 小数位 | 特点 |
|---|---|---|---|
| `S16_Q15_SAT` | 16 | 15 | 16 位有符号，Q15，饱和 |
| `S32_Q23_SAT` | 32 | 23 | 32 位有符号，Q23，饱和 |
| `S32_Q15` | 32 | 15 | 32 位有符号，Q15，不饱和 |
| `S32_Q23` | 32 | 23 | 32 位有符号，Q23，不饱和 |

**评估标准**：
- ⭐⭐⭐ 能列全并解释饱和/非饱和；
- ⭐⭐ 能列 2-3 个；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 4.3 从 `loc_mic_pacc_cb_tbl` 推断链序（★★★★ / 源码追踪）

**题目**：从 `loc_mic_pacc_cb_tbl` 的 `fmt_in/fmt_out` 字段，如何推断出链序 EQ → FSH → DRC？请用 fmt 衔接关系做图示。

**考察点**：
- 主考点：CS 链表与格式衔接；
- 副考点：`loc_mic_pacc_cb_tbl` 的定义；
- 区分点：能否画出格式递进图。

**参考答案**：
`loc_mic_pacc_cb_tbl` 定义：
- EQ CS：`fmt_in=S16_Q15_SAT`，`fmt_out=S32_Q23`；
- FSH CS（可选）：`fmt_in=S32_Q23`，`fmt_out=S32_Q23`；
- DRC CS：`fmt_in=S32_Q23`，`fmt_out=S16_Q15_SAT`。

衔接关系：
```
S16_Q15 → [EQ] → S32_Q23 → [FSH] → S32_Q23 → [DRC] → S16_Q15
```
输入 16bit，EQ 内部用 32bit 提升精度，DRC 输出回 16bit。

**评估标准**：
- ⭐⭐⭐ 能画出完整递进；
- ⭐⭐ 能说出 EQ→DRC；
- ⭐ 完全不知道。

---

### 题目 4.4 `loc_mic_pacc_enable()` 的条件化链头格式（★★★★ / 源码追踪）

**题目**：`loc_mic_pacc_enable()` 中根据 `WIRELESS_MIC_SOFT_GAIN_EN` 决定链头 IO 格式。为什么要做这种条件化设计？如果 Soft Gain 关掉，链头格式应该是什么？

**考察点**：
- 主考点：Soft Gain 与 PACC 的耦合；
- 副考点：`loc_mic_pacc_enable()` 的实现；
- 区分点：能否说出 Soft Gain 把 16bit 转 32bit 后送入 PACC。

**参考答案**：
`loc_mic_pacc_enable()`（`mic_effect.c:139-162`）：
- `#if WIRELESS_MIC_SOFT_GAIN_EN`：`pacc_cs_list_set_io_fmt(..., S32_Q15, S16_Q15_SAT)`——因为 `soft_gain_proc_16bit()` 返回 32bit 缓存；
- `#else`：`pacc_cs_list_set_io_fmt(..., S16_Q15_SAT, S16_Q15_SAT)`——16bit 直接进 EQ。

Soft Gain 在 `loc_mic_pacc_process()` 内逐点调用，把 16bit 样本扩展为 32bit 后再给 PACC，因此链头输入格式必须是 `S32_Q15`。

**评估标准**：
- ⭐⭐⭐ 能指出耦合点与格式差异；
- ⭐⭐ 能说出条件化；
- ⭐ 完全不知道。

---

### 题目 4.5 MIX 链与 LOC 链的枚举顺序（★★★★ / 源码追踪）

**题目**：`mix_pacc_cb_tbl` 的枚举编号与 `cb_tbl` 数组物理顺序相反。请解释为什么 LOC 从枚举能推断链序，而 MIX 不能。

**考察点**：
- 主考点：CS 链表建立的两种模式；
- 副考点：`pacc_effect_link()` 与 `pacc_effect_relink()` 的行为；
- 区分点：能否指出 LOC 用 `cb_tbl` 物理顺序，MIX 用 `cs_en` 重链。

**参考答案**：
- LOC：`loc_mic_pacc_cb_tbl` 数组顺序即链序，`pacc_effect_link(cb_tbl, LOC_MIC_PACC_MAX_CS, ...)` 直接按数组顺序建链；
- MIX：`mix_pacc_cb_tbl` 数组顺序与枚举相反（MIX_MIC_PACC_EQ_CS=0 但数组中 DRC 在前），真正链序由 `mix_pacc_enable()` 中 `pacc_effect_relink()` 按 `cs_en[]` 重排决定。

**评估标准**：
- ⭐⭐⭐ 能指出两种建链方式差异；
- ⭐⭐ 能说出"枚举与数组不一致"；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 4.6 `soft_gain_init` 地址为 0x00000000（★★★★★ / 调试排查）

**题目**：开启 `WIRELESS_MIC_EQ_DRC_EN=1` 后，从 `map.txt` 看到 `soft_gain_init` 的 section 地址是 0x00000000。这意味着什么？为什么按键音量调节仍然"有效"？

**考察点**：
- 主考点：链接脚本与函数地址分析；
- 副考点：Soft Gain 的双重门控（`WIRELESS_MIC_SOFT_GAIN_EN` 与 `WIRELESS_MIC_EQ_DRC_EN`）；
- 区分点：能否指出"未链接但逻辑上被调用"的坑。

**参考答案**：
地址 0x00000000 通常表示该符号被链接器丢弃或未定义。原因可能是 `soft_gain.c` 中 `#if WIRELESS_MIC_SOFT_GAIN_EN` 与 `#if WIRELESS_MIC_EQ_DRC_EN` 双重门控：若 `WIRELESS_MIC_EQ_DRC_EN=0`，整个 `soft_gain.c` 不编译；若 `loc_mic_pacc_process()` 未调用 `soft_gain_proc_16bit()`，链接器可能把 `soft_gain_init` gc-section 掉。

按键音量调节仍"有效"是因为 `msg_mic_emit.c` 中 `MSG_VOL_UP/DOWN` 直接调 `soft_gain_up/down()`，这些函数可能在其他路径被链接，或 `msg_mic_emit.c` 的调用使 `soft_gain_up/down` 保留，而 `soft_gain_init` 因未被调用被丢弃。

**评估标准**：
- ⭐⭐⭐ 能解释现象并给出验证方法；
- ⭐⭐ 能说出"被 gc-section"；
- ⭐ 完全不知道。

---

### 题目 4.7 UART 调 EQ 在线生效失败（★★★★ / 调试排查）

**题目**：UART 调试 EQ 在线生效失败，但 `printf "loc_mic_pacc_init"` 已打印。请说出至少 3 个原因（含 `cs_en`、`eq_drc_en`、资源加载失败）。

**考察点**：
- 主考点：在线调参链路；
- 副考点：`loc_mic_pacc_set_param()` 与 `loc_mic_pacc_enable()` 的状态；
- 区分点：能否区分"初始化过了"与"参数没生效"。

**参考答案**：
1. `loc_mic_pacc_set_param()` 中 `pacc_eq_set_by_res()` / `pacc_drc_set_by_res()` 返回非 `ERROR_NO`，导致 `cs_en[EQ_CS]` 或 `cs_en[DRC_CS]` 未置 1；
2. `loc_mic_pacc_enable()` 后 `wireless_mic_eq_drc_cfg.eq_drc_en` 仍为 false，导致 `mic_eq_drc_audio_input()` 跳过处理；
3. effect.bin 资源损坏或 `effect_res_addr_get()` 返回 NULL；
4. ToolKit 下发的 EQ 参数 `enable=0` 或 `band_cnt=0`。

**评估标准**：
- ⭐⭐⭐① 条并给出验证方法；
- ⭐⭐ 3 条；
- ⭐ 只说一句。

---

## 4. 设计权衡题

### 题目 4.8 PACC0 无锁共享的风险与改造（★★★★★ / 设计权衡）

**题目**：PACC0 硬件加速器按位图（`PACC0_LOC_MIC` / `PACC0_MIX_MIC`）分时复用，没有用信号量保护。请分析这套"无锁共享"的安全前提，如果未来要并发使用，你会怎么改造？

**考察点**：
- 主考点：硬件资源并发安全；
- 副考点：当前设计的安全前提（TX/RX 不同时运行或严格串行）；
- 区分点：能否提出锁/队列/优先级方案。

**参考答案**：
当前安全前提：TX 与 RX 不同时运行（同一固件只能是一个角色），或应用层保证 `pacc_ctl_proc()` 不会并发调用。`mic_effect.c:1-13` 注释明确"避免多线程同时调用 pacc_ctl_proc_cs"。

若未来并发，改造方案：
1. 加互斥锁（mutex）保护 `pacc_ctl_proc()`；
2. 引入 PACC job 队列，按优先级调度；
3. 用 `pacc_ctl_proc_kick()` + `pacc_ctl_proc_wait()` 实现异步完成通知，避免忙等。

**评估标准**：
- ⭐⭐⭐ 能给出安全前提与改造方案；
- ⭐⭐ 能给出一种方案；
- ⭐ 完全不知道。

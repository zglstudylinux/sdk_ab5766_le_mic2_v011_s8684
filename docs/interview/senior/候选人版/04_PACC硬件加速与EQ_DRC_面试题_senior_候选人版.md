# 04 PACC 硬件加速与 EQ/DRC 面试题（Senior · 候选人版）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 候选人版（先自行作答，无答案）

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，TX/RX 共用 PACC0 硬件加速器。
- 架构特征：PACC 用 CS（Control State）链表描述音频处理链，支持 EQ/DRC/MDRC/FSH/FIR，数据格式支持 S16_Q15/S32_Q15/S32_Q23 等。
- 本卷重点：CS 链表与 Q 格式递进、PACC0 按位图分时复用、Soft Gain 与 PACC 的双重门控、effect.bin 资源加载。

---

## 1. 概念理解题

### 题目 4.1 PACC 与 CS 链表模型（★★★★ / 概念理解）

**题目**：什么是 PACC？CS（Control State）链表模型与"函数调用栈"有什么本质区别？

**考察点**：
- 主考点：硬件加速器抽象模型；
- 副考点：CS 与函数调用的数据流差异；
- 区分点：能否说出 CS 是"数据流图节点"而非"函数调用"。

### 题目 4.2 PACC 数据格式（★★★ / 概念理解）

**题目**：列出 PACC EQ/DRC 支持的数据格式（`S16_Q15_SAT`、`S32_Q23_SAT`、`S32_Q15`、`S32_Q23`），并说明各自的位宽与小数位数。

**考察点**：
- 主考点：定点 Q 格式；
- 副考点：EQ/DRC 内部精度需求；
- 区分点：能否区分"饱和"与"非饱和"版本。

## 2. 源码追踪题

### 题目 4.3 从 `loc_mic_pacc_cb_tbl` 推断链序（★★★★ / 源码追踪）

**题目**：从 `loc_mic_pacc_cb_tbl` 的 `fmt_in/fmt_out` 字段，如何推断出链序 EQ → FSH → DRC？请用 fmt 衔接关系做图示。

**考察点**：
- 主考点：CS 链表与格式衔接；
- 副考点：`loc_mic_pacc_cb_tbl` 的定义；
- 区分点：能否画出格式递进图。

### 题目 4.4 `loc_mic_pacc_enable()` 的条件化链头格式（★★★★ / 源码追踪）

**题目**：`loc_mic_pacc_enable()` 中根据 `WIRELESS_MIC_SOFT_GAIN_EN` 决定链头 IO 格式。为什么要做这种条件化设计？如果 Soft Gain 关掉，链头格式应该是什么？

**考察点**：
- 主考点：Soft Gain 与 PACC 的耦合；
- 副考点：`loc_mic_pacc_enable()` 的实现；
- 区分点：能否说出 Soft Gain 把 16bit 转 32bit 后送入 PACC。

### 题目 4.5 MIX 链与 LOC 链的枚举顺序（★★★★ / 源码追踪）

**题目**：`mix_pacc_cb_tbl` 的枚举编号与 `cb_tbl` 数组物理顺序相反。请解释为什么 LOC 从枚举能推断链序，而 MIX 不能。

**考察点**：
- 主考点：CS 链表建立的两种模式；
- 副考点：`pacc_effect_link()` 与 `pacc_effect_relink()` 的行为；
- 区分点：能否指出 LOC 用 `cb_tbl` 物理顺序，MIX 用 `cs_en` 重链。

## 3. 调试排查题

### 题目 4.6 `soft_gain_init` 地址为 0x00000000（★★★★★ / 调试排查）

**题目**：开启 `WIRELESS_MIC_EQ_DRC_EN=1` 后，从 `map.txt` 看到 `soft_gain_init` 的 section 地址是 0x00000000。这意味着什么？为什么按键音量调节仍然"有效"？

**考察点**：
- 主考点：链接脚本与函数地址分析；
- 副考点：Soft Gain 的双重门控（`WIRELESS_MIC_SOFT_GAIN_EN` 与 `WIRELESS_MIC_EQ_DRC_EN`）；
- 区分点：能否指出"未链接但逻辑上被调用"的坑。

### 题目 4.7 UART 调 EQ 在线生效失败（★★★★ / 调试排查）

**题目**：UART 调试 EQ 在线生效失败，但 `printf "loc_mic_pacc_init"` 已打印。请说出至少 3 个原因（含 `cs_en`、`eq_drc_en`、资源加载失败）。

**考察点**：
- 主考点：在线调参链路；
- 副考点：`loc_mic_pacc_set_param()` 与 `loc_mic_pacc_enable()` 的状态；
- 区分点：能否区分"初始化过了"与"参数没生效"。

## 4. 设计权衡题

### 题目 4.8 PACC0 无锁共享的风险与改造（★★★★★ / 设计权衡）

**题目**：PACC0 硬件加速器按位图（`PACC0_LOC_MIC` / `PACC0_MIX_MIC`）分时复用，没有用信号量保护。请分析这套"无锁共享"的安全前提，如果未来要并发使用，你会怎么改造？

**考察点**：
- 主考点：硬件资源并发安全；
- 副考点：当前设计的安全前提（TX/RX 不同时运行或严格串行）；
- 区分点：能否提出锁/队列/优先级方案。

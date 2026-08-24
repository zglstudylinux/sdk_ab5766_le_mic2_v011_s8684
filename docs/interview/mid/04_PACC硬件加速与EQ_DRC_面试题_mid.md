# 04 PACC 硬件加速与 EQ/DRC 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：PACC 基本调用、EQ/DRC 接入、Soft Gain、资源加载

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风。
- 音频加速：PACC 硬件加速器通过 CS 链表处理 EQ/DRC。
- 本卷重点：PACC 调用流程、EQ/DRC 初始化、Soft Gain 调用点、`effect.bin` 资源加载。
- 核心源码入口：
  - `libs/cpu/api_pacc.h`
  - `modules/effect/mic_effect.c:82-187`
  - `modules/audio/mic_eq_drc.c:19-48`
  - `modules/audio/soft_gain.c:110-128`
  - `modules/effect/effect_table.c`

---

## 1. 概念理解题

### 题目 4.1 PACC 是什么（★★ / 概念理解）

**题目**：PACC 是什么？为什么不直接用 CPU 做 EQ/DRC？

**考察点**：
- 主考点：PACC 定位；
- 副考点：硬件加速 vs 软件计算；
- 区分点：能否说出“硬件加速、低延迟、低功耗”。

**参考答案**：
PACC 是芯片内置的音频算法硬件加速器，通过 CS 链表把 EQ/DRC 等算法交给硬件执行，减少 CPU 占用和功耗，同时保证 2.5ms 帧的实时性。软件做 EQ/DRC 会占用大量 CPU 周期。

**评估标准**：
- ⭐⭐⭐ 能说出三点；
- ⭐⭐ 说出两点；
- ⭐ 说不出。

---

### 题目 4.2 EQ/DRC 初始化顺序（★★ / 概念理解）

**题目**：`loc_mic_pacc_init()`、`loc_mic_pacc_set_param()`、`loc_mic_pacc_enable()` 的调用顺序是什么？

**考察点**：
- 主考点：PACC 初始化流程；
- 副考点：资源加载与使能分离；
- 区分点：能否说出“先建链、再载参、后使能”。

**参考答案**：
顺序：`loc_mic_pacc_init()` → `loc_mic_pacc_set_param()` → `loc_mic_pacc_enable()`。先建 CS 链表，再从 `effect.bin` 加载 EQ/DRC 参数，最后使能 PACC。

**评估标准**：
- ⭐⭐⭐ 顺序正确；
- ⭐⭐ 顺序正确但无解释；
- ⭐ 顺序错误。

---

## 2. 源码追踪题

### 题目 4.3 Soft Gain 在 TX 链中的位置（★★★ / 源码追踪）

**题目**：Soft Gain 在 TX 音频链中的准确位置是什么？请说出它在 `loc_mic_pacc_process()` 中的调用形式。

**考察点**：
- 主考点：Soft Gain 与 PACC 的耦合；
- 副考点：`loc_mic_pacc_process()` 内部逻辑；
- 区分点：能否说出“逐点调用先放到 32bit 缓存，再给 PACC”。

**参考答案**：
`loc_mic_pacc_process()`（`modules/effect/mic_effect.c:164-187`）中，若 `WIRELESS_MIC_SOFT_GAIN_EN`，则先把 16bit 输入逐点通过 `soft_gain_proc_16bit()` 扩展到 32bit 缓存，再把该缓存作为 PACC 输入。

**评估标准**：
- ⭐⭐⭐ 能说出逐点调用与位宽扩展；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

### 题目 4.4 `effect.bin` 参数如何到 PACC（★★★ / 源码追踪）

**题目**：`effect.bin` 中的 EQ 参数最终如何到达 PACC 硬件？请说出从 `effect_res_addr_get()` 到 `pacc_eq_set_by_res()` 的调用链。

**考察点**：
- 主考点：资源加载路径；
- 副考点：`loc_mic_pacc_set_param()`；
- 区分点：能否说出 `effect_info[]` 索引与 `effect_res_addr_get()` 的二次偏移。

**参考答案**：
1. `loc_mic_pacc_set_param()` 调 `effect_res_addr_get(EFFECT_IDX_LOC_MIC_EQ)` 与 `effect_res_len_get()`；
2. `effect_res_addr_get()` 通过 `effect_info[]` 中的偏移找到 `effect.bin` 中对应段的地址；
3. `pacc_eq_set_by_res()` 把该地址/长度传给 PACC，若返回 `ERROR_NO` 则 `cs_en[EQ_CS]=1`。

**评估标准**：
- ⭐⭐⭐ 完整链；
- ⭐⭐ 链大概；
- ⭐ 完全不知道。

---

## 3_ 调试排查题

### 题目 4.5 EQ 调了但听感无变化（★★★ / 调试排查）

**题目**：用 ToolKit 在线调 EQ，ACK 成功，但听感无变化。请给出 4 个可能原因。

**考察点**：
- 主考点：在线调参失效排查；
- 副考点：`cs_en`、`eq_drc_en`、资源、角色；
- 区分点：能否区分“参数没下发”与“参数下发了但没跑”。

**参考答案**：
1. `pacc_eq_set_by_res()` 失败，`cs_en[EQ_CS]` 未置 1；
2. `loc_mic_pacc_enable()` 后 `eq_drc_en` 仍为 false；
3. 当前角色是 Adapter，调的是 `mix_pacc` 不是 `loc_mic`；
4. ToolKit 下发的 EQ 参数 `enable=0` 或增益为 0dB。

**评估标准**：
- ⭐⭐三 4 条；
- ⭐⭐ 2-3 条；
- ⭐ 1 条。

---

## 4. 设计权衡题

### 题目 4.6 Soft Gain 用 32bit 还是 16bit（★★★ / 设计权衡）

**题目**：Soft Gain 当前实现把输入扩展到 32bit 再入 PACC。请分析为什么不用 16bit 直接入 PACC。

**考察点级**
- 主考点：位宽与精度；
- 副考点：PACC 输入格式；
- 区分点：能否说出“防溢出/保精度”。

**参考答案**：
直接 16bit 入 PACC 时，EQ/DRC 内部需要更高精度避免截断失真；Soft Gain 先扩展到 32bit 再入 PACC，可在增益调整时保留更多小数位，减少量化噪声和削波风险。

**评估标准**：
- ⭐⭐⭐ 能说出精度/溢出原因；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

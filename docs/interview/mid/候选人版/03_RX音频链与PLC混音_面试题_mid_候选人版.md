# 03 RX 音频链与 PLC 混音 面试题（Mid · 候选人版）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 候选人版（先自行作答，无答案）

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风 RX 端（Adapter）。
- 音频链：无线接收 → BFI/PLC → LC3S 解码 → frag0/frag1 拼接 → MIX_DRC(PACC) → DAC0/USB Mic。
- 本卷重点：mic_dec_prco_cb 调用链、btstack 与解码线程的异步关系、PLC 的“好坏帧都处理”、mix_pacc_process() 混音方式。

---

## 1. 概念理解题

### 题目 3.1 BFI 与 PLC 的处理时机（★★★ / 概念理解）

**题目**：`bfi`（Bad Frame Indicator）是怎么产生的？为什么好坏帧都要调 `plc_soft_process()`？

**考察点**：
- 主考点：PLC 工作原理；
- 副考点：`btstack`/无线层如何标记丢帧；
- 区分点：能否说明“好帧更新历史，坏帧做 concealment”。

### 题目 3.2 一拖二时 frag0/frag1 拼接（★★★ / 概念理解）

**题目**：`WIRELESS_CON_LINK_NB=2` 时，`frag0`/`frag1` 拼接的目的与工作机制是什么？与 `LINK_NB=1` 简化路径的差异？

**考察点**：
- 主考点：双路无线链的帧对齐；
- 副考点：`mic_dec.frag_samples` 与 `obuf_off`；
- 区分点：能否说明为什么需要两个半帧而不是直接混。

---

## 2. 源码追踪题

### 题目 3.3 BLE 收完一帧到 DAC 输出的完整调用链（★★★ / 源码追踪）

**题目**：BLE 收完一帧后，到 DAC 输出 PCM 的完整调用链。要求引用 `mic_proc.c`、`thread_dec.c`、`wireless_txrx_single.c` 三处。

**考察点**：
- 主考点：RX 音频链调用顺序；
- 副考点：`kick_dec_prio_trans()` 的异步机制；
- 区分点：能否按文件列出关键函数。

### 题目 3.4 PLC 参数分档（★★★ / 源码追踪）

**题目**：PLC 参数组按采样率分成两组（`>= SAMPLE_RATE_16K` 与 `<`）。当前 48k 走哪一组？这个分支的 `if` 条件容易让人误解什么？

**考察点**：
- 主考点：PLC 参数初始化；
- 副考点：采样率宏的定义；
- 区分点：能否指出“48k 走高采样率组”以及条件判断的写法。

### 题目 3.5 `kick_dec_prio_trans(idx)` 的异步解码（★★★ / 源码追踪）

**题目**：`kick_dec_prio_trans(idx)` 是如何避免在 BLE ISR 里跑 LC3S 解码的？请描述从 ISR → kick → thread 的完整异步路径。

**考察点**：
- 主考点：中断与线程的边界；
- 副考点：`os/msg_prio_trans` 机制；
- 区分点：能否说明为什么不能在 ISR 里解码。

---

## 3. 调试排查题

### 题目 3.6 RX 周期性咔嗒声（★★★ / 调试排查）

**题目**：RX 端音频有周期性咔嗒声，但 RSSI 良好、无双连。可能是 PLC 的 `last_bfi` 逻辑问题，请设计排查步骤（开 dump → 看 bfi 序列）。

**考察点**：
- 主考点：PLC 状态机调试；
- 副考点：BFI 序列观察方法；
- 区分点：能否给出可复现的 dump 方案。

### 题目 3.7 一拖二断开一路后的爆音（★★★ / 调试排查）

**题目**：一拖二场景下，断开一路后另一路出现短暂爆音。请从 `connected_sta`、`adapter_reset(mic_num, sta)` 部分复位、`mix_temp_buf` 残留三个角度分析。

**考察点**：
- 主考点：部分复位与混音缓冲；
- 副考点：`adapter_reset()` 在非全断开时的行为；
- 区分点：能否指出 `mix_temp_buf` 清零不及时。

---

## 4. 设计权衡题

### 题目 3.8 MIX_DRC 直接相加 vs /2（★★★ / 设计权衡）

**题目**：`MIX_DRC` 用 `ibuf0[i]+ibuf1[i]` 直接相加，而 `mic_proc.c` 的非 MIX_DRC 分支用 `/2`。两种混音方式哪个更优？为什么 RX 不用 Soft Gain？

**考察点**：
- 主考点：混音溢出保护；
- 副考点：DRC 与 Soft Gain 的职责；
- 区分点：能否说明直接相加依赖 DRC 压限，/2 简单但损失动态。

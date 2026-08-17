# 03 RX 音频链与 PLC 混音 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：mic_dec_prco_cb 每帧链、BFI/PLC、frag0/frag1 拼接、MIX_DRC、DAC/USB Mic 输出

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风 RX 端（Adapter）。
- 音频链：无线接收 → BFI/重传 → LC3S 解码 → PLC → frag0/frag1 拼接（一拖二时）→ MIX_DRC(PACC) → DAC0/USB Mic。
- 本卷重点：`mic_dec_prco_cb()` 调用链、`btstack` 与解码线程的异步关系、PLC 的“好坏帧都处理”、`mix_pacc_process()` 混音方式。
- 核心源码入口：
  - `modules/wireless/mic_proc.c:549-594`
  - `modules/wireless/wireless_txrx_single.c:274-281`
  - `os/thread_dec.c:4-20`
  - `modules/voice/plc_soft.c:18-64`
  - `modules/effect/mic_effect.c:294-313`
  - `functions/func_adapter.c:33-54`

---

## 1. 概念理解题

### 题目 3.1 BFI 与 PLC 的处理时机（★★★ / 概念理解）

**题目**：`bfi`（Bad Frame Indicator）是怎么产生的？为什么好坏帧都要调 `plc_soft_process()`？

**考察点**：
- 主考点：PLC 工作原理；
- 副考点：`btstack`/无线层如何标记丢帧；
- 区分点：能否说明“好帧更新历史，坏帧做 concealment”。

**参考答案**：
BFI 由无线接收层产生：当 CRC 错误、超时或明确收到坏帧指示时，`wireless_txrx_single.c` 中的接收逻辑把该标记传到解码线程。

`plc_soft_process()` 对每帧都调用：
- 好帧：更新 PLC 的历史缓冲区，为将来丢帧做准备；
- 坏帧：基于历史做 concealment，生成替代 PCM 保持听感连续。

**评估标准**：
- ⭐⭐⭐ 能区分好/坏帧的作用；
- ⭐⭐ 只说“丢帧补偿”；
- ⭐ 完全不知道。

---

### 题目 3.2 一拖二时 frag0/frag1 拼接（★★★ / 概念理解）

**题目**：`WIRELESS_CON_LINK_NB=2` 时，`frag0`/`frag1` 拼接的目的与工作机制是什么？与 `LINK_NB=1` 简化路径的差异？

**考察点**：
- 主考点：双路无线链的帧对齐；
- 副考点：`mic_dec.frag_samples` 与 `obuf_off`；
- 区分点：能否说明为什么需要两个半帧而不是直接混。

**参考答案**：
两路 TX 的无线帧到达时间可能错开半个帧周期。`frag0`/`frag1` 把每路 120 点拆成 60+60，在 `mic_dec_pcm_out_frag0()` 和 `frag1()` 中按到达顺序拼接成完整 240 点后再做 MIX_DRC。单路时无需拼接，直接输出。

**评估标准**：
- ⭐⭐⭐ 能说明“错开半帧”与拼接目的；
- ⭐⭐ 只说“混音”；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 3.3 BLE 收完一帧到 DAC 输出的完整调用链（★★★ / 源码追踪）

**题目**：BLE 收完一帧后，到 DAC 输出 PCM 的完整调用链。要求引用 `mic_proc.c`、`thread_dec.c`、`wireless_txrx_single.c` 三处。

**考察点**：
- 主考点：RX 音频链调用顺序；
- 副考点：`kick_dec_prio_trans()` 的异步机制；
- 区分点：能否按文件列出关键函数。

**参考答案**：
1. `wireless_txrx_single.c:274-281` `wireless_d2a_set_rxdec_cb()` → `rxpkt_done()` → `kick_dec_prio_trans(idx)`；
2. `thread_dec.c:4-20` `thread_dec_proc_msg_cb()` 收到 `MSG_MIC_DEC0/1` → `mic_dec_prco_cb(idx)`；
3. `modules/wireless/mic_proc.c:549-594` `mic_dec_prco_cb()`：
   - `wireless_d2a_get_rx_frame()` 取帧与 BFI；
   - `lc3s_dec()` 解码；
   - `plc_soft_process()`；
   - `mic_dec_pcm_out()`（或 frag0/frag1）；
   - `mix_drc_audio_input()`（一拖二时）；
   - `dac0_out_audio_input()` 或 `usb_mic_in_audio_input()`。

**评估标准**：
- ⭐⭐⭐ 完整链 + 三处文件；
- ⭐⭐ 链正确但缺文件；
- ⭐ 完全不知道。

---

### 题目 3.4 PLC 参数分档（★★★ / 源码追踪）

**题目**：PLC 参数组按采样率分成两组（`>= SAMPLE_RATE_16K` 与 `<`）。当前 48k 走哪一组？这个分支的 `if` 条件容易让人误解什么？

**考察点**：
- 主考点：PLC 参数初始化；
- 副考点：采样率宏的定义；
- 区分点：能否指出“48k 走高采样率组”以及条件判断的写法。

**参考答案**：
`plc_soft_init()` 中按 `sample_rate >= SAMPLE_RATE_16K` 选择参数组；48k 满足条件，走高采样率参数。容易误解的是：`SAMPLE_RATE_16K` 是枚举/宏而不是 16000，需确认其真实数值。

**评估标准**：
- ⭐⭐⭐ 能指出分支与宏的潜在歧义；
- ⭐⭐ 能说出分组；
- ⭐ 完全不知道。

---

### 题目 3.5 `kick_dec_prio_trans(idx)` 的异步解码（★★★ / 源码追踪）

**题目**：`kick_dec_prio_trans(idx)` 是如何避免在 BLE ISR 里跑 LC3S 解码的？请描述从 ISR → kick → thread 的完整异步路径。

**考察点**：
- 主考点：中断与线程的边界；
- 副考点：`os/msg_prio_trans` 机制；
- 区分点：能否说明为什么不能在 ISR 里解码。

**参考答案**：
BLE ISR 里只做帧标记与队列操作，调用 `kick_dec_prio_trans(idx)` 把一个解码消息投到 `thread_dec` 线程。该线程被唤醒后调 `mic_dec_prco_cb()` 执行 `lc3s_dec()`。这样 ISR 快速退出，解码在较宽松的线程上下文完成，避免阻塞 BLE 时序。

**评估标准**：
- ⭐⭐⭐ 能说出异步路径与原因；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 3.6 RX 周期性咔嗒声（★★★ / 调试排查）

**题目**：RX 端音频有周期性咔嗒声，但 RSSI 良好、无双连。可能是 PLC 的 `last_bfi` 逻辑问题，请设计排查步骤（开 dump → 看 bfi 序列）。

**考察点**：
- 主考点：PLC 状态机调试；
- 副考点：BFI 序列观察方法；
- 区分点：能否给出可复现的 dump 方案。

**参考答案**：
1. 打开 `WIRELESS_DUMP_EN` 或自定义 dump，记录每帧 `bfi` 与 `last_bfi`；
2. 在 `mic_dec_prco_cb()` 中打印 `bfi` 序列；
3. 观察咔嗒声是否对应连续 BFI 或 `last_bfi` 未更新；
4. 检查 `plc_soft_process()` 的输入参数 `mic_dec.last_bfi[idx] || bfi` 是否正确反映连续丢帧。

**评估标准**：
- ⭐⭐⭐ 步骤可操作；
- ⭐⭐ 步骤基本正确；
- ⭐ 只说“看 RSSI”。

---

### 题目 3.7 一拖二断开一路后的爆音（★★★ / 调试排查）

**题目**：一拖二场景下，断开一路后另一路出现短暂爆音。请从 `connected_sta`、`adapter_reset(mic_num, sta)` 部分复位、`mix_temp_buf` 残留三个角度分析。

**考察点**：
- 主考点：部分复位与混音缓冲；
- 副考点：`adapter_reset()` 在非全断开时的行为；
- 区分点：能否指出 `mix_temp_buf` 清零不及时。

**参考答案**：
断开一路时 `connected_sta` 仍含另一路，`adapter_reset(mic_num, connected_sta)` 只复位该路的 PLC/codec/命令，不会退出 DAC0。若 `mix_temp_buf` 中仍残留断开路的 PCM，混音时会把旧数据与新数据叠加，产生爆音。修复：部分复位后清零 `mix_temp_buf` 或跳过该路混音。

**评估标准**：
- ⭐⭐⭐ 三角度都能分析；
- ⭐⭐ 两角度；
- ⭐ 只说一句。

---

## 4. 设计权衡题

### 题目 3.8 MIX_DRC 直接相加 vs /2（★★★ / 设计权衡）

**题目**：`MIX_DRC` 用 `ibuf0[i]+ibuf1[i]` 直接相加，而 `mic_proc.c` 的非 MIX_DRC 分支用 `/2`。两种混音方式哪个更优？为什么 RX 不用 Soft Gain？

**考察点**：
- 主考点：混音溢出保护；
- 副考点：DRC 与 Soft Gain 的职责；
- 区分点：能否说明直接相加依赖 DRC 压限，/2 简单但损失动态。

**参考答案**：
直接相加可能溢出，但后接 MIX_DRC 会做压缩/限幅，保留更多动态；`/2` 安全但直接损失 6dB。RX 不用 Soft Gain 是因为混音后的动态控制由 DRC 负责，Soft Gain 更适合 TX 单链路的输入增益调整。

**评估标准**：
- ⭐⭐⭐ 能对比并解释职责划分；
- ⭐⭐ 能说出一种；
- ⭐ 完全不知道。

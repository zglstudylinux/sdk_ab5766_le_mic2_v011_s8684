# 03 RX 音频链与 PLC 混音 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 音频算法方向
> 核心考察：实时解码、PLC 可靠性、混音算法复杂度优化、异步线程调度

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风 RX 端（Adapter）。
- 音频链：无线接收 → BFI/PLC → LC3S 解码 → frag0/frag1 拼接 → MIX_DRC(PACC) → DAC0/USB Mic。
- 本卷重点：解码线程调度、PLC 在差网环境的容错、混音的算力分配。
- 核心源码入口：
  - `modules/wireless/mic_proc.c:549-594,736-750`
  - `modules/wireless/wireless_txrx_single.c:274-281`
  - `modules/voice/plc_soft.c:55-64`
  - `modules/effect/mic_effect.c:294-313`
  - `os/thread_dec.c:4-20`

---

## 1. 概念理解题

### 题目 3.1 PLC 的“过度补偿”风险（★★★★ / 概念理解）

**题目**：当 BFI 连续为 1 时，`plc_soft_process()` 会基于历史数据 concealment。若 BFI 持续 50ms，PLC 会产生什么听感问题？如何缓解？

**考察点**：
- 主考点：PLC 的局限性；
- 副考点：历史数据有效性;
- 区分点：能否说出“声音变机器人音/重复音”。

**参考答案**：
长时间 concealment 会导致：
1. **机器人音**：历史数据反复插值导致音调畸变；
2. **重复音**：短时周期信号被重复播放；
3. **能量衰减**：PLC 无法恢复真实语音能量。

缓解方法：限制 PLC concealement 时长（如最多 20ms），超时后输出静音/舒适噪声；在 `plc_soft_process()` 中加入连续 BFI 计数。

**评估标准**：
- ⭐⭐⭐ 能说出两种听感问题与缓解；
- ⭐⭐ 能说出问题；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 3.2 `adapter_reset()` 的“单路复位”边界（★★★★★ / 源码追踪）

**题目**：`adapter_reset(idx, sta)` 中，当 `sta != 0`（还有另一路在线）时，哪些资源必须复位、哪些不能复位？请引用 `modules/wireless/mic_proc.c:736-750` 的具体行号。

**考察点**：
- 主考点：部分复位边界；
- 副考点：PACC/DAC 的共享状态；
- 区分点：能否精确说出“复位本路 buffer/codec/PLC，不复位 DAC0/PACC”。

**参考答案**：
`adapter_reset()`（[mic_proc.c:736-750](modules/wireless/mic_proc.c#L736-L750)）：
- **复位**：`last_bfi[idx]`、`plc_soft_exit(idx)`、`m_lc3s_dec_exit(idx)`、`wireless_cmd_reset(idx)`、PCM buffer `memset`（对该 idx）；
- **不复位**：`connected_sta` 其他位、DAC0（因为另一路还在用）、PACC（PACC 是共享的，另一路可能还在用）。

若错误复位 DAC0，会导致正在播放的另一路爆音。

**评估标准**：
- ⭐⭐⭐ 能精确分资源和行号；
- ⭐⭐ 能分资源；
- ⭐ 只说“复位该路”。

---

### 题目 3.3 `mix_pacc_process()` 的算力分配优化（★★★★★ / 源码追踪）

**题目**：`mix_pacc_process()` 中 `for(uint i=0; i<size; i++) { mix_temp_buf[i] = ibuf0[i] + ibuf1[i]; }` 是 CPU 算。请估算在 160MHz、2 路 48K 混音时的 CPU 占用率，并提出两种优化方法。

**考察点**：
- 主考点：混音的算力消耗；
- 副考点：加法的指令周期;
- 区分点：能否估算“120 cycles ≈ 0.75μs，占比 0.03%” 或提出优化。

**参考答案**：
混音是 `size=120` 次循环，每次 1 个 32 位加法 + 1 个索引 + 1 个存储，约 3 指令，总计 360 cycles。160MHz 下约 2.25μs，在 2.5ms 帧中占比约 0.09%，可忽略。

优化方法：
1. 若 PACC 有 MIX 模块，直接让 PACC 混音；
2. 用 DSP 指令或 SIMD（若 RISC-V 有扩展）批量加法；
3. 若 MIX_DRC 关闭，该循环可直接用 `memcpy` 优化（若只复制一路）。

**评估标准**：
- ⭐⭐⭐ 能估算并提出优化；
- ⭐⭐ 能估算；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 3.4 USB Mic 场景下的音频抖动（★★★★★ / 调试排查）

**题目**：在 USB Mic 模式（`ADAPTER_USB_MIC_RX_EN=1`）下，电脑录到的声音有周期性抖动，但 DAC 听音正常。请给出 4 个可能原因。

**考察点**：
- 主考点：USB 上行与 DAC 的差异；
- 副考点：USB buffer 与 DAC 的时钟差异；
- 区分点：能否区分“USB 采样率不同步”与“解码线程被抢占”。

**参考答案**：
1. **USB 与 DAC 采样率不同步**：USB Host 按自己的 48K 采样，但 RX 的 `mic_dec` 基于 BLE 时钟，`usb_mic_in_audio_input()` 发送时若未做 SRC 或缓冲，会产生抖动；
2. **USB buffer 满/空**：`usb_mic_in_audio_input()` 在 `obuf_off == WIRELESS_MIC_SAMPLES_SELECT` 时才发送，若 BLE 丢帧导致 `obuf_off` 不足，USB 会等数据，产生 gap；
3. **解码线程被 USB 中断抢占**：USB 高优先级中断打断 `thread_dec`，导致 PCM 包未能及时送入 USB；
4. **MIX_DRC 未启用**：双路 PCM 未对齐，frag0/frag1 拼接时机错乱，导致 USB 收到非对齐帧。

**评估标准**：
- ⭐⭐⭐ 4 条；
- ⭐⭐ 3 条；
- ⭐ 2 条以下。

---

## 4. 设计权衡题

### 题目 3.5 双路解码是否该用两个线程（★★★★ / 设计权衡）

**题目**：当前 `thread_dec_proc_msg_cb()` 单线程依次处理 `MSG_MIC_DEC0` 和 `MSG_MIC_DEC1`。若双路解码改用两个独立线程，请从实时性、资源占用、可维护性三个角度评估。

**考察点**：
- 主考点：并发调度设计;
- 副考点：PACC 共享风险；
- 区分点：能否指出当前单线程是为避免 PACC 共享冲突。

**参考答案**：
- **实时性**：两线程可并行解码，路 0 不阻塞路 1；但 PACC 是共享资源（见 `mic_effect.c:1-13`），两线程同时跑 MIX_DRC 会冲突；
- **资源占用**：多一个线程 = 多一份 stack/heap + 调度复杂度；
- **可维护性**：单线程顺序执行简单可靠；多线程需加锁，调试难。

当前设计是合理选择：解码耗时短（LC3S 单路 < 1ms），单线程足以满足 2.5ms 帧期；PACC 共享冲突是更大风险。

**评估标准**：
- ⭐⭐⭐ 能三角度并指出 PACC 冲突关键；
- ⭐⭐ 两角度；
- ⭐∞ 一角度。

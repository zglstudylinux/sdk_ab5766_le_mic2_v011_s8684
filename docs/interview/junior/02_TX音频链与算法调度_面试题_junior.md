# 02 TX 音频链与算法调度 面试题（Junior）

> 难度：★ / ★★
> 岗位：初级嵌入式工程师
> 核心考察：TX 音频链基本顺序、SDADC、LC3S 编码

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风 TX 端。
- 音频链：SDADC → AINS4_32K → SRC → EQ/DRC → LC3S 编码 → 无线发送。
- 本卷重点：TX 音频链基本顺序、`mic_enc_proc_cb()` 的作用。
- 核心源码入口：
  - `modules/wireless/mic_proc.c:277-378`
  - `bsp/bsp_sdadc.c:26-53`

---

## 1. 概念理解题

### 题目 2.1 TX 音频链包含哪几步（★★ / 概念理解）

**题目**：TX 音频链从麦克风采集到无线发送，包含哪几个主要步骤？

**考察点**：
- 主考点：TX 链基本组成；
- 副考点：SDADC、算法、编码、无线；
- 区分点：能否列出 4 步。

**参考答案**：
SDADC 采集 → AINS4_32K 降噪 → SRC 32k→48k → EQ/DRC（含 Soft Gain）→ LC3S 编码 → 无线发送。

**评估标准**：
- ⭐⭐⭐ 完整列出；
- ⭐⭐ 列出 4 步；
- ⭐ 说不出。

---

### 题目 2.2 LC3S 编码在哪调（★★ / 概念理解）

**题目**：LC3S 编码是在中断里做吗？如果不是，在哪里调？

**考察点**：
- 主考点：编码调用上下文；
- 副考点：`mic_enc_proc_cb()`；
- 区分点：能否说出在 AUPCM 线程或回调中。

**参考答案**：
LC3S 编码不在 ISR 里做，而是在 `mic_enc_proc_cb()` 中调（`modules/wireless/mic_proc.c:358-359`）。该函数由 SDADC DMA 完成后的 AUPCM 线程/回调触发。

**评估标准**：
- ⭐⭐⭐ 能说出上下文与函数名；
- ⭐⭐ 能说出“不在 ISR”；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 2.3 `mic_enc_proc_cb()` 里最后两步（★★ / 源码追踪）

**题目**：`mic_enc_proc_cb()` 中，LC3S 编码之后还要做哪两步？

**考察点**：
- 主考点：编码后的无线入队与同步；
- 副考点：`wireless_d2a_put_tx_frame()` 与 `wl_play_sync_tick1()`；
- 区分点：能否说出两步。

**参考答案**：
1. `wireless_d2a_put_tx_frame(mic_enc.buffer, WIRELESS_MIC_FRAME_SIZE)`；
2. `wl_play_sync_tick1(...)` 做无线播放同步。

**评估标准**：
- ⭐⭐⭐ 说出两步；
- ⭐⭐ 说出一步；
- ⭐ 完全不知道。

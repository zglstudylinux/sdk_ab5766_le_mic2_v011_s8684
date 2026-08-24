# 07 调试排查与设计权衡 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：启动时序、角色错判、音频链分段排查、基础权衡

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风。
- 调试工具：UART 日志、ToolKit、LED、示波器。
- 本卷重点：启动卡死、角色错判、TX/RX 无声的分层排查。
- 核心源码入口：
  - `bsp/bsp_sys.c:199-267`
  - `modules/wireless/wireless_proc.c:5-18,90-295`
  - `modules/wireless/mic_proc.c:277-378,549-594`
  - `functions/msg_mic_emit.c`

---

## 1. 概念理解题

### 题目 7.1 连接成功的日志关键行（★★★ / 概念理解）

**题目**：请说出 TX/RX 连接成功时 UART 日志中必须出现的 3 行关键打印。

**考察点**：
- 主考点：连接生命周期日志；
- 副考点：`wireless_emit_notice()` 中的打印；
- 区分点：能否说出“抬频/初始化/连接标记”三步对应的日志。

**参考答案**：
1. `sys_clk_req INDEX_DECODE SYS_160M`（或类似抬频日志）；
2. `mic_emit_init()` 或 `adapter_init()` 的打印；
3. `connected_sta` 被置位的打印或 dump。

**评估标准**：
- ⭐⭐⭐ 能说出 3 行；
- ⭐⭐ 2 行；
- ⭐ 1 行。

---

## 2. 源码追踪题

### 题目 7.2 角色错判的三层定位（★★★ / 源码追踪）

**题目**：发现 TX 板被识别成 RX。请从 `config`、`xcfg.bin`、内存三个层定位根因。

**考察点**：
- 主考点：配置加载链；
- 副考点：`wireless_mic_role_init()`；
- 区分点：能否说明“编译宏/参数区/运行时内存”三层。

**参考答案**：
1. `config` 层：确认 `Output/bin/Settings/` 中用的是 `wireless_mic_emit.setting` 而非 `wireless_adapter.setting`；
2. `xcfg.bin` 层：用工具查看 `wireless_adapter_en` bit 是否为 0，`wireless_mic_emit_en` bit 是否为 1；
3. 内存层：调试器或 printf 看 `xcfg_cb.wireless_adapter_en`、`xcfg_cb.wireless_mic_emit_en`；
4. `wireless_mic_role_init()`（`modules/wireless/wireless_proc.c:5-18`）加打印看走了哪个分支。

**评估标准**：
- ⭐⭐⭐ 三层完整；
- ⭐⭐ 两层；
- ⭐ 一层。

---

## 3. 调试排查题

### 题目 7.3 TX 已连接但 RX 无声（★★★ / 调试排查）

**题目**：TX 已连接（日志显示 connected），但 RX 耳机无声。给出 5 步排查流程。

**考察点**：
- 主考点：跨板音频链排查；
- 副考点：TX/RX 两侧分层；
- 区分点：能否按 TX→空口→RX 路径逐段排查。

**参考答案**：
1. TX：确认 `sys_cb.mic_alg_en=1`；`mic_enc_proc_cb()` 有打印；SDADC 有数据；
2. TX→空口：`lc3s_enc()` 输出非 0；`wireless_d2a_put_tx_frame()` 被调；
3. RX：`mic_dec_prco_cb()` 被调；`bfi` 不全为 1；
4. RX：`lc3s_dec()` 输出非 0；`plc_soft_process()` 后 PCM 正常；
5. RX：`dac0_out_audio_input()` 被调；DAC0 硬件正常（示波器看输出）。

**评估标准**：
- ⭐⭐⭐ 5 步；
- ⭐⭐ 3-4 步；
- ⭐ 1-2 步。

---

### 题目 7.4 按键软硬区分（★★★ / 调试排查）

**题目**：按 K2 切换 MAGIC 无效。如何区分是按键硬件问题、消息分派问题、还是算法未启用问题？

**考察点**：
- 主考点：按键链分段排查；
- 副考点：`WIRELESS_MIC_MAGIC_EN` 宏与运行时 `magic_cfg.mute`；
- 区分点：能否给出三段排查。

**参考答案**：
1. **硬件/扫描**：用示波器或打印看 `bsp_key_scan()` 是否检测到按键事件；
2. **消息分派**：在 `func_mic_emit_message()` 加打印，看 `MSG_CHANGE_MAGIC` 是否到达；
3. **算法**：确认 `WIRELESS_MIC_MAGIC_EN=1`（编译期），且 `magic_cfg.mute=0`（运行时）；必要时重新编译。

**评估标准**：
- ⭐⭐⭐ 三段清晰；
- ⭐⭐ 两段；
- ⭐ 一段。

---

## 4. 设计权衡题

### 题目 7.5 同步算法 vs 异步算法（★★★ / 设计权衡）

**题目**：TX 中 AINS4_32K 是异步（算法线程），Soft Gain 是同步（每帧内调）。为什么 Soft Gain 不放异步？

**考察点**：
- 主考点：算法放置位置；
- 副考点：计算量与延迟；
- 区分点：能否说出 Soft Gain 计算量小、放异步得不偿失。

**参考答案**：
Soft Gain 只是逐点乘加（`__builtin_muls_shift15`），120 点耗时极短，同步调几乎不占帧预算；放异步反而引入攒帧延迟和线程切换开销。AINS4_32K 是 DNN，计算量大，必须放异步避免阻塞音频。

**评估标准**：
- ⭐⭐⭐ 能量化对比；
- ⭐⭐ 能定性；
- ⭐ 完全不知道。

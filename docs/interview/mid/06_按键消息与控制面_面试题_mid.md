# 06 按键消息与控制面 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：按键表设计、消息队列、双板同步命令、无线 TX/RX 参数同步

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，当前板卡三个按键（PP/K1/K2）。
- 按键链：物理按键 → `bsp_key_scan()` → 键值表 → 消息队列 → `func_mic_emit_message()` → 具体模块 + `wireless_tx_cmd_fw()` 同步 RX。
- 本卷重点：`mic_emit_key_msg_tbl` 结构、`wireless_tx_cmd_fw()` 同步命令格式、MSG 与 CMD 的对应关系。
- 核心源码入口：
  - `projects/microphone/port/port_key.c:50-58`
  - `bsp/bsp_key.c:70-135`
  - `functions/msg_mic_emit.c`
  - `modules/wireless/wireless_cmd_api.c/h`

---

## 1. 概念理解题

### 题目 6.1 为什么 MSG 和 CMD 分开（★★ / 概念理解）

**题目**：按键消息 `MSG_VOL_UP` 与无线同步命令 `wireless_tx_cmd_fw()` 中的 CMD 有什么区别？为什么不直接让 RX 监听 TX 的 `MSG_VOL_UP`？

**考察点**：
- 主考点：本地消息 vs 无线命令；
- 副考点：协议分层；
- 区分点：能否说出“MSG 是本地事件，CMD 是空口包”。

**参考答案**：
`MSG_*` 是本地消息队列事件，只在当前板卡内传递；`wireless_tx_cmd_fw()` 发出的 CMD 是空口数据包，通过无线链路发给 RX。RX 无法直接访问 TX 的本地消息队列，必须用 CMD 同步关键参数（音量、mute、MAGIC 等）。

**评估标准**：
- ⭐⭐⭐ 能说出分层；
- ⭐⭐ 能说出区别；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 6.2 MSG_VOL_MUTE 的同步命令（★★★ / 源码追踪）

**题目**：`MSG_VOL_MUTE` 触发后，TX 发送给 RX 的同步命令包含哪些字段？请说出 `wireless_tx_cmd_fw()` 的调用参数。

**考察点**：
- 主考点：无线命令格式；
- 副考点：`wireless_tx_cmd_fw()` 的 API；
- 区分点：能否说出命令 ID + 参数。

**参考答案**：
`functions/msg_mic_emit.c` 中 `MSG_VOL_MUTE`：
1. 翻转 `mic_enc_mute_en` 并 `mic_enc_mute_en_set()`；
2. 调 `wireless_tx_cmd_fw(WIRELESS_CMD_MIC_MUTE, &mute_level, 1)`（或类似 CMD ID）把当前 mute 状态发给 RX。

**评估标准**：
- ⭐⭐⭐ 能说出命令 ID 与参数；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 6.3 音量按键 RX 端不同步（★★★ / 调试排查）

**题目**：TX 按 K1/K2 调音量，本地听感正常，但 RX 端显示/记录的音量不同步。请给出 4 个可能原因。

**考察点**：
- 主考点：同步命令丢失排查；
- 副考点：连接状态、命令发送、命令解析、显示逻辑；
- 区分点：能否给出 TX→空口→RX 的排查路径。

**参考答案**：
1. TX 未连接（`wireless_mic.connected_sta == 0`），`wireless_tx_cmd_fw()` 未发送；
2. `wireless_tx_cmd_fw()` 发送失败（无线 busy、队列满）；
3. RX 侧 `wireless_cmd_proc()` 未收到或未解析该 CMD；
4. RX 端显示/记录逻辑未更新（如 `sys_cb` 里没有对应字段或未刷新 LED）。

**评估标准**：
- ⭐⭐⭐ 4 条；
- ⭐⭐ 2-3 条；
- ⭐ 1 条。

---

## 4. 设计权衡题

### 题目 6.4 为什么不把所有本地状态都同步（★★★ / 设计权衡）

**题目**：当前只同步音量/mute/MAGIC/ECHO 等少数参数到 RX。为什么不把 SDADC 原始数据、EQ 系数、PACC 状态都同步过去？

**考察点**：
- 主考点：无线带宽 vs 功能需求；
- 副考点：隐私/功耗；
- 区分点：能否说出“按需同步”。

**参考答案**：
无线带宽有限且需优先传音频；同步所有状态会占用宝贵空口资源、增加功耗、且大部分参数 RX 不需要（如原始 SDADC 数据、PACC 内部系数）。按需同步只保留用户可见/需要联动的参数，是最优折中。

**评估标准**：
- ⭐⭐⭐ 能从带宽/功耗/隐私分析；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

# 06 按键消息与控制面 面试题（Junior）

> 难度：★ / ★★
> 岗位：初级嵌入式工程师
> 核心考察：按键表、消息队列、MAGIC 入口、双板同步命令

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，当前板卡只有 `KEY_ID_PP`、`KEY_ID_K1`、`KEY_ID_K2` 三个有效按键。
- 按键链：物理按键 → `bsp_key_scan()` → 键值表 `mic_emit_key_msg_tbl` → 消息队列 → `func_mic_emit_message()` → 具体模块（mute、soft_gain、magic 等）。
- 本卷重点：按键事件到消息的映射、MAGIC 的“长按抬起”触发、TX/RX 参数同步命令。
- 核心源码入口：
  - `projects/microphone/port/port_key.c:50-58`
  - `bsp/bsp_key.c:70-135`
  - `functions/msg_mic_emit.c`

---

## 1. 概念理解题

### 题目 6.1 按键表第一维/第二维（★★ / 概念理解）

**题目**：当前板卡只有三个键，而 `mic_emit_key_msg_tbl` 是 `KEY_TBL_MAX_NB × KEY_MSG_MAX_IDX` 的二维表。第一维/第二维各表示什么？

**考察点**：
- 主考点：按键表结构；
- 副考点：按键类型与事件类型；
- 区分点：能否说明第一维是“哪个键”，第二维是“什么事件”。

**参考答案**：
- 第一维（行）：按键 ID，如 `KEY_ID_PP`、`KEY_ID_K1`、`KEY_ID_K2`、`KEY_ID_K3`；
- 第二维（列）：事件类型，如 `KEY_SHORT_UP`、`KEY_LONG`、`KEY_HOLD`、`KEY_LONG_UP`、`KEY_DOUBLE` 等，对应 `evt_type_2_msg_idx()` 的 0~7。

**评估标准**：
- ⭐⭐⭐ 能准确说出两维含义；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

### 题目 6.2 `MSG_VOL_UP` 的两件事（★★ / 概念理解）

**题目**：`MSG_VOL_UP` 触发后，TX 做了哪两件事？为什么要做第二件事（同步给 RX）？

**考察点**：
- 主考点：TX 按键消息处理；
- 副考点：`wireless_tx_cmd_fw()` 同步命令；
- 区分点：能否说出“本地调音量 + 通知 RX”。

**参考答案**：
`functions/msg_mic_emit.c` 中 `MSG_VOL_UP`：
1. 调 `soft_gain_up()` 提升本地 Soft Gain；
2. 若已连接，调 `wireless_tx_cmd_fw(...)` 把当前音量等级同步给 RX（Adapter 显示/记录）。

第二件事是为了 RX 端能显示或记录当前 TX 音量，便于用户看到对端状态。

**评估标准**：
- ⭐⭐⭐ 能说出两件事及原因；
- ⭐⭐ 能说出两件事；
- ⭐ 只说出本地调节。

---

## 2. 源码追踪题

### 题目 6.3 K2 长按抬起触發 MAGIC（★★ / 源码追踪）

**题目**：从按下 K2 键“长按抬起”，到听感切换到下一档 MAGIC 的完整调用链。要求覆盖 `port_key.c`、`bsp_key.c`、`msg_mic_emit.c` 三个文件。

**考察点**：
- 主考点：按键到算法的完整链；
- 副考点：`KEY_LONG_UP` 事件；
- 区分点：能否指出 MAGIC 是“长按后松手”触发。

**参考答案**：
1. `bsp_key.c` 的 `bsp_key_scan()` 检测到 K2 长按后抬起，生成 `KEY_LONG_UP` 事件；
2. `bsp_key_get_msg()` 查 `mic_emit_key_msg_tbl`：`port_key.c:51-57` 非 K12 表中 `KEY_ID_K2` 的 `KEY_LONG_UP` 列（索引 3）映射到 `MSG_CHANGE_MAGIC`；
3. `msg_enqueue(MSG_CHANGE_MAGIC)`；
4. `func_mic_emit_message()` 调 `magic_effect_level_change()`，切换 `magic_cfg.magic_effect_level` 并调 `magic_audio_param_set()`；
5. 同时调 `wireless_tx_cmd_fw()` 同步 MAGIC 等级给 RX。

**评估标准**：
- ⭐⭐⭐ 完整链 + 三个文件；
- ⭐⭐ 链正确但缺文件；
- ⭐ 完全不知道。

---

### 题目 6.4 `KEY_LONG_UP` 与 `KEY_LONG` 的区别（★★ / 源码追踪）

**题目**：`KEY_LONG_UP` 与 `KEY_LONG` 是两种不同的事件。它们的区别是什么？为什么 MAGIC 选择“长按抬起”而非“长按下”作为触发？

**考察点**：
- 主考点：按键事件语义；
- 副考点：用户体验设计；
- 区分点：能否说明“长按抬起”避免误触。

**参考答案**：
- `KEY_LONG`：长按中（按住不放）；
- `KEY_LONG_UP`：长按后松手。

MAGIC 用 `KEY_LONG_UP` 是因为长按过程中用户可能犹豫，松手才确认切换，避免误触；同时长按可能与其他功能（如关机）冲突，松手触发更安全。

**评估标准**：
- ⭐⭐⭐ 能解释区别与设计原因；
- ⭐⭐ 能说出区别；
- ⭐ 完全不知道。

---

### 题目 6.5 `MSG_VOICE_RM` 留壳价值（★★ / 源码追踪）

**题目**：`MSG_VOICE_RM` 的实现被注释了，只剩 `printf` 和连接检查。这种"留壳"的代码风格在嵌入式项目中的价值？

**考察点**：
- 主考点：代码可维护性；
- 副考点：预留接口；
- 区分点：能否说出“便于后续恢复/调试”。

**参考答案**：
留壳保留了消息入口、打印和连接检查，后续若要恢复消原音功能，只需填回算法调用即可，无需改按键表/消息分派。同时保留了调试信息，便于确认按键事件已到达。

**评估标准**：
- ⭐⭐⭐ 能说出维护与调试价值；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 6.6 PP 键关机要按很久且偶尔失灵（★★ / 调试排查）

**题目**：用户反馈：按 PP 键关机要按很久，且偶尔失灵。请列排查步骤。

**考察点**：
- 主考点：按键灵敏度/时长排查；
- 副考点：`PWRON_PRESS_TIME` / `PWROFF_PRESS_TIME`；
- 区分点：能否从硬件、配置、扫描周期三方面排查。

**参考答案**：
1. 检查硬件：PP 键接触是否良好，ADC 键分压电阻是否正确；
2. 检查配置：`xcfg_cb.pwron_press_time` / `pwroff_press_time` 是否设置过长；
3. 检查扫描：`bsp_key_scan()` 周期是否被其他任务拉长（如 `WIRELESS_MIC_KEY_PRESS_MUTE_EN` 时改为 1ms ISR 扫描）；
4. 检查消息：`MSG_PWR_HOLD` 是否被正确入队，`func_message()` 是否处理。

**评估标准**：
- ⭐⭐⭐ 4 步完整；
- ⭐⭐ 2-3 步；
- ⭐ 只说一句。

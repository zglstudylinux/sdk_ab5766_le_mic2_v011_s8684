# 01 系统架构与角色调度 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：super-loop 主循环、角色判定、连接生命周期、bsp_sys_init 初始化顺序

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，单固件双角色（TX Mic Emit / RX Adapter）。
- 架构特征：非 RTOS，应用层 super-loop 状态机 `func_run()`，平台库提供 bt_stack/aupcm/driver/timer 等线程。空闲 24M 主频，无线连接后抬到 160M。
- 本卷重点：`func_cb.sta` 状态机、`bsp_sys_init()` 初始化序列、`wireless_emit_notice()` 连接/断开流程。
- 核心源码入口：
  - `projects/microphone/main.c:5-22`
  - `functions/func.c:134-178`
  - `bsp/bsp_sys.c:199-267`
  - `modules/wireless/wireless_proc.c:5-18,90-295`

---

## 1. 概念理解题

### 题目 1.1 顶层状态机如何分派（★★★ / 概念理解）

**题目**：`func_run()` 如何决定进入 `func_mic_emit()` 还是 `func_adapter()`？`func_cb.sta` 何时会被改变？

**考察点**：
- 主考点：`func_run()` 与角色宏的关系；
- 副考点：`FUNC_LE_DUT_EN` 覆盖、断开重连后的状态恢复；
- 区分点：能否说出 `wireless_role_is_adapter()` 与 `cfg_wireless_role` 的关系。

**参考答案**：
`functions/func.c:134-178`：
```c
if (wireless_role_is_adapter()) {
    func_cb.sta = FUNC_ADAPTER;
} else {
    func_cb.sta = FUNC_MIC_EMIT;
}
```
`wireless_role_is_adapter()` 是宏，取 `cfg_wireless_role`（`libs/ble/api_wireless_mic.h:52`）。`cfg_wireless_role` 由 `wireless_mic_role_init()` 在启动时根据 `xcfg_cb` 设置。进入 `while(1)` 后，若 `func_cb.sta` 被无线事件改变（如断开/重新连接且角色不变）通常不切换，除非显式调用 `func_cb.sta = FUNC_PWROFF` 或 DUT 模式。

**评估标准**：
- ⭐⭐⭐ 能引用行号并解释 `func_cb.sta` 的变化条件；
- ⭐⭐ 能说出二选一，但无行号；
- ⭐ 完全不知道。

---

### 题目 1.2 `bsp_sys_init()` 初始化顺序（★★★ / 概念理解）

**题目**：请列出 `bsp_sys_init()` 的主要初始化步骤（至少 6 步），并说明哪一步负责加载运行时配置 `xcfg_cb`。

**考察点**：
- 主考点：BSP 初始化序列；
- 副考点：`xcfg_init()` 的位置；
- 区分点：能否区别 `config_ab5766_le_mic.h` 与 `xcfg_cb`。

**参考答案**：
`bsp/bsp_sys.c:199-267` 的主要步骤：
1. `xcfg_init(&xcfg_cb, sizeof(xcfg_cb))` —— 加载运行时配置（约 202 行）；
2. `sys_get_rand_key_init()` —— 随机种子；
3. `bsp_var_init()` —— 系统变量/按键/USB/低功耗/消息队列；
4. `wireless_mic_var_init()` —— 角色判定与代码段加载；
5. `pmu_init(cfg_pmu_get())` —— PMU；
6. SARADC/按键/LED/充电/VBAT 条件初始化；
7. `power_on_check()` —— 开机按键确认；
8. `sys_clk_set(SYS_CLK_SEL)` / `bsp_param_init()` / `xosc_init()` / `sys_set_tmr_enable(1,1)`；
9. `mic_bias_trim_init()`（可选）；
10. `eq_drc_dbg_init()`（可选，UART 在线调参）。

`xcfg_cb` 由第 1 步 `xcfg_init()` 从 `xcfg.bin` 加载。

**评估标准**：
- ⭐⭐⭐ 列 6+ 步并指出 xcfg_init 位置；
- ⭐⭐ 列 4-5 步；
- ⭐ 不知道。

---

## 2. 源码追踪题

### 题目 1.3 TX 从 main 到 mic_emit_init（★★★ / 源码追踪）

**题目**：从 `main()` 到 `mic_emit_init()` 的完整调用链是什么？要求引用文件和行号。

**考察点**：
- 主考点：TX 初始化调用链；
- 副考点：连接事件驱动；
- 区分点：能否指出 `mic_emit_init()` 是连接后才调，而不是 `bsp_sys_init()` 阶段。

**参考答案**：
1. `main()` → `bsp_sys_init()` → `func_run()`；
2. `func_run()` 判 `func_cb.sta = FUNC_MIC_EMIT` → `func_mic_emit()`；
3. `func_mic_emit()` 进入主循环，等待无线连接；
4. `BT_NOTICE_WIRELESS_CONNECTED` 到达 `wireless_emit_notice()`（`modules/wireless/wireless_proc.c:90-295`）；
5. 首条链路且角色为 Device 时，`mic_emit_init()` 被调（约 106-109 行）。

**评估标准**：
- ⭐⭐⭐ 完整链路 + 强调“连接后才初始化”；
- ⭐⭐ 只到 func_mic_emit；
- ⭐ 完全不知道。

---

### 题目 1.4 断开与复位顺序（★★★ / 源码追踪）

**题目**：最后一条链路断开时，`wireless_emit_notice()` 中 `sys_cb.mic_alg_en=0`、`adapter_reset()`/`mic_emit_reset()`、`sys_clk_free()` 的调用顺序是什么？为什么必须先清 `mic_alg_en`？

**考察点**：
- 主考点：断开生命周期；
- 副考点：防止 DMA 回调闯入已释放资源；
- 区分点：能否指出顺序反了会导致什么问题。

**参考答案**：
`modules/wireless/wireless_proc.c:177-194`：
1. `sys_cb.mic_alg_en = 0`（177-179 行）——先关总使能，阻止 SDADC kick 和新数据进算法；
2. Adapter 走 `adapter_reset(mic_num, 0)`，Device 走 `mic_emit_reset()` + `lowpwr_pwroff_auto_en()`（180-185 行）——释放 codec/算法/DAC；
3. `sys_clk_free(INDEX_DECODE)`（186 行）——释放 160M 请求。

若先复位再清 `mic_alg_en`，SDADC DMA 回调可能在 codec 已退出后仍进入，导致空指针或数据错乱。

**评估标准**：
- ⭐⭐⭐ 顺序正确并能解释原因；
- ⭐⭐ 顺序正确但无解释；
- ⭐ 顺序错误。

---

## 3. 调试排查题

### 题目 1.5 TX 连接成功但无声音（★★★ / 调试排查）

**题目**：TX 已连接（日志显示 connected），但 RX 耳机无声。给出至少 4 个嫌疑点并说明验证手段。

**考察点**：
- 主考点：TX 音频链排查；
- 副考点：`mic_alg_en`、SDADC、codec、无线发送；
- 区分点：能否分层（配置 → 采集 → 编码 → 发送）。

**参考答案**：
1. `sys_cb.mic_alg_en` 是否为 1（看日志或内存）；
2. SDADC 是否启动（`bsp_sdadc_init()` 打印、示波器看 MIC_BIAS）；
3. `mic_enc_proc_cb()` 是否被调（加 printf 或 GPIO 翻转）；
4. `lc3s_enc()` 是否执行（检查输出帧长度）；
5. `wireless_d2a_put_tx_frame()` 是否投递成功（dump 无线统计）。

**评估标准**：
- ⭐⭐⭐ 4+ 点并分层；
- ⭐⭐ 2-3 点；
- ⭐ 只说“看连接”。

---

## 4. 设计权衡题

### 题目 1.6 为什么不选 FreeRTOS（★★★ / 设计权衡）

**题目**：如果让你重新设计，你会选择 FreeRTOS 还是继续 super-loop？请从代码量、实时性、功耗三个角度说明理由。

**考察点**：
- 主考点：RTOS 选型；
- 副考点：本项目资源限制（128K Flash/61K RAM）；
- 区分点：能否量化或结合具体场景。

**参考答案**：
- **代码量**：FreeRTOS 内核 + heap + 任务栈至少多出几 KB~十几 KB Flash/RAM；本项目 Flash 128K、RAM 61K，留给算法的空间已紧张；
- **实时性**：音频最严苛的 2.5ms 帧由平台库 aupcm/driver 线程保证，应用层 super-loop 只需处理按键/消息，响应足够；FreeRTOS 任务切换开销反而可能增加 jitter；
- **功耗**：super-loop 空闲时可快速进 sleep；FreeRTOS idle 任务 + tick 中断会增加功耗。

结论：在当前资源与实时性需求下，继续 super-loop 更优；若未来功能暴增（多路音频、更多用户交互），再考虑 RTOS。

**评估标准**：
- ⭐⭐⭐ 三个角度并结合资源数字；
- ⭐⭐ 两个角度；
- ⭐ 泛泛而谈。

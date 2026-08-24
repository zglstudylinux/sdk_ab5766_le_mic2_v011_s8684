# 01 系统架构与角色调度 面试题（Mid · 候选人版）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 本卷重点：`func_cb.sta` 状态机、`bsp_sys_init()` 初始化序列、`wireless_emit_notice()` 连接/断开流程。

> 本版不含参考答案与评估标准，建议先自行尝试回答。

---

## 1. 概念理解题

### 题目 1.1 顶层状态机如何分派（★★★ / 概念理解）

**题目**：`func_run()` 如何决定进入 `func_mic_emit()` 还是 `func_adapter()`？`func_cb.sta` 何时会被改变？

**考察点**：`func_run()` 与角色宏的关系；`FUNC_LE_DUT_EN` 覆盖、断开重连后的状态恢复；`wireless_role_is_adapter()` 与 `cfg_wireless_role` 的关系。

### 题目 1.2 `bsp_sys_init()` 初始化顺序（★★★ / 概念理解）

**题目**：请列出 `bsp_sys_init()` 的主要初始化步骤（至少 6 步），并说明哪一步负责加载运行时配置 `xcfg_cb`。

**考察点**：BSP 初始化序列；`xcfg_init()` 的位置；`config_ab5766_le_mic.h` 与 `xcfg_cb` 的区别。

## 2. 源码追踪题

### 题目 1.3 TX 从 main 到 mic_emit_init（★★★ / 源码追踪）

**题目**：从 `main()` 到 `mic_emit_init()` 的完整调用链是什么？要求引用文件和行号。

**考察点**：TX 初始化调用链；连接事件驱动；`mic_emit_init()` 的调用时机。

### 题目 1.4 断开与复位顺序（★★★ / 源码追踪）

**题目**：最后一条链路断开时，`wireless_emit_notice()` 中 `sys_cb.mic_alg_en=0`、`adapter_reset()`/`mic_emit_reset()`、`sys_clk_free()` 的调用顺序是什么？为什么必须先清 `mic_alg_en`？

**考察点**：断开生命周期；防止 DMA 回调闯入已释放资源；顺序反了会导致什么问题。

## 3. 调试排查题

### 题目 1.5 TX 连接成功但无声音（★★★ / 调试排查）

**题目**：TX 已连接（日志显示 connected），但 RX 耳机无声。给出至少 4 个嫌疑点并说明验证手段。

**考察点**：TX 音频链排查；分层（配置 → 采集 → 编码 → 发送）。

## 4. 设计权衡题

### 题目 1.6 为什么不选 FreeRTOS（★★★ / 设计权衡）

**题目**：如果让你重新设计，你会选择 FreeRTOS 还是继续 super-loop？请从代码量、实时性、功耗三个角度说明理由。

**考察点**：RTOS 选型权衡。

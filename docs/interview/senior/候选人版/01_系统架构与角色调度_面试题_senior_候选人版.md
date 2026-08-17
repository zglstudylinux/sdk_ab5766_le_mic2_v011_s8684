# 01 系统架构与角色调度 面试题（Senior · 候选人版）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 候选人版（先自行作答，无答案）

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，单固件双角色（TX Mic Emit / RX Adapter）。
- 架构特征：非 RTOS，应用层 super-loop 状态机 `func_run()`，平台库提供 bt_stack/aupcm/driver/timer 等线程。空闲 24M 主频，无线连接后抬到 160M。
- 本卷重点：`func_cb.sta` 状态机、`xcfg_cb` 角色判定、`wireless_emit_notice()` 连接生命周期、`ram.ld` 角色段复用。

---

## 1. 概念理解题

### 题目 1.1 super-loop 与平台线程的边界（★★★★ / 概念理解）

**题目**：
项目注释“应用层无 RTOS”，但 `os/thread_dec.c`、`os/thread_alg.c` 存在“线程分发”。请解释 super-loop 主循环与平台库提供的“伪线程”的边界与协作模型。

**考察点**：
- 主考点：对裸机 super-loop + 厂商平台线程模型的理解；
- 副考点：应用层 vs 平台层的职责划分；
- 区分点：能否指出 `func_run()` 与 `kick_dec_prio_trans()` / `kick_alg_prio_trans()` 的关系。

**可能的陷阱**：候选人若只答“没有 RTOS，都是中断”，需追问“那 LC3S 解码在哪跑？”。

---

### 题目 1.2 `xcfg_cb` 与编译期宏的分层（★★★★ / 概念理解）

**题目**：
`xcfg_cb` 是运行时从 Flash 参数区加载的结构体（`projects/microphone/xcfg.h:7-86`），而 `config_ab5766_le_mic.h` 是编译期宏。请解释二者如何共同决定“同一份固件既能当 TX 也能当 RX”。

**考察点**：
- 主考点：编译期宏 vs 运行时参数；配置工具产线与代码一致性的关系；
- 副考点：`wireless_adapter_en`/`wireless_mic_emit_en` 的优先级；
- 区分点：能否给出“改角色必须改 setting→xcfg.bin→重新烧录或在线写参”的完整链路。

---

## 2. 源码追踪题

### 题目 1.3 从 main 到角色判定（★★★★★ / 源码追踪）

**题目**：
从 `main()` 到判决 `cfg_wireless_role = true` 的完整调用链是什么？要求引用 `main.c`、`bsp_sys.c`、`wireless_proc.c` 三处函数名 + 行号。

**考察点**：
- 主考点：跨文件调用链追踪；
- 副考点：`bsp_sys_init()` 与 `wireless_mic_var_init()` 的顺序；
- 区分点：能否指出 `wireless_mic_role_init()` 不是 `bsp_sys_init()` 直接调，而是由 `wireless_mic_var_init()` 调。

---

### 题目 1.4 连接成功的四步顺序（★★★★★ / 源码追踪）

**题目**：
`BT_NOTICE_WIRELESS_CONNECTED` 触发后，"抬主频 → 初始化算法 → 使能 `mic_alg_en` → 标记链路" 这四步的严格顺序是什么？为什么必须先初始化算法再置 `mic_alg_en`？如果反过来会发生什么？

**考察点**：
- 主考点：`wireless_emit_notice()` 连接生命周期；
- 副考点：状态标志位 `sys_cb.mic_alg_en` 的时序含义；
- 区分点：能否举出“反过来”的具体失败模式。

---

### 题目 1.5 linker 段双重映射（★★★★★ / 源码追踪）

**题目**：
`ram.ld` 中 `__comm_adapter_vma` 与 `__comm_emit_vma` 是同一地址（`comm + 27k`），两个 section 都 `AT > flash`。请描述上电后 `.code_adapter` 与 `.code_emit` 在 Flash/RAM 中的实际布局，并解释 `wireless_mic_load_code()` 的拷贝逻辑。

**考察点**：
- 主考点：链接脚本 VMA/LMA、`AT()`、RAM 复用设计；
- 副考点：为什么 TX/RX 代码不能同时驻留；
- 区分点：能否说明 `memcpy` 的源地址/目标地址/大小来自哪里。

---

## 3. 调试排查题

### 题目 1.6 上电串口卡死在 SDK 版本打印（★★★★ / 调试排查）

**题目**：
上电后串口只输出到 `"SDK: vXXXX LIBS: vXXXX"`，无更多打印。给出至少 3 个可能原因，按嫌疑度排序，并说出验证手段。

**考察点**：
- 主考点：启动时序调试、UART/Debug 配置；
- 副考点：`main.c` 打印顺序、`bsp_sys_init()` 常见挂点；
- 区分点：能否结合 `main.c`、`bsp_uart_debug.c`、晶振/时钟分层排查。

---

### 题目 1.7 TX 板被错判为 RX（★★★★ / 调试排查）

**题目**：
发现角色错判（TX 板被识别为 RX）。如何从 `xcfg.bin`、`xcfg_cb` 内存、`wireless_mic_role_init` 三个角度定位根因？

**考察点**：
- 主考点：运行时配置加载问题定位；
- 副考点：setting 文件 vs `xcfg.bin` vs 内存的一致性；
- 区分点：能否给出分层排查路径。

---

## 4. 设计权衡题

### 题目 1.8 上电即跑 vs 连接才初始化（★★★★★ / 设计权衡）

**题目**：
本工程可以选择“上电即跑算法”或“连接才初始化算法”。当前是后者。请分析两套方案在功耗、启动时间、复杂度上的取舍，并解释厂商选择后者的动机。

**考察点**：
- 主考点：资源初始化时机 vs 功耗/启动时间；
- 副考点：无线麦克风的应用场景；
- 区分点：能否量化启动时间差异与功耗差异。

---

### 题目 1.9 comm_adapter / comm_emit VMA 复用（★★★★★ / 设计权衡）

**题目**：
`comm_adapter` / `comm_emit` 做 VMA 重叠复用 RAM 是一种典型 flash-fat / ram-tight 设计。请推理：如果 RAM 增到 256K，你会保留这个设计吗？为什么？

**考察点**：
- 主考点：内存资源 vs 工程复杂度的权衡；
- 副考点：角色切换需求；
- 区分点：能否说出 RAM 足够后改独立 VMA 的收益与代价。

---

## 5. 编程实战题（可选）

### 题目 1.10 新增 FUNC_LE_DUT 模式（★★★ / 编程实战）

**题目**：
要求新增 `FUNC_LE_DUT` 模式作为出厂自检模式，在 `func_run()` 中插入判决分支。请口头描述修改 `func.c` 的最小改动。

**考察点**：
- 主考点：状态机扩展、`func_enter()` 键表绑定；
- 副考点：最小侵入性修改；
- 区分点：能否指出需要同时改 `func_info[]` 和 `port_key.c`/`func_le_dut.c`。

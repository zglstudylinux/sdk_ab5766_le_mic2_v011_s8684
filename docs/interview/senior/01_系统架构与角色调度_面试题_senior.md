# 01 系统架构与角色调度 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 核心考察：super-loop 与平台线程的边界、单固件双角色、连接驱动算法生命周期、RAM 复用与链接脚本

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，单固件双角色（TX Mic Emit / RX Adapter）。
- 架构特征：非 RTOS，应用层 super-loop 状态机 `func_run()`，平台库提供 bt_stack/aupcm/driver/timer 等线程。空闲 24M 主频，无线连接后抬到 160M。
- 本卷重点：`func_cb.sta` 状态机、`xcfg_cb` 角色判定、`wireless_emit_notice()` 连接生命周期、`ram.ld` 角色段复用。
- 核心源码入口：
  - `projects/microphone/main.c:5-22`
  - `functions/func.c:134-178`
  - `bsp/bsp_sys.c:199-267`
  - `modules/wireless/wireless_proc.c:5-18,90-295`
  - `projects/microphone/ram.ld`

---

## 1. 概念理解题

### 题目 1.1 super-loop 与平台线程的边界（★★★★ / 概念理解）

**题目**：项目注释“应用层无 RTOS”，但 `os/thread_dec.c`、`os/thread_alg.c` 存在“线程分发”。请解释 super-loop 主循环与平台库提供的“伪线程”的边界与协作模型。

**考察点**：
- 主考点：对裸机 super-loop + 厂商平台线程模型的理解；
- 副考点：应用层 vs 平台层的职责划分；
- 区分点：能否指出 `func_run()` 与 `kick_dec_prio_trans()` / `kick_alg_prio_trans()` 的关系。

**参考答案**：
应用层是一个显式 `while(1)` 状态机，见 `functions/func.c:134-178`：角色选定后进入 `func_adapter()` 或 `func_mic_emit()`，各自主循环里轮询 `wireless_mic_proc()`、`func_process()`（含 `bt_run_loop()`、充电、`pwroff_process()`）以及 `msg_dequeue()`。

平台库（`libs/cpu`）则提供 bt_stack、aupcm、driver、timer 等线程与信号量，见 `libs/cpu/api_os.h`。音频/算法作为这些线程的一部分运行：
- SDADC/SDDAC DMA 中断只做事件转移（`sdadc_done_proc_kick()`），复杂处理交给 AUPCM 线程；
- 解码在 `thread_dec.c` 中通过 `MSG_MIC_DEC0/1` 分发到 `mic_dec_prco_cb()`；
- AINS4_32K/Robotization 在 `thread_alg.c` 中分发。

**评估标准**：
- ⭐⭐⭐ 能同时讲清应用层 super-loop 与平台线程的信号量/kick 机制，并指出 `msg_dequeue()` 与 `func_*_message()` 的位置；
- ⭐⭐ 能区分主循环与线程，但说不清 AUPCM/算法线程的 kick 路径；
- ⭐ 说不清边界，或把 super-loop 说成“没有任何并发”。

**可能的陷阱**：候选人若只答“没有 RTOS，都是中断”，需追问“那 LC3S 解码在哪跑？”。

---

### 题目 1.2 `xcfg_cb` 与编译期宏的分层（★★★★ / 概念理解）

**题目**：`xcfg_cb` 是运行时从 Flash 参数区加载的结构体（`projects/microphone/xcfg.h:7-86`），而 `config_ab5766_le_mic.h` 是编译期宏。请解释二者如何共同决定“同一份固件既能当 TX 也能当 RX”。

**考察点**：
- 主考点：编译期宏 vs 运行时参数；配置工具产线与代码一致性的关系；
- 副考点：`wireless_adapter_en`/`wireless_mic_emit_en` 的优先级；
- 区分点：能否给出“改角色必须改 setting->xcfg.bin->重新烧录或在线写参”的完整链路。

**参考答案**：
编译期宏 `config_ab5766_le_mic.h` 决定“哪些代码被编译进来”，例如 `WIRELESS_MIC_EQ_DRC_EN`、`ADAPTER_USB_MIC_RX_EN`。

运行时参数 `xcfg_cb` 由 `xcfg.xm` 生成 `xcfg.bin/h`，`bsp_sys_init()` 调 `xcfg_init(&xcfg_cb,...)` 加载（`bsp/bsp_sys.c:202-204`）。关键字段 `wireless_adapter_en`、`wireless_mic_emit_en`决定 `wireless_mic_role_init()` 中 `cfg_wireless_role` 的取值（`modules/wireless/wireless_proc.c:5-18`）。Adapter 优先于 Mic Emit；两者都关时默认 TX。

**评估标准**：
- ⭐⭐⭐ 能举出具体字段（`wireless_adapter_en` 等）+ 调用链 + `setting` 文件路径（`Output/bin/Settings/wireless_adapter.setting`）；
- ⭐⭐ 能分层但说不清 `xcfg.bin` 的生成工具；
- ⭐ 把编译期宏与运行时参数混为一谈。

---

## 2. 源码追踪题

### 题目 1.3 从 main 到角色判定（★★★★★ / 源码追踪）

**题目**：从 `main()` 到判决 `cfg_wireless_role = true` 的完整调用链是什么？要求引用 `main.c`、`bsp_sys.c`、`wireless_proc.c` 三处函数名 + 行号。

**考察点**：
- 主考点：跨文件调用链追踪；
- 副考点：`bsp_sys_init()` 与 `wireless_mic_var_init()` 的顺序；
- 区分点：能否指出 `wireless_mic_role_init()` 不是 `bsp_sys_init()` 直接调，而是由 `wireless_mic_var_init()` 调。

**参考答案**：
1. `main()` 位于 `projects/microphone/main.c:5-22`；
2. `bsp_sys_init()` 位于 `bsp/bsp_sys.c:199-267`；
3. `bsp_sys_init()` 中的 `wireless_mic_var_init()` 位于 `bsp/bsp_sys.c:211`，实现见 `modules/wireless/wireless_proc.c:342-348`；
4. `wireless_mic_var_init()` 依次调：
   - `wireless_mic_role_init()`：读 `xcfg_cb.wireless_adapter_en / wireless_mic_emit_en`，设置 `cfg_wireless_role`，见 `modules/wireless/wireless_proc.c:5-18`；
   - `wireless_mic_load_code()`：按角色拷贝 `.code_adapter` 或 `.code_emit` 到复用 RAM，见 `modules/wireless/wireless_proc.c:24-32`；
   - `wireless_cmd_init()`。

**评估标准**：
- ⭐⭐⭐ 完整列出 `main → bsp_sys_init → wireless_mic_var_init → wireless_mic_role_init`，并注明两个源文件行号；
- ⭐⭐ 能列出但漏掉 `wireless_mic_var_init()`；
- ⭐ 只说“在 bsp_sys_init 里”，无细节。

---

### 题目 1.4 连接成功的四步顺序（★★★★★ / 源码追踪）

**题目**：`BT_NOTICE_WIRELESS_CONNECTED` 触发后，"抬主频 → 初始化算法 → 使能 `mic_alg_en` → 标记链路" 这四步的严格顺序是什么？为什么必须先初始化算法再置 `mic_alg_en`？如果反过来会发生什么？

**考察点**：
- 主考点：`wireless_emit_notice()` 连接生命周期；
- 副考点：状态标志位 `sys_cb.mic_alg_en` 的时序含义；
- 区分点：能否举出“反过来”的具体失败模式。

**参考答案**：
在 `modules/wireless/wireless_proc.c:98-151` 中，当 `connected_sta == 0`（首条链路）时：

1. `sys_clk_req(INDEX_DECODE, SYS_160M)`（约 102-104 行）抬升主频；
2. Adapter 走 `adapter_init()`，Device 走 `mic_emit_init()`（约 104-109 行），完成 codec/SDADC/算法/PACC 等初始化；
3. `sys_cb.mic_alg_en = 1`（约 110 行）打开音频算法与收发总使能；
4. `wireless_mic.connected_sta |= BIT(mic_num)`（约 123 行）标记链路。

反过来若先置 `mic_alg_en=1` 再初始化，则 SDADC DMA 回调可能进来时 codec/算法尚未就绪，`lc3s_enc_init`、`ains4_32k_init`、`loc_mic_pacc_init` 等还没跑，导致内存族错误、空指针、或首帧即丢。

**评估标准**：
- ⭐⭐⭐ 能给出准确行号并解释状态机；
- ⭐⭐ 能讲顺序但无行号；
- ⭐ 把顺序讲反或漏掉抬主频。

**可能的陷阱**：有些候选人会答“`adapter_init` 和 `mic_emit_init` 同时跑”，实际是根据 `wireless_role_is_adapter()` 二选一。

---

### 题目 1.5 linker 段双重映射（★★★★★ / 源码追踪）

**题目**：`ram.ld` 中 `__comm_adapter_vma` 与 `__comm_emit_vma` 是同一地址（`comm + 27k`），两个 section 都 `AT > flash`。请描述上电后 `.code_adapter` 与 `.code_emit` 在 Flash/RAM 中的实际布局，并解释 `wireless_mic_load_code()` 的拷贝逻辑。

**考察点**：
- 主考点：链接脚本 VMA/LMA、`AT()`、RAM 复用设计；
- 副考点：为什么 TX/RX 代码不能同时驻留；
- 区分点：能否说明 `memcpy` 的源地址/目标地址/大小来自哪里。

**参考答案**：
- `ram.ld` 中 `comm` 27K，`comm_adapter` 与 `comm_emit` 都从 `comm + 27K` 开始（VMA 相同），大小 ~34K；
- `.code_adapter` 和 `.code_emit` 的 LMA 都在 Flash 中（`AT > flash`），均参与固件镜像；
- 上电后 RAM 里这两个 VMA 区域是空的（或残旧），`bsp_sys_init()` 调 `wireless_mic_var_init()` → `wireless_mic_load_code()`（`modules/wireless/wireless_proc.c:24-32`）按角色拷贝：
  - Adapter 时 `memcpy(&__comm_adapter_vma, &__comm_apapter_lma, &__comm_apapter_size)`；
  - Emit 时 `memcpy(&__comm_emit_vma, &__comm_emit_lma, &__comm_emit_size)`。
- 两份代码互斥，运行时只驻留当前角色的那一份；另一份仍在 Flash 里不被加载。

**评估标准**：
- ⭐⭐⭐ 能说出 VMA/LMA 区别 + `__comm_*_lma/size` 符号来源 + 拷贝是运行时按需；
- ⭐⭐ 能讲“同一块 RAM 复用”，但说不清何时拷贝；
- ⭐ 以为两份代码都常驻 RAM。

---

## 3. 调试排查题

### 题目 1.6 上电串口卡死在 SDK 版本打印（★★★★ / 调试排查）

**题目**：上电后串口只输出到 `"SDK: vXXXX LIBS: vXXXX"`，无更多打印。给出至少 3 个可能原因，按嫌疑度排序，并说出验证手段。

**考察点**：
- 主考点：启动时序调试、UART/Debug 配置；
- 副考点：`main.c` 打印顺序、`bsp_sys_init()` 常见挂点；
- 区分点：能否结合 `main.c`、`bsp_uart_debug.c`、晶振/时钟分层排查。

**参考答案**：
可能原因（按嫌疑度）：
1. **UART Debug GPIO/Baud 配置错误**：`BSP_UART_DEBUG_EN=GPIO_PB3`（`config_ab5766_le_mic.h:24`），若接线错或波特率不是 1.5Mbps，会看不到后续打印。验证：示波器看 PB3 波形、核对 `bsp/bsp_uart_debug.c`。
2. **晶振未起振或频率不对**：`SYS_CLK_SEL=SYS_24M`（`config:19`），`sys_clk_set()` 在 `bsp_sys.c:241`；24M 不出则后续 printf 挂。验证：测 24M 晶振输出、查 `xosc_init()`。
3. **`bsp_sys_init()` 中某步卡住**：如 `power_on_check()` 等待 PP 键（`bsp_sys.c:152-196`）、或 `xcfg_init()` 失败导致 assert/死循环。验证：加临时 GPIO 翻转、逐段给 `bsp_sys_init()` 加打印。
4. ** Flash 中代码/资源区损坏**，导致 `main` 后续跳转失败。验证：重新烧录已知良好固件对比。

**评估标准**：
- ⭐⭐⭐ 能列 3+ 条并按嫌疑度排序 + 验证手段；
- ⭐⭐ 能列 2 条但无验证手段；
- ⭐ 只说“程序挂了”。

---

### 题目 1.7 TX 板被错判为 RX（★★★★ / 调试排查）

**题目**：发现角色错判（TX 板被识别为 RX）。如何从 `xcfg.bin`、`xcfg_cb` 内存、`wireless_mic_role_init` 三个角度定位根因？

**考察点**：
- 主考点：运行时配置加载问题定位；
- 副考点：setting 文件 vs `xcfg.bin` vs 内存的一致性；
- 区分点：能否给出分层排查路径。

**参考答案**：
1. **看 setting 文件**：确认烧录的是 `wireless_mic_emit.setting` 还是 `wireless_adapter.setting`。`Output/bin/Settings/wireless_mic_emit.setting` 中 `wireless_adapter_en=False`，`wireless_mic_emit_en=True`。
2. **看 `xcfg.bin`**：用 xmaker/hex 查看 `xcfg_cb.wireless_adapter_en` 对应 offset/bit 是否为 0。
3. **看内存**：通过调试器或 UART printf 打印 `xcfg_cb.wireless_adapter_en`、`xcfg_cb.wireless_mic_emit_en` 的值。
4. **看 `wireless_mic_role_init()`**：在 `modules/wireless/wireless_proc.c:5-18` 加打印或断点，看走了哪个分支。
5. 若 setting 正确但 `xcfg.bin` 错，说明 xmaker/烧录器问题；若 `xcfg.bin` 正确但内存错，说明 Flash 参数区读错或被后续代码覆盖。

**评估标准**：
- ⭐⭐⭐ 分层清晰并能说出具体字段/文件；
- ⭐⭐ 能定位但无分层；
- ⭐ 只会说“改宏”。

---

## 4. 设计权衡题

### 题目 1.8 上电即跑 vs 连接才初始化（★★★★★ / 设计权衡）

**题目**：本工程可以选择“上电即跑算法”或“连接才初始化算法”。当前是后者。请分析两套方案在功耗、启动时间、复杂度上的取舍，并解释厂商选择后者的动机。

**考察点**：
- 主考点：资源初始化时机 vs 功耗/启动时间；
- 副考点：无线麦克风的应用场景；
- 区分点：能否量化启动时间差异与功耗差异。

**参考答案**：
- **上电即跑**：上电就初始化 SDADC/codec/算法，优点是 TX 采集可立即开始，无需等连接；缺点是始终耗电，初始化时间长（`lc3s_enc_init`、`ains4_32k_init`、PACC 合并初始化），开机来不及走按键/低功耗逻辑。
- **连接才初始化**：当前方案（`wireless_proc.c:98-115`），连接后抬主频到 160M、`adapter_init()`/`mic_emit_init()`、置 `mic_alg_en=1`；断开则反向 `adapter_reset`/`mic_emit_reset` + `sys_clk_free(INDEX_DECODE)`。优点是空闲低功耗（24M、无算法），快速响应按键；代价是首条连接有初始化延迟。

厂商动机：无线麦/适配器常见使用模式是“长时间空闲、短时通话”，连接驱动的初始化最省电，同时广播/扫描逻辑和解码/编码资源可分离。

**评估标准**：
- ⭐⭐⭐ 能从功耗/启动/复杂度量化对比 + 场景解释；
- ⭐⭐ 只定性；
- ⭐ 完全没想到功耗。

---

### 题目 1.9 comm_adapter / comm_emit VMA 复用（★★★★★ / 设计权衡）

**题目**：`comm_adapter` / `comm_emit` 做 VMA 重叠复用 RAM 是一种典型 flash-fat / ram-tight 设计。请推理：如果 RAM 增到 256K，你会保留这个设计吗？为什么？

**考察点**：
- 主考点：内存资源 vs 工程复杂度的权衡；
- 副考点：角色切换需求；
- 区分点：能否说出 RAM 足够后改独立 VMA 的收益与代价。

**参考答案**：
当 RAM 只有约 61K（27K + 34K）时，TX 和 RX 代码不能同时驻留，必须用 VMA 复用 + 启动拷贝；代价是上电拷贝时间、启动时序复杂度、调试难度（map 看起来两份代码同地址）。

若 RAM 增到 256K：
- **保留复用**的收益：Flash 镜像更小、同一份固件仍有“按需加载”的灵活性；
- **改独立 VMA** 的收益：运行时无需启动拷贝，调试 straightforward，可同时调试 TX/RX 代码路径；
- 代价：Flash 镜像需要同时保存两份，可能超过 128K 限制；产品仍需单固件双角色。

结论：若 RAM 充足且 Flash 也充足，可改独立 VMA + 独立加载以简化启动和调试；否则保留复用更稳。

**评估标准**：
- ⭐⭐⭐ 能同时考虑 RAM/Flash/调试/维护维度并给出明确选择；
- ⭐⭐ 只看 RAM；
- ⭐ 完全没考虑 Flash 或调试。

---

## 5. 编程实战题（可选）

### 题目 1.10 新增 FUNC_LE_DUT 模式（★★★ / 编程实战）

**题目**：要求新增 `FUNC_LE_DUT` 模式作为出厂自检模式，在 `func_run()` 中插入判决分支。请口头描述修改 `func.c` 的最小改动。

**考察点**：
- 主考点：状态机扩展、`func_enter()` 键表绑定；
- 副考点：最小侵入性修改；
- 区分点：能否指出需要同时改 `func_info[]` 和 `port_key.c`/`func_le_dut.c`。

**参考答案**：
1. 在 `functions/func.c:4-20` 的 `func_info[]` 中确认已有 `FUNC_LE_DUT` 条目（若无则补）；
2. 在 `func_run()` 中，`wireless_role_is_adapter()` 判角色前先检查 `FUNC_LE_DUT_EN` 或某个 `xcfg` 标志；若成立则 `func_cb.sta = FUNC_LE_DUT`；
3. `func_enter()` 会通过 `key_set_msg_tbl(func_info[func_cb.sta].msg_tbl)` 安装 LE DUT 键表；
4. `msg_le_dut.c` / `func_le_dut.c` 负责具体自检流程。

**评估标准**：
- ⭐⭐⭐ 能提到 `func_info[]` 键表和 `FUNC_LE_DUT_EN` 宏；
- ⭐⭐ 只说“加个 if”；
- ⭐ 完全没概念。

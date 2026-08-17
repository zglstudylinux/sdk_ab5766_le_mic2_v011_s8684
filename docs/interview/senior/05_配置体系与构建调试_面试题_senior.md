# 05 配置体系与构建调试 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 核心考察：构建系统设计、链接脚本高级用法、量产配置管理、在线调参的边界

## 0. 项目上下文速览

- 芯片/项目：AB5766 LE Mic AB5766 LE Mic。
- 配置三层：`config.h`（选产品）→ `config_ab5766_le_mic.h`（编译宏）→ `config_extra.h`（派生）→ `xcfg.xm/bin/h`（运行时参数区）。
- 构建链：`app.cbp` → `prebuild.bat (xmaker 生成 res.bin/xcfg.bin/effect.c/h)` → 编译链接 → `postbuild.bat` → `app.dcf`。
- 本卷重点：从“修改一个功能”到“生成可烧录固件”的完整链路、`AT()` 段与 `ram.ld` 的高级使用、量产配置工具的职责。
- 核心源码入口：
  - `projects/microphone/config.h`
  - `projects/microphone/config_ab5766_le_mic.h`
  - `projects/microphone/config_extra.h`
  - `projects/microphone/ram.ld`
  - `projects/microphone/Output/bin/prebuild.bat`、`postbuild.bat`、`app.xm`

---

## 1. 概念理解题

### 题目 5.1 `config_extra.h` 的“派生配置”责任（★★★★ / 概念理解）

**题目**：`config_extra.h` 的作用是“把 `config_ab5766_le_mic.h` 中的宏做安全派生与校验”。请举 3 个例子，并说明如果没有 `config_extra.h` 会有什么风险。

**考察点**：
- 主考点：构建期依赖管理；
- 副考点：条件编译的冲突防范；
- 区分点：能否举出真实例子。

**参考答案**：
`config_extra.h` 典型责任：
1. **派生宏**：如 `BSP_KEY_EN = (BSP_ADKEY_EN || BSP_IOKEY_EN)`，`DNR_FRE_EN = (WIRELESS_MIC_DNR_FRE_EN || ADAPTER_DNR_FRE_EN)`；
2. **校验/冲突**：如检查 `FLASH_SIZE` 与 `CM_SIZE` 是否满足 4K 对齐、检查 `WIRELESS_CON_VERS` 是否支持所选 codec；
3. **强制覆盖**：如 FOTA 未启用时强制 `AB_FOT_BLE_EN=0`；ECHO 与 Room Reverb 同开时强制 ECHO delay=0。

若缺失，可能出现：宏冲突（充电同时开与关）、Flash 区不对齐、资源重复分配，导致链接错误或运行时异常。

**评估标准**：
- ⭐⭐⭐ 能举 3 例并解释；
- ⭐⭐ 能举 2 例；
- ⭐ 只能举 1 例或不清楚。

---

### 题目 5.2 `app.xm` 的打包结构（★★★★ / 概念理解）

**题目**：`app.xm` 定义了 DCF（下载包）的组成。请说明 `make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin, updater.bin)` 的每一部分作用。

**考察点**：
- 主考点：固件打包结构；
- 副考点：xmaker 语法；
- 区分点：能否指出各部分在运行时的作用。

**参考答案**：
1. `header.bin`：固件头信息（版本、签名、长度、校验等），下载器首先读取；
2. `app.bin`：主应用固件二进制（text/rodata/data），运行于 RAM/Flash；
3. `res.bin`：资源二进制（提示音、effect 参数等），由代码在运行时索引；
4. `xcfg.bin`：运行时配置（角色、名称、按键时长、电池电压阈值等），由 `bsp_sys_init()` 的 `xcfg_init()` 读取；
5. `updater.bin`：固件更新/回滚帮助程序（如双备份/FOTA），若无 FOTA 功能可为空或简单。

**评估标准**：
- ⭐⭐⭐ 能说明 5 部分并对应运行时读取点；
- ⭐⭐ 能说明 4 部分；
- ⭐ 不能说明。

---

## 2. 源码追踪题

### 题目 5.3 `AT > flash` 与 `> comm_emit` 的链接器交互（★★★★★ / 源码追踪）

**题目**：`ram.ld` 中 `.code_emit` 这样写：

```ld
.code_emit ... > comm_emit AT > flash
```

请详细解释 `>` 和 `AT >` 两个指令在链接器输出 ELF 中的体现，以及启动代码如何完成拷贝。

**考察点**：
- 主考点：链接脚本的 VMA/LMA；
- 副考点：运行时拷贝函数；
- 区分点：能否指出 `> comm_emit` 决定 VMA，`AT > flash` 决定 LMA，启动时 `memcpy` 完成映射。

**参考答案**：
1. **链接器层面**：
   - `> comm_emit`：把该 section 的运行时地址（VMA）设为 `comm_emit` 区域的起始（约 `0x10C00 + 27K`）。
   - `AT > flash`：把该 section 的加载地址（LMA）设为 `flash` 区域。
   - ELF 中会生成两个程序头/表项：VMA 指向 RAM 地址，LMA 指向 Flash 地址。

2. **启动代码**：
   - `wireless_mic_load_code()`（`modules/wireless/wireless_proc.c:24-32`）按角色：
     - `memcpy(&__comm_adapter_vma, &__comm_adapter_lma, &__comm_adapter_size)`；
     - `memcpy(&__comm_emit_vma, &__comm_emit_lma, &__comm_emit_size)`。
   - 链接器自动生成 `__comm_adapter_vma` / `__comm_adapter_lma` / `__comm_adapter_size` 等符号。

3. **执行**：
   - PC 跳转到 VMA 地址执行代码，但数据/RO 从 Flash 读取。

**评估标准**：
- ⭐⭐⭐ 能区分 VMA/LMA 并说明启动拷贝；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

### 题目 5.4 `prebuild.bat` 失败后的连锁后果（★★★★ / 源码追踪）

**题目**：若 `prebuild.bat` 中 `copy effect.c ..\..\effect.c` 失败，后续编译会报什么错？请给出从 `app.cbp` 依赖到 `include.h` 再到 `toolkit.c` 的连锁反应。

**考察点**：
- 主考点：生成物在代码中的引用；
- 副考点：构建依赖链；
- 区分点：能否从“缺文件”→“缺宏”→“缺函数”推导。

**参考答案**：
1. `app.cbp` 的 prebuild 步骤失败，由于 `|| goto err`，构建终止；
2. 即使去掉 `|| goto err`，若 `effect.h` 没拷贝，编译阶段 `include.h` 或 `modules.h` 包含 `effect.h` 会报“file not found”；
3. 若 `effect.h` 存在但版本旧，`toolkit.c` 访问 `effect_info[]` 或 `ALL_MODE_NAME_SUM` 可能因定义不匹配而编译失败或运行时校验失败（`sumcheck != ALL_MODE_NAME_SUM`）。

**评估标准**：
- ⭐⭐⭐ 能从 prebuild 失败推导到编译/运行期；
- ⭐⭐ 能推导到编译期；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 5.5 链接后 RAM 溢出但 Flash 充足（★★★★★ / 调试排查）

**题目**：链接报 `region 'comm_adapter' overflowed by 1024 bytes`，但 Flash 还有 30K 剩余。请给出 5 个排查思路，并说明为什么不能“把代码从 Flash 移回 RAM” 的简单方案。

**考察点**：
- 主考点：RAM/Flash 权衡；
- 副考点：段归属、启动时间、功耗；
- 区分点：能否区分“代码可放 Flash 但 buffer/变量必须放 RAM”。

**参考答案**：
排查思路：
1. 查看 `map.txt` 中 `.code_adapter` / `.data_adapter` 的分解，定位超出的模块（如 USB、PLC decoder、MIX_DRC）；
2. 核对是否有大 buffer/常量误放入 `.data_adapter` 而非 `.rodata`；
3. 检查 `ADAPTER_USB_MIC_RX_EN=1` 是否引入了 USB 相关 buffer/代码；
4. 检查 `WIRELESS_CON_LINK_NB=2` 的 `mic_dec.pcm[WIRELESS_CON_LINK_NB]` 占用是否超标；
5. 检查 `pacc_eq_t` / `pacc_drc_t` 的 buffer 大小（如 `EQ_COEF_BUF_SIZE`）是否超出预期。

不能简单“把代码移回 RAM” 的原因：Flash 剩余 ≠ RAM 剩余；`.code_adapter` 的代码必须在 RAM 运行以满足实时性（Flash 直读会有 wait state，且 XIP 缓存不可控），不能为了编译通过而牺牲实时性。

**评估标准**：
- ⭐⭐⭐ 5 条思路且能解释为什么不能用 Flash；
- ⭐⭐ 3-4 条思路；
- ⭐ 只能说“改大 RAM”。

---

### 题目 5.6 ToolKit 连不上的“三层定位法”（★★★★ / 调试排查）

**题目**：BlueTrum ToolKit 连不上（串口 OK 无 response）。请用“串口 → HUART → 固件”三层定位。

**考察点**：
- 主考点：在线调参链路排查；
- 副考点：波特率、GPIO、`xcfg_cb.huart_en`；
- 区分点：能否从物理层→链路层→应用层排查。

**参考答案**：
1. **串口层**：确认 PC 端 COM 口正确、波特率 1500000、无占用；
2. **HUART 层**：确认板卡 `xcfg_cb.huart_io_sel` 对应 GPIO 与 ToolKit 线序匹配（单线半双工），用示波器查看 ToolKit 下发时是否有 12 字节头；
3. **固件层**：确认 `xcfg_cb.huart_en=1`（在 `Output/bin/Settings/*.setting` 中查看），`bsp_sys_init()` 是否调了 `eq_drc_dbg_init()`，UART ISR 是否收到数据（GPIO 翻转或 printf）。

**评估标准**：
- ⭐⭐⭐ 三层完整；
- ⭐⭐ 两层；
- ⭐ 只提串口。

---

## 4. 设计权衡题

### 题目 5.7 把 `xcfg` 改为宏编译 vs 参数区存储（★★★★★ / 设计权衡）

**题目**：若你负责 SDK 架构，请评估“把 `xcfg` 删除，全部改为编译期宏” 的利弊，并给出厂线测试的影响分析。

**考察点**：
- 主考点：配置架构选型；
- 副考点：产测/客户化/OTA 升级的灵活性；
- 区分点：能否从固件大小、产测灵活性、升级风险三方面对比。

**参考答案**：
**改为宏编译的优点**：
- 省去 `xcfg_init()` 加载步骤，启动稍快；
- 省去 `xcfg.bin` 打包与参数区管理；
- 链接时即可静态检查，避免运行时配置错误。

**缺点**：
- **产线测试不灵活**：每块板需单独编译固件（蓝牙地址、RF 参数不同），产线需维护 N 份固件；
- **客户定制难**：渠道商需重新编译，容易引入错误；
- **OTA 升级风险高**：固件与配置强耦合，一旦配置错误需全量重刷。

**产线影响**：
- 当前模式：一份固件 + 多份 .setting → 生成多个 `xcfg.bin`，产线只改配置不写代码；
- 宏编译模式：每块板可能需独立编译，固件版本爆炸，测试成本增加。

**结论**：保留 `xcfg` 更利于大规模量产与客户化。

**评估标准**：
- ⭐⭐⭐ 能从三个维度对比并给出明确结论；
- ⭐⭐ 能从两个维度；
- ⭐ 只想到一个点。

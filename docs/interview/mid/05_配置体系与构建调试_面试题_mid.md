# 05 配置体系与构建调试 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：三层配置模型、prebuild/postbuild 生成物、链接脚本 AT 段、ToolKit 在线调参

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风。
- 配置三层：`config.h`（选产品 `CONFIG_AB5766_LE_MIC`）→ `config_ab5766_le_mic.h`（编译宏）→ `config_extra.h`（派生检查）→ `xcfg.xm/bin/h`（运行时参数区）。
- 构建链：`app.cbp -> prebuild.bat (xmaker 生成 res.bin/xcfg.bin/effect.c/h) -> 编译链接 -> postbuild.bat -> app.dcf`。
- 本卷重点：配置分层职责、prebuild 生成物、`AT(.xxx)` 段放置、ToolKit 在线调参与固化。
- 核心源码入口：
  - `projects/microphone/config.h`
  - `projects/microphone/config_ab5766_le_mic.h`
  - `projects/microphone/config_extra.h`
  - `projects/microphone/Output/bin/prebuild.bat`
  - `projects/microphone/Output/bin/postbuild.bat`
  - `projects/microphone/ram.ld`

---

## 1. 概念理解题

### 题目 5.1 三层配置各自管什么（★★★ / 概念理解）

**题目**：列出本工程三层配置（`config.h` / `config_ab5766_le_mic.h` / `xcfg.bin`）各自的用途，并给一个具体的"改特性开关"案例。

**考察点**：
- 主考点：配置分层；
- 副考点：编译期 vs 运行时；
- 区分点：能否给出具体字段示例。

**参考答案**：
- `config.h`：选择产品（`#define USER_CONFIG CONFIG_AB5766_LE_MIC`）；
- `config_ab5766_le_mic.h`：编译期功能宏（如 `WIRELESS_MIC_EQ_DRC_EN`、`ADAPTER_USB_MIC_RX_EN`）；
- `xcfg.bin`：运行时参数（如 `wireless_adapter_en`、`le_name`、`rf_tx_pwr`）。

案例：把 `ADAPTER_USB_MIC_RX_EN` 从 0 改 1，需重新编译；把 `le_name` 从 "LE_MIC2" 改成 "LE_MIC3"，只需改 setting 重新生成 `xcfg.bin` 并写参，无需重新编译。

**评估标准**：
- ⭐⭐⭐ 能区分三层并给出案例；
- ⭐⭐ 能区分两层；
- ⭐ 完全不知道。

---

### 题目 5.2 prebuild 与 postbuild 的作用（★★★ / 概念理解）

**题目**：`prebuild.bat` 调用 `riscv32-elf-xmaker` 生成 `effect.c/xcfg.h`，`postbuild.bat` 调用 `objcopy` 生成 `app.dcf`。这两步的作用与必要性？

**考察点**：
- 主考点：构建流程；
- 副考点：xmaker 的角色；
- 区分点：能否说出 `effect.c/h` 是“生成物而非手写源码”。

**参考答案**：
- `prebuild.bat`：根据 `res.xm`、`xcfg.xm` 生成 `res.bin/h`、`xcfg.bin/h`，并把生成的 `effect.c/h` 复制到 `projects/microphone/`，让编译器引用最新的资源偏移；
- `postbuild.bat`：把 `app.rv32` 转成 `app.bin`，再用 xmaker 打包 `app.dcf`（含 app/res/xcfg/updater），供下载。

**评估标准**：
- ⭐⭐⭐ 能说出生成物与必要性；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 5.3 打开 ECHO 后的级联变化（★★★ / 源码追踪）

**题目**：当 `WIRELESS_MIC_ECHO_EN` 从 0 改为 1，编译期会发生什么级联变化？从 `config` → `modules/voice/echo.c` 的颗粒度。

**考察点**：
- 主考点：条件编译影响；
- 副考点：`config_extra.h` 的派生检查；
- 区分点：能否说出 ECHO 需要 `mram` 段与 delay buffer。

**参考答案**：
1. `config_ab5766_le_mic.h:101` 宏变为 1；
2. `config_extra.h` 中若 ECHO 与 Room Reverb 同时开，会强制ECHO delay=0；
3. `modules/voice/echo.c` 开始编译（`#if ECHO_EN`）；
4. `ram.ld` 中 `.mram_echo` 段生效，`ECHO_DELAY_BUF_SIZE` 占用 24K mram；
5. `mic_emit_init()` 调 `echo_audio_init()`，`mic_enc_proc_cb()` 调 `echo_audio_process()`。

**评估标准**：
- ⭐⭐⭐ 能列 4+ 步；
- ⭐⭐ 2-3 步；
- ⭐ 完全不知道。

---

### 题目 5.4 `AT > flash` 与 VMA 的链接器机制（★★★ / 源码追踪）

**题目**：`ram.ld` 中 `.code_emit` 节通过 `AT > flash` 把代码放 Flash，但 `> comm_emit` 把 VMA 放 RAM。请解释链接器如何同时满足两个地址空间。

**考察点**：
- 主考点：链接脚本的 VMA/LMA；
- 副考点：`AT()` 与 `>` 的区别；
- 区分点：能否说出“加载地址”与“运行地址”。

**参考答案**：
- `> comm_emit` 指定 VMA（运行时地址）；
- `AT > flash` 指定 LMA（加载地址）；
- 链接器生成 ELF 时，该 section 的 `sh_addr` 是 VMA，但 `program header` 的 `p_paddr` / `p_vaddr` 可不同；
- 启动代码（`wireless_mic_load_code()`）负责把 LMA 内容拷到 VMA。

**评估标准**：
- ⭐⭐⭐ 能区分 VMA/LMA；
- ⭐⭐ 能说出“拷贝”；
- ⭐ 完全不知道。

---

### 题目 5.5 `xcfg_cb` 与编译期宏的覆盖关系（★★★ / 源码追踪）

**题目**：`xcfg_cb` 的字段如何与编译期宏互相覆盖？比如 `xcfg_cb.buck_mode_en` 与 `config` 中的 `BUCK_MODE_EN` 宏。

**考察点**：
- 主考点：运行时参数与编译期宏的优先级；
- 副考点：`config_extra.h` 的强制覆盖；
- 区分点：能否说明通常编译期宏决定功能有无，xcfg 决定具体参数。

**参考答案**：
编译期宏决定“功能是否编译进来”；`xcfg_cb` 决定“运行时具体参数”。`config_extra.h` 中有些宏会强制覆盖（如 FOTA 未启用时强制 `AB_FOT_BLE_EN=0`）。`xcfg_cb.buck_mode_en` 不会覆盖 `BUCK_MODE_EN`，而是作为 PMU 配置的输入。

**评估标准**：
- ⭐⭐⭐ 能说明优先级与例外；
- ⭐⭐ 能说明一般规则；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 5.6 链接报 `comm_emit` overflow（★★★ / 调试排查）

**题目**：链接报 `"region 'comm_emit' overflowed by 512 bytes"`。请给出至少 4 个排查思路（含开 dump 看 section size、关某算法、工具链优化等级）。

**考察点**：
- 主考点：内存溢出排查；
- 副考点：`map.txt` 分析；
- 区分点：能否给出系统化排查。

**参考答案**：
1. 打开 `map.txt` 看 `.code_emit` / `.data_emit` 各 section 大小，定位超出的模块；
2. 关闭一个算法（如 AGC、Room Reverb）重编译看是否通过；
3. 检查工具链优化等级（`app.cbp` 中是否含 `-Os` 或 `-O2`），改小优化；
4. 检查是否有大数组未加 `AT()` 到正确段；
5. 检查 `config_extra.h` 是否意外打开了额外功能。

**评估标准**：
- ⭐⭐⭐ 4+ 条；
- ⭐⭐ 2-3 条；
- ⭐ 只说一句。

---

### 题目 5.7 ToolKit 在线调 EQ 重启后失效（★★★ / 调试排查）

**题目**：ToolKit 在线调 EQ 后返回 ACK，但重启后失效。为什么会这样？如何把在线参数"固化"进固件？

**考察点**：
- 主考点：在线调参 vs 离线固化；
- 副考点：`effect.bin` 的生成与烧录；
- 区分点：能否说明在线调参只改 RAM，不改 Flash。

**参考答案**：
在线调参通过 `toolkit_process()` 把参数写入 PACC RAM，立即生效但不写 Flash。重启后固件重新从 `effect.bin` 加载默认参数，所以失效。

固化方法：
1. ToolKit 保存当前参数为 `effect.bin`；
2. 重新执行 `prebuild.bat` 生成新的 `res.bin`；
3. 重新编译并烧录 `app.dcf`。

**评估标准**：
- ⭐⭐⭐ 能说明原因与固化步骤；
- ⭐⭐ 能说明原因；
- ⭐左右 完全不知道。

---

## 4. 设计权衡题

### 题目 5.8 为什么 xcfg 不打包进源码宏（★★★ / 设计权衡）

**题目**：为什么厂商不把 `xcfg` 直接打包进源码宏，而是单独走产测烧入？请从产线测试、客户定制、版本管理三个角度分析。

**考察点**：
- 主考点：产测与配置管理；
- 副考点：同一固件多客户；
- 区分点：能否说出“产线可不改代码只改配置”。

**参考答案**：
- **产线测试**：每张板可能需不同蓝牙地址/RF 参数，产线工具直接写 `xcfg.bin` 到参数区；
- **客户定制**：同一固件可给不同客户，只改 setting 生成不同 `xcfg.bin`；
- **版本管理**：代码与配置解耦，代码升级不影响客户参数，参数升级无需重新编译。

**评估标准**：
- ⭐⭐⭐ 三角度都能分析；
- ⭐⭐两 角度；
- ⭐ 只说一句。

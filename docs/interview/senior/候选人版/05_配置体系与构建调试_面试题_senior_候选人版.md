# 05 配置体系与构建调试 面试题（Senior · 候选人版）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 候选人版（先自行作答，无答案）

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风。
- 配置三层：`config.h`（选产品）→ `config_ab5766_le_mic.h`（编译宏）→ `config_extra.h`（派生）→ `xcfg.xm/bin/h`（运行时参数区）。
- 构建链：`app.cbp` → `prebuild.bat (xmaker 生成 res.bin/xcfg.bin/effect.c/h)` → 编译链接 → `postbuild.bat` → `app.dcf`。
- 本卷重点：`config_extra.h` 的派生职责、`app.xm` 打包结构、`AT > flash` 的链接器机制。

---

## 1. 概念理解题

### 题目 5.1 `config_extra.h` 的“派生配置”责任（★★★★ / 概念理解）

**题目**：`config_extra.h` 的作用是“把 `config_ab5766_le_mic.h` 中的宏做安全派生与校验”。请举 3 个例子，并说明如果没有 `config_extra.h` 会有什么风险。

**考察点**：
- 主考点：构建期依赖管理；
- 副考点：条件编译的冲突防范；
- 区分点：能否举出真实例子。

### 题目 5.2 `app.xm` 的打包结构（★★★★ / 概念理解）

**题目**：`app.xm` 定义了 DCF（下载包）的组成。请说明 `make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin, updater.bin)` 的每一部分作用。

**考察点**：
- 主考点：固件打包结构；
- 副考点：xmaker 语法；
- 区分点：能否指出各部分在运行时的作用。

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
- 区分点：能否指出 `> comm_emit` 决定 VMA，`AT > flash` 决定 LMA。

### 题目 5.4 `prebuild.bat` 失败后的连锁后果（★★★★ / 源码追踪）

**题目**：若 `prebuild.bat` 中 `copy effect.c ..\..\effect.c` 失败，后续编译会报什么错？请给出从 `app.cbp` 依赖到 `include.h` 再到 `toolkit.c` 的连锁反应。

**考察点**：
- 主考点：生成物在代码中的引用；
- 副考点：构建依赖链；
- 区分点：能否从“缺文件”→“缺宏”→“缺函数”推导。

## 3. 调试排查题

### 题目 5.5 链接后 RAM 溢出但 Flash 充足（★★★★★ / 调试排查）

**题目**：链接报 `region 'comm_adapter' overflowed by 1024 bytes`，但 Flash 还有 30K 剩余。请给出 5 个排查思路，并说明为什么不能“把代码从 Flash 移回 RAM” 的简单方案。

**考察点**：
- 主考点：RAM/Flash 权衡；
- 副考点：段归属、启动时间、功耗；
- 区分点：能否区分“代码可放 Flash 但 buffer/变量必须放 RAM”。

### 题目 5.6 ToolKit 连不上的“三层定位法”（★★★★ / 调试排查）

**题目**：BlueTrum ToolKit 连不上（串口 OK 无 response）。请用“串口 → HUART → 固件”三层定位。

**考察点**：
- 主考点：在线调参链路排查；
- 副考点：波特率、GPIO、`xcfg_cb.huart_en`；
- 区分点：能否从物理层→链路层→应用层排查。

## 4. 设计权衡题

### 题目 5.7 把 `xcfg` 改为宏编译 vs 参数区存储（★★★★★ / 设计权衡）

**题目**：若你负责 SDK 架构，请评估“把 `xcfg` 删除，全部改为编译期宏” 的利弊，并给出厂线测试的影响分析。

**考察点**：
- 主考点：配置架构选型；
- 副考点：产测/客户化/OTA 升级的灵活性；
- 区分点：能否从固件大小、产测灵活性、升级风险三方面对比。

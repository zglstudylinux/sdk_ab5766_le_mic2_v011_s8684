# CLAUDE.md

本文档为 Claude Code（claude.ai/code）在此仓库中工作时提供指引。

## 回复语言

- 与用户交流时，所有回复必须使用中文。此规则长期生效，适用于本项目今后的所有对话与回答，无论用户使用何种语言提问，均以中文回复。

## 项目概述

这是面向 AB5766 RISC-V 芯片的嵌入式固件，适用于可配置的低功耗无线麦克风/适配器。当前在 [projects/microphone/config.h](projects/microphone/config.h) 中选定的产品配置是 `CONFIG_AB5766_LE_MIC`。仓库的 README 仅标识 SDK，未提供额外的开发说明。

## 构建与生成产物

- 构建入口是 Code::Blocks 工程 [projects/microphone/app.cbp](projects/microphone/app.cbp)，其中仅定义 `Debug` 目标，并配置使用 `riscv32-v3` 编译器。在已配置厂商 RV32 环境的 Code::Blocks 中打开该工程并构建 `Debug`。
- 该目标使用 `-march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_xbs1`，链接 `libs/` 下预编译的平台、C 运行时和蓝牙协议栈静态库，并将 [projects/microphone/ram.ld](projects/microphone/ram.ld) 预处理为链接器输入。
- 构建输出位于 `projects/microphone/Output/`。常规固件产物包括 `Output/bin/app.rv32`、`app.bin`、`app.dcf`、`download.xm` 和 `map.txt`。
- 工程会在编译前执行 `Output/bin/prebuild.bat`。该脚本依赖 `riscv32-elf-xmaker` 生成 `res` 和 `xcfg` 产物，并将生成的 `effect.c`/`effect.h` 复制到工程目录。
- 工程会在链接后执行 `Output/bin/postbuild.bat`。该脚本调用 `riscv32-elf-objcopy` 和 `riscv32-elf-xmaker`，生成 `.bin`、`.dcf` 和下载镜像；如果存在 `C:\upload\upload.bat`，则以 `-D AB5766 app.dcf` 调用它；如果 RV32 工具链安装在预期的 Windows 路径，则生成反汇编列表。
- 已跟踪仓库中没有独立构建系统、lint 命令、测试框架或单测运行命令。应使用工程构建验证改动；影响硬件行为的改动还应通过相应的固件/硬件流程验证。

## 架构

- [projects/microphone/main.c](projects/microphone/main.c) 是固件入口。它记录复位/唤醒状态、输出版本和内存诊断信息，调用 `bsp_sys_init()`，随后进入不返回的 `func_run()` 状态机循环。
- [header/include.h](header/include.h) 是应用层聚合头文件，规定了包含和配置顺序：全局定义与生成配置（`xcfg.h`）、选定的产品配置、公开 SDK API、BSP、功能层、驱动层和模块层。
- [projects/microphone/config.h](projects/microphone/config.h) 用于选择三个产品配置之一。当前选定的 [projects/microphone/config_ab5766_le_mic.h](projects/microphone/config_ab5766_le_mic.h) 是主要的编译期功能和硬件策略入口，涵盖时钟、Flash 布局、外设、无线协议/音频设置、音效开关、USB 角色和按键行为。修改角色或启用功能时，保持相关宏定义一致。
- 运行时及设备特定值由 [bsp/bsp_sys.c](bsp/bsp_sys.c) 中的 `bsp_sys_init()` 调用 `xcfg_init()` 加载到 `xcfg_cb`。该初始化建立 PMU、无线状态、按键、LED、可选的充电/ADC/USB 功能、时钟、参数存储、定时器回调和可选的 EQ/DRC 调试能力。
- [functions/func.c](functions/func.c) 管理顶层运行模式选择。它通过 `wireless_role_is_adapter()` 决定适配器或麦克风发射端模式，再在持久运行的状态机中分派至 `func_adapter()` 或 `func_mic_emit()`。公共周期工作和全局消息分别由 `func_process()` 与 `func_message()` 处理。
- `driver/` 提供寄存器级外设驱动，例如 GPIO、时钟、ADC/DAC、定时器、UART、SPI/I2C/IIS、电源和按键。`bsp/` 将这些驱动组合为板级行为，例如系统启动、参数、按键、LED、充电、音频 I/O 和调试 UART。
- `modules/` 包含可复用的协议和信号处理单元，包括无线麦克风传输/控制、音频处理/EQ/DRC/增益、编解码器、语音效果、BLE profile 数据、FOTA 和调试工具。`functions/` 将用户行为和消息映射到这些模块。
- [projects/microphone/strong_symbol.c](projects/microphone/strong_symbol.c) 覆盖预编译 SDK 库中的弱函数，并提供平台回调。需要有意挂接库行为时，应在此处实现，而不是尝试修改预编译库。
- 链接段放置至关重要。应用、BSP 和功能层代码中的 `AT(...)` 标注，与预处理后的 [projects/microphone/ram.ld](projects/microphone/ram.ld) 共同协调 RAM/Flash 放置。移动时序或内存敏感代码时，保留这些标注。

## 改动定位

- 修改产品行为或功能开关时，先从选定配置 [projects/microphone/config_ab5766_le_mic.h](projects/microphone/config_ab5766_le_mic.h) 开始，再跟踪 BSP、功能层和模块实现中对应的 `#if` 分支。
- 修改无线麦克风行为时，从 `modules/wireless/` 开始，并沿当前顶层功能（`func_mic_emit` 或 `func_adapter`）追踪调用链。
- 将 `projects/microphone/Output/` 视为构建/资源输出目录。工程及脚本会消费其中的生成配置与资源文件；不要假定该目录中的每个文件都是手写源文件。

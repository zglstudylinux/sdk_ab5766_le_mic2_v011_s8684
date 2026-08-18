# CLAUDE.md

本文档为 Claude Code（claude.ai/code）在此仓库中工作时提供指引。

## 回复语言

- 与用户交流时，所有回复必须使用中文，且今后一律如此（包括解释、总结、报错说明、状态汇报、提问等所有面向用户的输出，均用中文，不混用英文成段）。
- 代码、宏名、函数名、文件路径、命令、日志原文等保持原样，不翻译；仅在需要解释其含义时用中文说明。

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

## 当前实测状态与已知限制

本节记录当前板卡实测得到、但无法从代码或 git 历史直接推断的事实。证据等级参照 [docs/SDK/AB5766_LE_Mic_音频算法测试快速入门.md](docs/SDK/AB5766_LE_Mic_音频算法测试快速入门.md) 第 1 节。修改这些路径前先确认本节约束没有被违反。

### 当前实测链路

- 已验证的声学链路：手机外放音乐 → TX 麦克风采集 → 专有无线链路 → RX → 耳机正常听音。这证明 TX 采集、无线传输、RX 解码、DAC/耳机输出路径基本可用。
- **只能证明以上路径**，不证明 USB Mic、不证明算法客观指标（RMS/频谱/失真）已通过。耳机听音正常 ≠ USB Mic 正常。

### 已完成测试

- **静音**：通过按键/`MSG_VOL_MUTE` 触发，听感上静音与恢复正常。
- **Soft Gain 等级与渐变**：等级升降与渐变在听感上可辨识。
- **MAGIC**：切换档位在听感上可辨识。
- 上述三项当前只到「听感证据 + 控制面日志证据」等级，未量化 RMS/频谱。不要把控制面日志或听感写成「算法客观指标已通过」。

### 当前板卡按键硬件

- 当前开发板只有 `KEY_ID_PP`、`KEY_ID_K1`、`KEY_ID_K2` 三个有效按键，无 `KEY_ID_K3`。
- 项目未定义 `WIRELESS_MIC_K12_KEY_EN`，因此编译使用 [projects/microphone/port/port_key.c](projects/microphone/port/port_key.c) 的非 K12 按键表。
- **MAGIC 入口**：非 K12 表中 `KEY_ID_K2` 的「长按抬起」列（事件索引 3，`KEY_LONG_UP`，即长按后松手触发）映射到 `MSG_CHANGE_MAGIC`。注意是松手时触发，不是按下时、也不是双击。
- K12 配置下才是 K3 单击切换 MAGIC；当前板卡不适用 K3 描述。修改或引用按键入口时按当前非 K12 表为准。
- **按键消息表枚举值上限**：`mic_emit_key_msg_tbl` 的元素类型是 `u8`，[bsp/bsp_key.c](bsp/bsp_key.c) 的 `key_set_msg_tbl` 按字节 `memcpy` 拷贝，`bsp_key_get_msg` 按 `u8` 取值返回。因此经按键表分派的消息，其枚举值必须 < 0x100（0~255），否则写入/读取被截断为低 8 位，悄无声息变成 `MSG_NO`。放在 `MSG_SYS_MAX(0x7ff)` 之后的 `EVT_*` 类消息（如原 `EVT_TEST_X=0x800`）不可放入按键表。实测踩坑记录见 [docs/SDK/AB5766_嵌入式固件SDK_架构导览.md](docs/SDK/AB5766_嵌入式固件SDK_架构导览.md) 第 22 章。

### USB Mic 已知限制

- USB Mic 是**电脑采集输入**方向（RX 作为 USB Audio 麦克风端点，供电脑录音），不是 USB 播放输出方向。不要期待从 USB 播放到耳机。
- `ADAPTER_USB_MIC_RX_EN=1` 只是编译期开关。实际运行还需 `xcfg_cb.wireless_adapter_en` 成立（进入 Adapter/RX 角色）并完成 USB 枚举，见 [bsp/bsp_sys.c](bsp/bsp_sys.c) 中 `dev_init(1)` 的条件。
- 当前用户 USB Mic **尚未通过**。后续算法测试先用 RX 耳机完成，USB Mic 故障排查放到算法测试之后，作为独立步骤；不在算法 A/B 中改动 USB 相关宏。
- 排查 USB Mic 时按「角色 → 枚举 → PCM → 电脑端」分层收集证据，不得未经证据断定根因。

### 测试纪律与算法边界

- 每次算法测试只改变一个宏或一个参数，独立 Debug 构建/刷写/恢复，保留 `config_diff.txt`、构建日志、`map.txt`、UART、录音。
- Dump（`WIRELESS_DUMP_EN`）需要时单独建诊断构建，测完恢复 0，不影响实时音频基线。
- Room Reverb 与 ECHO 同时开启时，源码中 Room Reverb 会被自动 mute（见 [modules/voice/room_reverb.c](modules/voice/room_reverb.c)）；测试 Room Reverb 必须先确认 ECHO 关闭。
- AGC 当前源码标注「调试中」，可能存在实现/链接风险，应放在算法测试最后，先做构建/链接验证再决定是否刷写。
- 预编译库核心（LC3S、PLC、PACC 内核、AGC 内部阈值等）只记录公开 API、包装层、调用时机和输入输出，不从函数名推断内部数学实现。

## 改动定位

- 修改产品行为或功能开关时，先从选定配置 [projects/microphone/config_ab5766_le_mic.h](projects/microphone/config_ab5766_le_mic.h) 开始，再跟踪 BSP、功能层和模块实现中对应的 `#if` 分支。
- 修改无线麦克风行为时，从 `modules/wireless/` 开始，并沿当前顶层功能（`func_mic_emit` 或 `func_adapter`）追踪调用链。
- 将 `projects/microphone/Output/` 视为构建/资源输出目录。工程及脚本会消费其中的生成配置与资源文件；不要假定该目录中的每个文件都是手写源文件。

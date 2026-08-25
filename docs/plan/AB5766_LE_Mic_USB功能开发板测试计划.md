# AB5766 LE Mic USB 功能开发板测试计划

> 本文是 USB Mic 开发板实测的**执行计划与证据记录**。稳定源码调用关系与 USB 全链路知识见 [../SDK/AB5766_USB_设备与USB_Mic_全链路知识详解.md](../SDK/AB5766_USB_设备与USB_Mic_全链路知识详解.md)。测完按本文逐步执行，并把每层结果填入第 8 节证据记录。

## 1. 目标与范围

### 1.1 目标

在开发板上验证 USB Mic 上行链路：

```
RX Adapter 解码 PCM → usb_mic_in_audio_input → USB Audio 麦克风端点 → 电脑识别并录音
```

### 1.2 范围边界

| 项目 | 是否测试 | 说明 |
|---|---|---|
| USB Mic（电脑采集输入） | ✅ 测 | `ADAPTER_USB_MIC_RX_EN` |
| USB Speaker（下行播放） | ❌ 不测 | `ADAPTER_USB_SPK_TX_EN=0`，注释「不支持」 |
| USB Storage / HID | ❌ 不测 | `UDE_STORAGE_EN=0`、`UDE_HID_EN=0` |
| 耳机/DAC 输出 | ✅ 佐证 | `ADAPTER_DAC_OUTPUT_EN=1`，作为「PCM 有数据」的同源佐证 |

### 1.3 成功标准（分层递进，每层通过才进入下一层）

1. **角色层**：RX 板 UART 打印 `func_adapter`（不是 `func_mic_emit`）。
2. **枚举层**：插拔 USB 打印 `EVT_PC_INSERT` / `EVT_PC_REMOVE`，且电脑设备管理器出现 RX 的 USB 音频**输入**设备。
3. **PCM 层**：TX→RX 无线已连接且解码有数据（RX 耳机有声作为同源佐证）。
4. **电脑端**：电脑录音软件选 RX USB Mic 录到有效 PCM（非全 0）。

## 2. 前置事实（必须先纠正的过时信息）

> 以下结论均以**当前源码实测为准**，不依赖命名猜测。

1. **当前 USB 全关**：`ADAPTER_USB_SPK_TX_EN=0`（[config:124](../../projects/microphone/config_ab5766_le_mic.h#L124)）、`ADAPTER_USB_MIC_RX_EN=0`（[config:125](../../projects/microphone/config_ab5766_le_mic.h#L125)），四个 `UDE_*_EN` 全为 0，`UDE_ENUM_TYPE=0`（[config:178-182](../../projects/microphone/config_ab5766_le_mic.h#L178-L182)）。
2. **仓库多处文档过时**：`CLAUDE.md`、`docs/SDK/AB5766_LE_Mic_音频算法测试快速入门.md` 第 141/488/863 行、`docs/SDK/算法详解/14_USB_Mic枚举链.md` 均写 `ADAPTER_USB_MIC_RX_EN=1`，**与代码（=0）不符**。本计划以代码为准。
3. **打开 USB Mic 只需改一个宏**：`ADAPTER_USB_MIC_RX_EN 0→1`（[config:125](../../projects/microphone/config_ab5766_le_mic.h#L125)）。派生宏自动生效：`UDE_MIC_EN=ADAPTER_USB_MIC_RX_EN=1`（[config:181](../../projects/microphone/config_ab5766_le_mic.h#L181)）、`UDE_ENUM_TYPE=0x08`（仅 MIC 枚举，[config:182](../../projects/microphone/config_ab5766_le_mic.h#L182)）。
4. **方向**：USB Mic 是**电脑采集输入**方向（RX 作为 USB Audio 麦克风端点供电脑录音），**不是** USB 播放到耳机。USB 线接 **RX Adapter 板**，不是 TX 板。
5. **USB 功能代码当前被空 stub 覆盖**：EN=0 时 `strong_ble.c:211-218` 用 6 个空函数覆盖预编译库的 USB 入口（`ude_control_flow`/`ude_ep_reset`/`ude_isoc_tx_process`/`usb_isr` 等）。但 `WIRELESS_MIC_32K_EN=1` 使 `pcm_48k`（定义在预编译 `usb_device_mic_in.o`）仍被链接，因此 USB 模块的**数据段**已在镜像里，只是**功能代码段**被 GC 丢弃。EN=1 后 stub 不再编译，USB 功能代码真正链接进来。

## 3. 测试环境与硬件准备

| 项目 | 要求 | 依据 |
|---|---|---|
| 板卡 | TX 板 + RX 板各一块 | — |
| RX 角色配置 | `wireless_adapter.setting`：`wireless_adapter_en=True`、`wireless_mic_emit_en=False`（[wireless_adapter.setting:20-21](../../projects/microphone/Output/bin/Settings/wireless_adapter.setting#L20-L21)） | 双板测试指南 |
| 调试串口 | RX 板 **PA4** 单线 UART0，波特率 **1500000**、8N1、无流控；USB-TTL RX 接 PA4 + 共地 | [config:24](../../projects/microphone/config_ab5766_le_mic.h#L24) |
| USB 线 | 数据线接 **RX 板** USB 口（DM/DP） | 仓库无板级原理图，接口位置由用户按板卡确认 |
| 电脑 | 设备管理器 + 录音软件（Audacity/系统录音机，48kHz/单声道/关软件降噪/AGC/回声消除） | — |
| 声源 | 对 TX 麦克风说话或播放固定音 | — |

> 烧录方式：仓库无烧录器/接线/下载模式资料，`C:\upload\upload.bat` 不存在。**刷写由用户按厂商下载流程操作**；本计划负责改宏、构建、map 分析、日志解读与证据记录。

## 4. 分层通过准则（角色 → 枚举 → PCM → 电脑端）

| 层 | 通过判据 | 失败现象与排查方向 |
|---|---|---|
| L1 角色 | UART 打印 `func_adapter` | 打印 `func_mic_emit` → RX 板角色配置错，需烧 `wireless_adapter` |
| L2 枚举 | 插拔 USB 打印 `EVT_PC_INSERT`/`EVT_PC_REMOVE`（[msg_adapter.c:19/25](../../functions/msg_adapter.c#L19)）；设备管理器出现 USB 音频输入设备 | 无 `EVT_PC_INSERT` → `usb_detect` 未运行（角色不对/USB 物理层/线缆）；有打印但设备管理器无设备 → 枚举未完成（`UDE_ENUM_TYPE`/描述符） |
| L3 PCM | TX 已连 RX，RX 耳机有声（`dac0_out_audio_input` 同源，[mic_proc.c:466](../../modules/wireless/mic_proc.c#L466)） | 耳机无声 → 先解决无线/解码，USB 层无法单独成立 |
| L4 电脑端 | 电脑录音软件从 RX USB Mic 录到有效 PCM（非全 0） | 能枚举但录不到 → 端点/采样率/电脑默认设备/软件降噪，需逐项排除 |

## 5. 改动清单与 Flash 风险

### 5.1 改动

| 文件 | 宏 | 改动 | 说明 |
|---|---|---|---|
| [config_ab5766_le_mic.h:125](../../projects/microphone/config_ab5766_le_mic.h#L125) | `ADAPTER_USB_MIC_RX_EN` | `0 → 1` | 唯一必需改动 |

派生宏（自动，无需手改）：`UDE_MIC_EN=1`、`UDE_ENUM_TYPE=0x08`。

### 5.2 Flash 风险（必须先处理）

上次 `build.ps1 -Rebuild` 已 `CODE SIZE: 120 KB`（Flash 顶满）。打开 USB 会新增 `usb_device_run_loop/ep/enum/comm/device/table/audio` 的函数段（估算 6~8 KB），**大概率溢出**。因此：

- **步骤 0 先只改 EN=1 构建一次**，观察是否溢出。
- **溢出则按以下优先级关宏腾空间**（每一步独立构建 + 记录 `config_diff.txt`）：

| 优先级 | 宏 | 当前值 | 改为 | 影响 |
|---|---|---|---|---|
| 1 | `EFFECT_DBG_ADJUST_IN_UART`（[config:189](../../projects/microphone/config_ab5766_le_mic.h#L189)） | 1 | 0 | 关 UART 在线调音效（依赖 2/3） |
| 2 | `EFFECT_DBG_ADJUST_EN`（[config:188](../../projects/microphone/config_ab5766_le_mic.h#L188)） | 1 | 0 | 关音效离线调试 |
| 3 | `EQ_DRC_DBG_IN_UART`（[config:187](../../projects/microphone/config_ab5766_le_mic.h#L187)） | 1 | 0 | 关 UART 在线调 EQ |
| 4 | `ADAPTER_MIX_DRC_EN`（[config:126](../../projects/microphone/config_ab5766_le_mic.h#L126)） | 1 | 0 | 关一拖二混音 DRC（单路收听不受影响） |
| 5（最后） | `WIRELESS_MIC_DAC_OUT_EN`（[config:95](../../projects/microphone/config_ab5766_le_mic.h#L95)） | 1 | 0 | 关 TX 本地监听（非首选，影响 TX 端） |

> 原则：优先关「调试类」宏（1~3），它们不影响 USB 功能本身；`ADAPTER_MIX_DRC_EN` 与 `WIRELESS_MIC_DAC_OUT_EN` 是最后手段。

**实际执行结果（已落地，详见 [config_diff.txt](config_diff.txt)）**：仅改 EN=1 → 124 KB 溢出；依次关优先级 1~3（三调试宏，省 4.26 KB）和优先级 4（`ADAPTER_MIX_DRC_EN`，省 1.15 KB）后仍 124 KB 溢出；`WIRELESS_MIC_DAC_OUT_EN` 因 `dac0_out` 仍被 RX 端 `ADAPTER_DAC_OUTPUT_EN` 引用而省不出空间，**最终额外关 `WIRELESS_MIC_AINS4_32K_EN`（TX 端 AINS4 降噪，约 12 KB，与 RX 端 USB 链路无关）才降到 112 KB 构建成功**。即当前固件共改动 6 个宏（EN=1 + 腾空间 5 个），另把 `BSP_UART_DEBUG_EN` 从 PB3 改到 PA4。

## 6. 分步执行步骤

### 步骤 0：改宏 + 构建 + map 验证（纯软件，不需硬件）

1. 备份当前 config，改 `ADAPTER_USB_MIC_RX_EN 0→1`，写 `docs/plan/config_diff.txt`。
2. `powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1 -Rebuild`。
3. 若 Flash 溢出（`Code [0x0-0x1EFFF] ... overlap` 或 CODE SIZE > 120 KB），按 5.2 优先级关宏，重新构建直到通过。
4. 核对新 `map.txt`：
   - **出现**：`usb_device_enter`、`usb_device_process`、`usbchk_connect`、`usb_detect`、`usb_mic_in_audio_input` 的最终符号地址（应用层调用点编译 + 预编译实现被拉入）。
   - **消失**：`strong_ble.o` 的 6 个 USB 空 stub（`#if !ADAPTER_USB_MIC_RX_EN` 不再成立）。
   - 记录 CODE SIZE、USB 段大小、构建产物哈希。
5. 保留 EN=1 固件用于步骤 1~4；**不要**在步骤 1~4 之间改其他宏。

### 步骤 1：角色层验证（需用户刷写 + 观察）

1. 用户刷 RX 板 EN=1 固件 + `wireless_adapter` setting；TX 板沿用现有固件/setting。
2. RX 板串口（PA4/1500000/8N1）上电观察，确认打印 `func_adapter`（非 `func_mic_emit`）。
3. 记录启动日志（保存到证据目录）。

### 步骤 2：枚举层验证（需用户操作 + 观察）

1. USB 数据线接 RX 板 USB 口、另一头接电脑。
2. 观察 RX UART：插上应打印 `EVT_PC_INSERT`，拔下应打印 `EVT_PC_REMOVE`。
3. 电脑设备管理器 →「音频输入和输出」/「声音、视频和游戏控制器」确认出现 RX 的 USB 音频**输入**设备。
4. 记录 UART 与设备管理器截图。

### 步骤 3：PCM 层验证（需用户操作）

1. TX 上电并与 RX 建立无线连接（两端 `WIRELESS_CONNECTED, 0`）。
2. 对 TX 麦克风说话，确认 RX 耳机有声（`ADAPTER_DAC_OUTPUT_EN=1` 的 DAC 同源输出，[mic_proc.c:466](../../modules/wireless/mic_proc.c#L466)）。
3. 记录两端连接日志与听感。

### 步骤 4：电脑端录音验证（需用户操作）

1. 电脑录音软件：输入设备选 RX 的 USB Audio 设备，采样率 48kHz、单声道，关闭软件降噪/AGC/回声消除。
2. 对 TX 麦克风说话或播放固定音，录音约 10~30 秒。
3. 回放/看波形确认录到有效 PCM（非全 0、有语音包络）。
4. 保存录音文件到证据目录。

## 7. 回滚方案

全部测完后（无论结果如何）：

1. 恢复 `ADAPTER_USB_MIC_RX_EN 1→0`。
2. 恢复步骤 0 中为腾空间而关闭的所有宏为原值。
3. 重新 `build.ps1 -Rebuild`，确认回到 `CODE SIZE: 120 KB` 基线，map.txt 中 USB 函数段恢复为 `0x00000000`（stub 覆盖）。
4. 更新 `config_diff.txt` 标记已回滚。

## 8. 证据记录模板（每层一次）

复用算法测试执行记录的 E0~E3 分级：

| 级别 | 需要保存的证据 |
|---|---|
| E0 | `config.h` 产品选择、USB 相关宏值、TX/RX setting 或 xcfg 快照 |
| E1 | 宏 → 初始化 → 帧处理 → 输出的真实源码调用关系（文件/函数/行号） |
| E2 | 当次 map.txt（USB 符号/段）、`app.rv32`/`app.bin`/`app.dcf` 大小与哈希 |
| E3 | UART 原始日志、设备管理器截图、录音文件、听感记录 |

每层结论只能写 `Pass` / `Fail` / `Blocked` / `Inconclusive`；证据不齐写 `Blocked`，不臆断根因。

```markdown
### 步骤 N：<层名>
- **日期与操作者：**
- **结论：** Pass / Fail / Blocked / Inconclusive
- **E0 宏/配置：**
- **E1 源码链：**
- **E2 构建产物身份：**
- **E3 现象与证据：**（UART 原文 / 截图 / 录音路径）
- **与通过准则对照：**
- **未决问题 / 下一步：**
```

## 9. 风险与注意事项

| 风险 | 影响 | 缓解 |
|---|---|---|
| Flash 溢出 | EN=1 构建失败，无法出固件 | 5.2 优先级腾空间，每步独立构建 + config_diff.txt |
| 角色错（RX 跑成 TX） | USB 永不枚举 | 步骤 1 先确认 `func_adapter`，再测枚举 |
| USB 线接错板 | 插拔无事件 | 明确 USB 接 RX 板；检查线缆/接口 |
| PB3/PB4 与 USB DP/DM 冲突 | 调试串口占 PB3 会与 USB 数据线冲突、USB 枚举异常 | 调试串口已改 PA4（config:24）；PB3/PB4 留给 USB DP/DM |
| 电脑端软件降噪/重采样 | 录音失真/误判 | 录音软件关降噪/AGC/回声消除，48kHz 单声道 |
| 耳机有声 ≠ USB 录音成功 | 把 DAC 佐证误当成 USB 通过 | 严格按四层判据，L3 只是 PCM 佐证，L4 才是 USB 验收 |
| 烧录器/下载模式未知 | 无法刷写 | 仓库无资料，由用户按厂商流程操作，本计划不臆造步骤 |

## 10. 结论边界

- 本计划只验证「USB Mic 能否被电脑识别并录到声音」这一功能层事实，不量化 RMS/频谱/失真等客观指标（量化属后续算法测试范围）。
- 预编译库内部 USB 协议/描述符实现不可见，文档只记录符号、段、调用点、时机与实测现象，不推断内部细节。

## 11. 实测结果与证据记录（2026-04-27）

### 步骤 0：构建 + map 验证 —— `Pass`

- **E0**：宏改动见 [config_diff.txt](config_diff.txt)（`ADAPTER_USB_MIC_RX_EN=1` + 腾 Flash 5 个 + 串口 PA4）。
- **E2**：`app.dcf` SHA256 `A88FCAAD…`，`CODE SIZE: 112 KB`；map.txt 中 `usb_device_enter/usb_device_process/usbchk_connect/usb_detect/usb_mic_in_audio_input` 等 USB 符号均有真实地址（来自 `libplatform.a`），`strong_ble.o` 的 6 个 USB 空 stub 消失。

### 步骤 1：角色层 —— `Pass`

- RX 板烧 `app.dcf` + `wireless_adapter` setting，进入 `func_adapter` 角色。

### 步骤 2：枚举层 —— `Pass`

- 插拔 USB 打印 `EVT_PC_INSERT` / `EVT_PC_REMOVE`（事件链路 `usb_detect→usbchk_connect→dev_online_filter→msg_enqueue(EVT_PC_INSERT)→usb_device_enter(UDE_ENUM_TYPE)` 实际跑通）。
- Windows 设备管理器出现 USB 音频输入设备。

### 步骤 3：PCM 层 —— `Pass`

- TX→RX 无线连接，RX 解码有数据（耳机有声作为同源佐证）。

### 步骤 4：电脑端 —— `Pass`

- 电脑录音软件从 RX USB Mic 录到有效声音：**发送端麦克风声音经无线链路到接收端，再经 USB 传到电脑**。

### 关键现象记录

- **乱码根因**：调试串口 USB-TTL 只接了 PA4 信号线、未接 GND 共地导致乱码；**接 GND 后拔插打印正常**。并非时钟问题，也不影响 USB 功能。

### 采样率 / 位宽 / 通道确认 —— `Pass`

- 电脑录音设备实测：**1 通道 / 16 位 / 48 kHz**，与固件配置一致。
- 决定链（详见 [../SDK/AB5766_USB_设备与USB_Mic_全链路知识详解.md](../SDK/AB5766_USB_设备与USB_Mic_全链路知识详解.md) 第 16 节）：
  - **48 kHz**：`WIRELESS_MIC_SAMPLE_RATE_SELECT=SAMPLE_RATE_48K`（[config:79](../../projects/microphone/config_ab5766_le_mic.h#L79)）决定无线 PCM 链路采样率；TX 端因 `WIRELESS_MIC_32K_EN=1` 以 32 kHz 采集（[bsp_sdadc.c:71-75](../../bsp/bsp_sdadc.c#L71-L75)），再经 SRC 升采样到 48 kHz（[mic_proc.c:305-308](../../modules/wireless/mic_proc.c#L305-L308)、`src_init(0,32000,48000)` [mic_proc.c:629](../../modules/wireless/mic_proc.c#L629)）。
  - **16 位**：PCM 数据类型为 `s16`（`pcm_48k`、`obuf` 均为 s16 数组）。
  - **1 通道**：`WIRELESS_MIC_CHANNEL_SELECT=1`（[config:81](../../projects/microphone/config_ab5766_le_mic.h#L81)，注释「目前只支持单声道」）。

### 稳定性：多次拔插 —— `Pass`

- 连续多次拔插 USB，每次均稳定重枚举（`EVT_PC_REMOVE`/`EVT_PC_INSERT` 交替打印），录音正常恢复。

### 配置去向：方案 A（保留 USB Mic）

- 决定**保留 USB Mic**（`ADAPTER_USB_MIC_RX_EN=1`），不回滚到 EN=0。
- 注意：当前为腾 Flash 关闭了 `WIRELESS_MIC_AINS4_32K_EN`（TX 降噪）、`ADAPTER_MIX_DRC_EN`、三个调音调试宏（`EQ_DRC_DBG_IN_UART`/`EFFECT_DBG_ADJUST_EN`/`EFFECT_DBG_ADJUST_IN_UART`）。若产品需 AINS4 与 USB 同时开启，需更大 Flash 型号或功能裁剪决策。

### 结论

USB Mic 上行链路（TX 采集 → 无线 → RX 解码 → USB Audio → 电脑录音）**功能验证通过**，四层判据全部满足；采样率 48 kHz/16 bit/1 ch 与配置一致；多次拔插稳定；配置采用方案 A 保留。

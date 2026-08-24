# BlueTrum ToolKit 上位机使用与调参指南（AB5766 LE Mic）

> 本文面向已在 AB5766 LE Mic 工程上配置/调试音频链路的工程师。
>
> 本文聚焦三件事：
> 1. **BlueTrum ToolKit 是什么**：界面结构、与固件的关系；
> 2. **如何接入与联调**：串口、波特率、GPIO、固件开关；
> 3. **参数怎么调**：EQ、DRC、ECHO、MAGIC、MIX、Soft Gain 等控件含义与映射关系。
>
> 约束：本文只基于本仓库可读源码和公开头文件描述接口、数据流与调用时机；预编译库内核（PACC、LC3S、PLC、AINS4、EQ/DRC 系数计算等）**不推测内部数学实现**，仅记录其 API 边界与调用时机。

---

## 1. ToolKit 在工程中的定位

BlueTrum ToolKit 是厂商提供的 **上位机调参/下载工具**，通过 UART（HSUART）与 AB5766 固件交互，实现两类事情：

1. **在线调参（Online Debug）**：实时下发 EQ/DRC/ECHO/MAGIC/MIX 等参数，不等下次重启即可生效；
2. **离线固化（Offline Update）**：把调好的参数打包进 `effect.bin`，再经 `res.xm` / `prebuild.bat` 固化进固件下载镜像，重新烧录后作为默认参数。

本工程当前 **已打开** ToolKit 在线调参开关，配置在 [config_ab5766_le_mic.h:185`:

```c
#define EQ_DRC_DBG_IN_UART          1   // 使能 UART 在线调节 EQ
#define EFFECT_DBG_ADJUST_EN        1   // 使能音效离线调试
#define EFFECT_DBG_ADJUST_IN_UART   1   // 使能 UART 在线调试音效
```

这三个宏共同决定了 ToolKit 与固件的联通能力；若任何一个改为 0，对应通道会被裁掉。

### 1.1 固件侧相关文件

| 路径 | 职责 |
|---|---|
| `modules/tool/toolkit.h` | 协议帧头、命令字、CRC 等定义；`toolkit_process()` 声明 |
| `modules/tool/toolkit.c` | 在线调参主处理：UART 数据解析、模块回调分发、ACK 应答 |
| `modules/tool/toolkit_effect.c` | 各效果模块（MIC_EQ/MIC_DRC/ECHO/MAGIC/MIX_EQ/MIX_DRC/USB 等）的在线参数更新 callback |
| `modules/tool/toolkit_effect.h` | 上位机下发的参数结构体定义（`tool_eq_coef_t`、`tool_drc_coef_t`、`tool_echo_pitch_coef_t`、`tool_magic_pitch_coef_t` 等） |
| `modules/audio/eq_drc_dbg_in_uart.c/h` | ToolKit UART 初始化、中断服务、ACK 发送 |
| `bsp/bsp_huart.c/h` | HSUART 外设驱动与 GPIO 映射 |
| `bsp/bsp_sys.c` | `xbsp_sys_init()` 中调 `eq_drc_dbg_init()`（[258-262](../bsp/bsp_sys.c#L258-L262)） |
| `functions/func.c` | 主循环收到 `EVT_ONLINE_SET_EFFECT` 消息时调 `toolkit_process()`（[98-102](../functions/func.c#L98-L102)） |

### 1.2 ToolKit 工作区文件

| 路径 | 说明 |
|---|---|
| `Output/bin/effect.bsln` | ToolKit 工作区文件（`.bsln`），记录固件路径 `app.dcf` 并关联 `effect.bts` |
| `Output/bin/effect.bts` | ToolKit 布局/连线文件（`.bts`），保存你截图里的模块图、参数界面状态 |
| `Output/bin/effect.layout` | ToolKit 布局元数据 XML |
| `Output/bin/effect.c` / `Output/bin/effect.h` | **由 `riscv32-elf-xmaker` 在 prebuild 阶段自动生成**，把 ToolKit 中的效果参数映射成 C 数组/偏移宏，供固件编译时引用 |
| `Output/bin/res/effect/effect.bin` | prebuild 时打包进 `res.bin` 的效果参数二进制 |

> 注意：`effect.c/h` 是 **prebuild 产物**，不要在 `Output/bin/` 手工改；要持久改参数，要么在 ToolKit 里改后重新 prebuild，要么直接改 `effect.c/h` 源目录（`projects/microphone/` 下的那份）并重新构建。

---

## 2. 硬件与固件准备

### 2.1 串口与 GPIO

- **波特率**：固定 `1500000`（1.5 Mbps），在 `eq_drc_dbg_in_uart.c:43` 中配置；
- **GPIO**：使用 `xcfg_cb.huart_io_sel` 指定的 IO（单线半双工 `HSUT_ONELINE_EN`），默认映射见 `bsp_huart.c`、`eq_drc_dbg_in_uart.c`；
- **协议特征**：HSUART DMA 收发、1500000 波特率、带 CRC16 校验的帧结构；
- **触发**：ToolKit 下发以 `"CFG"` 开头的指令帧。

### 2.2 固件侧必须满足的开关

在线调参能跑通，需要同时满足：

1. `EQ_DRC_DBG_IN_UART = 1`（[config_ab5766_le_mic.h:185](../../projects/microphone/config_ab5766_le_mic.h#L185)）；
2. `EFFECT_DBG_ADJUST_EN = 1`（[config_ab5766_le_mic.h:186](../../projects/microphone/config_ab5766_le_mic.h#L186)）；
3. `EFFECT_DBG_ADJUST_IN_UART = 1`（[config_ab5766_le_mic.h:187](../../projects/microphone/config_ab5766_le_mic.h#L187)）；
4. `xcfg_cb.huart_en` 为 true（在对应的 `.setting` 文件中配置，如 `wireless_mic_emit.setting` / `wireless_adapter.setting` 中 `huart_en`）；
5. 该板卡有对应角色的算法模块被编译进去（如调 TX 的 MIC_EQ/MIC_DRC，需要 `WIRELESS_MIC_EQ_DRC_EN=1`）。

### 2.3 当前角色与可调模块映射

| 板卡角色 | 当前宏 | ToolKit 中可调模块 |
|---|---|---|
| TX（发射端，`wireless_mic_emit_en=True`） | `WIRELESS_MIC_EQ_DRC_EN=1`，`WIRELESS_MIC_SOFT_GAIN_EN=1`，`WIRELESS_MIC_ECHO_EN=0`，`WIRELESS_MIC_MAGIC_EN=0`，`WIRELESS_FREQ_SHIFT2_EN=0` | `MIC_EQ`、`MIC_DRC`、`SOFT_GAIN`（Soft Gain 通过本地按键/寄存器调，ToolKit 可发命令改等级） |
| RX（适配器，`wireless_adapter_en=True`） | `ADAPTER_MIX_DRC_EN=1`，`ADAPTER_USB_MIC_RX_EN=0` | `MIX_EQ`、`MIX_DRC` |
| 临时调 ECHO/MAGIC | 需把对应宏改 1 构建后 | `ECHO`、`MAGIC` 才会出现在 `effect_info` 并被回调 |

> 当前截图里 `ECHO`、`MAGIC` 显示灰色（未选中），是因为当前编译配置里它们为 0（或未被使能），`effect_update_callback_tbl` 对应回调存在但在 `mic_effect` / `room_reverb` / `magic` 中未被编译链接进运行链。

---

## 3. ToolKit 界面结构与连线图含义

从你贴的截图看，ToolKit 主界面是一个 **音频链路拓扑图**，用图形化方式把「输入源 → 算法模块 → 输出源」连起来。当前布局中：

- **`MIC0/1`**：本地麦克风输入（TX 侧本地采集）；
- **`MIC_EQ`**：TX 的本地麦克风 EQ；
- **`MIC_DRC`**：TX 的本地麦克风 DRC；
- **`ECHO` / `MAGIC`**：TX 的回声/魔音（当前灰色，未启用）；
- **`TX_MIC0/1`**：TX 编码/发送后写入无线链路的输出端；
- **`RX_MIC0` / `RX_MIC1`**：RX 侧两路无线麦克风解码输入；
- **`MIX`**：混音（把两路 RX 混成一路）；
- **`MIX_EQ` / `MIX_DRC`**：混音后的 EQ/DRC；
- **`DAC0`**：DAC 输出（耳机/扬声器）；
- **`USB_TX`**：USB Audio 上行（作为 USB Mic 给电脑录音）。

这个连线和 `modules/effect/mic_effect.c` 中的 PACC CS 链表一一对应：

- TX 链：`PACC_EQ → PACC_DRC`（`loc_mic_pacc`）；
- RX 链：`PACC_EQ → PACC_DRC`（`mix_pacc`）。

---

## 4. 参数详解与固件结构体映射

ToolKit 每改一个参数，都会通过 UART 下发一个参数帧；帧中参数结构与固件 `modules/tool/toolkit_effect.h` 中的 packed struct 一一对应。

### 4.1 EQ（MIC_EQ / MIX_EQ）

结构体：`tool_eq_coef_t`（[toolkit_effect.h:38-52](../../modules/tool/toolkit_effect.h#L38-L52)）

```c
typedef struct __attribute__((packed)){
    int8_t  channel;        // 通道
    int8_t  enable;         // 0 关 / 1 开
    int8_t  sample_rate;    // 采样率标识
    int8_t  max_bandnum;    // 本次下发实际band数
    int8_t  param_size;
    int8_t  param_cnt;
    int8_t  band_value_start;
    int8_t  band_value_cnt;
    int8_t  reverse : 1;    // 反相
    int16_t RSVD    : 15;
    int32_t gain;           // 总增益（dB 转 Q 格式）
    int32_t coef[6][5];     // 每 band 5个系数
} tool_eq_coef_t;
```

| 上位机控件 | 固件字段 | 说明 |
|---|---|---|
| 使能开关 | `enable` | 决定 callback 里是否真正把参数写进 PACC；为 0 时上位机仍会发参数但 `toolkit_ack_fail()` 或不生效 |
| 采样率 | `sample_rate` | 决定系数换算（16k/32k/44.1k/48k 等） |
| band 数 | `max_bandnum` / `param_cnt` | `pacc_eq_set_by_param(cs, band_cnt, eq_param)` 的 `band_cnt` |
| 类型（Peak / LowShelf / HighShelf / LowPass / HighPass / Notch / Disable） | `coef[band][*]` 中的系数 | 决定每条 biquad 的 `b0,b1,b2,a1,a2` |
| Freq | 每 band 的中心频率/截止频率 | 参与系数换算 |
| Gain | 每 band 的增益（dB） | Peak/Shelf 才有意义 |
| Q | 每 band 的 Q 值 | 决定带宽 |
| 总增益 | `gain` | 整个 EQ 的 post-gain（Q 格式） |
| 反相 | `reverse` | 相位反转 |

**EQ 上限说明**：`EQ_BAND_MAX_NB = 10`（[api_pacc.h:52](../../libs/cpu/api_pacc.h#L52)），`EQ_COEF_BUF_SIZE = EQ_COEF_LEN(10) = 20*10+4 = 204` 字节（单声道）。当前 ToolKit 里 10 个可用 band 与之一致。注意：`tool_eq_coef_t.coef[6][5]` 是上位机每次下发的临时数组（对应 `param_cnt` 个有效 band），不等于 PACC 最大支持的 10 band；当 `param_cnt > 6` 时，上位机会分多帧拆包下发。

### 4.2 DRC（MIC_DRC / MIX_DRC）

> DRC 的在线调参入口是 `pacc_drc_set_by_param(cs, band_cnt, drc_param)`，其中 `band_cnt = tool_drc_coef_t.coeff_cnt`，`drc_param` 指向 `in_attack_coeff` 起始的连续 32 位系数数组。`coeff_cnt` 决定 PACC 实际加载几段拐点；当前结构体给 4 段（LT/LG/CT/CG/ET/EG/NT/NG）。

结构体：`tool_drc_coef_t`（[toolkit_effect.h:70-85](../../modules/tool/toolkit_effect.h#L70-L85)）

```c
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int16_t delay;          // DelayBuf（ms）
    int8_t  sample_rate;
    int8_t  coeff_cnt;      // 实际系数段数
    int16_t RSVD;
    int32_t in_attack_coeff;    // Attack Level
    int32_t in_release_coeff;   // Release Level
    int32_t out_attack_coeff;   // Attack Gain
    int32_t out_release_coeff;  // Release Gain
    int32_t tav_coeff;          // 检测窗口
    int32_t drc_coef[4][3];     // 4 段 threshold/gain/slope
} tool_drc_coef_t;
```

| 上位机控件 | 固件字段 | 说明 |
|---|---|---|
| 使能 | `enable` | 决定是否真正进 PACC DRC |
| DelayBuf | `delay` | 预延迟缓冲（ms），用于 lookahead |
| LT / LG / CT / CG / ET / EG / NT / NG | `drc_coef[0..3]` 的 `threshold`、`gain`、`slope` | 四段限幅/压缩/扩展/噪声门拐点 |
| Attack Level / Release Level | `in_attack_coeff` / `in_release_coeff` | 输入检测 attack/release 系数 |
| Attack Gain / Release Gain | `out_attack_coeff` / `out_release_coeff` | 输出增益 attack/release 系数 |
| Level/RMS 窗口 | `tav_coeff` | 电平检测时间常数 |

**DRC 结构**：`drc_coef_t` 在 [api_pacc.h:39-49](../../libs/cpu/api_pacc.h#L39-L49) 已读：`in_attack, in_release, out_attack, out_release, in_level, out_gain, tav, in_rms, drc_coeff_t coeffs[4]`。每段 `coeffs[i]` 为 `threshold/gain/slope` 三元组。

### 4.3 关于当前板卡按键

> 当前开发板只有 `KEY_ID_PP`、`KEY_ID_K1`、`KEY_ID_K2` 三个有效按键，**没有 `KEY_ID_K3`**；项目未定义 `WIRELESS_MIC_K12_KEY_EN`，编译使用 `port_key.c` 的**非 K12** 按键表。MAGIC 入口在当前板卡为 **K2 长按抬起**（`KEY_LONG_UP`，长按后松手触发）→ `MSG_CHANGE_MAGIC`，**不是** K3 单击、也不是双击。K12 配置下才是 K3 单击，当前板卡不适用。

### 4.4 ECHO

结构体：`tool_echo_pitch_coef_t`（[toolkit_effect.h:15-24](../../modules/tool/toolkit_effect.h#L15-L24)）

```c
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int8_t  lp_filter_en;   // 低通滤波开关
    int8_t  attention;      // 衰减（0~90）
    int16_t delay;          // 延迟样点数
    int16_t cutoff_freq;    // 低通截止频率
    int16_t dry;            // 干度 Q15
    int16_t wet;            // 湿度 Q15
} tool_echo_pitch_coef_t;
```

参数映射到 `echo_audio_param_set(attenuation, delay_length, cutoffFreq_set, lp_filter_en, dry_set, wet_set)`（[echo.h:30](../../modules/voice/echo.h#L30)）。

| 上位机控件 | 固件参数 | 范围 |
|---|---|---|
| 使能 | `enable` → `echo_audio_mute_set(!enable)` | 0/1 |
| 低通开关 | `lp_filter_en` | 0/1 |
| 衰减 | `attention` → `attenuation` | 0~90 |
| 延迟 | `delay` → `delay_length` | 样点数 |
| 截止频率 | `cutoff_freq` → `cutoffFreq_set` | 1~4000 |
| 干度 | `dry` → `dry_set` | 0~49152 |
| 湿度 | `wet` → `wet_set` | 0~32768 |

### 4.4 MAGIC

结构体：`tool_magic_pitch_coef_t`（[toolkit_effect.h:6-11](../../modules/tool/toolkit_effect.h#L6-L11)）

```c
typedef struct __attribute__((packed)){
    int8_t  channel;
    int8_t  enable;
    int16_t shfit_hz;       // 变调量
} tool_magic_pitch_coef_t;
```

映射到 `magic_audio_mute_set(!enable)` + `magic_audio_param_set(shift_hz)`（[magic.c:57-65](../../modules/voice/magic.c#L57-L65)）。

| 上位机控件 | 固件参数 | 说明 |
|---|---|---|
| 使能 | `enable` → `magic_audio_mute_set(!enable)` | 0/1 |
| 变调量 | `shfit_hz` → `magic_param` | -32767 ~ +32767，对应不同音效等级 |

当前代码里有五档预设（`magic_effect_tbl`）：原音(0)、男神(5000)、女神(-13500)、娃娃音(-30000)、魔兽(15000)。ToolKit 里可以直接给任意 `shift_hz`。

### 4.5 Soft Gain

Soft Gain **不走 ToolKit 的 `*_effect_update_callback` 通道**（没有 `CFG_SOFT_GAIN` 名称），但它并非完全不可调：当前工程的 Soft Gain 等级由 `SOFT_GAIN_MAX_LEVEL=8`、`SOFT_GAIN_DEFAULT_LEVEL=0` 决定，每级步进见 `soft_gain_tbl_8`（-0/-2/-4/-6/-8/-10/-12/-15/-110 dB）。TX 的 Soft Gain 参数由本地按键控制（`MSG_SOFT_GAIN_UP` / `MSG_SOFT_GAIN_DOWN`），ToolKit 本身没有专用 Soft Gain 界面；如需联动，需通过其他控制消息（如 BLE 命令）或本地按键。

### 4.6 MIX_EQ / MIX_DRC

RX 侧结构和 TX 一致，只是 callback 变成 `mix_mic_eq_effect_update_callback` / `mix_mic_drc_effect_update_callback`，要求 `!wireless_role_is_adapter()` 才生效（RX 才允许调）。

---

## 5. 在线调参流程

### 5.1 连接步骤（按你当前截图）

1. **确认硬件接线**：HSUART 对应 GPIO（由 `xcfg_cb.huart_io_sel` 指定，通常是 PA0 单线）连接到串口工具板；
   > 注意：当前工程 `projects/microphone/Output/bin/Settings/*.setting` 里会定义 `huart_io_sel` 对应 IO，需与 ToolKit 所在 PC 的串口连线一致。
2. **打开 ToolKit**：加载 `effect.bsln`（或直接打开 `effect.bts`）；
3. **设置通信口**：选择正确 COM 口（你截图是 COM6）；
4. **设置波特率**：`1500000`；
5. **点击「开始」**：ToolKit 开始下发握手帧；
6. **使能各模块**：把 `MIC_EQ`、`MIC_DRC` 等开关拨到「使能」；
7. **在线调参**；
8. **停止**：调试完成后点「停止」或关闭 ToolKit。

### 5.2 固件侧处理时序

```text
ToolKit 下发 UART → HUART DMA RX → ISR 收到 'C','F','G' 开头
→ msg_enqueue(EVT_ONLINE_SET_EFFECT)
→ func.c 主循环取消息 → toolkit_process()
→ 解析 12 字节模块名（"CFG_MIC_EQ##" 等）
→ 匹配 effect_info[i].effect_cfg_name
→ 调 effect_update_callback_tbl[i].effect_update_callback(buf, len, 1)
→ 对应模块 callback 把参数写入 PACC / voice 模块
→ tx_ack() 回 14 字节 ACK
```

关键函数：
- `eq_drc_dbg_cmd_process`（中断上下文）：检查帧头 `'C','F','G'`，入队消息；
- `toolkit_process`（主循环上下文）：解析完整帧、CRC 校验、按名字分发；
- `*_effect_update_callback`（主循环上下文）：把上位机参数写入 PACC / voice 模块。

### 5.3 ACK 与错误处理

- **CRC 错误**：`CRC_ERROR` 打印，NACK（`toolkit_ack=1`）；
- **模块名不存在**：NACK；
- **角色不匹配**：如 TX 板收到 `MIX_EQ` / `MIX_DRC`，或 RX 板收到 `MIC_EQ` / `MIC_DRC`，`toolkit_ack_fail()` 直接 NACK；
- **算法未使能**（`sys_cb.mic_alg_en == 0`）：参数先不更新，NACK（直到连接后被使能）。

---

## 6. 离线固化：从 ToolKit 参数到固件默认参数

调好的参数如果直接断电就丢了。要变成固件默认参数：

1. **在 ToolKit 中保存效果**：点「导出 / 保存效果」；
2. **确认生成最新的 `effect.bin`**：ToolKit 会把当前链路参数打包成 `effect.bin`；
   > 你当前 `Output/bin/res/effect/effect.bin` 就是它。
3. **重新执行 prebuild**：`riscv32-elf-xmaker -b res.xm` 会把 `effect.bin` 打包进 `res.bin`，同时重新生成 `effect.c/h`；
4. **重新构建固件**：Code::Blocks 里 Build `Debug`；
5. **烧录**：`postbuild.bat` 调 `C:\upload\upload.bat -D AB5766 app.dcf` 下载。

> 留意：`effect.c/h` 是 prebuild 自动生成的，文件时间戳可能和 ToolKit 保存时间相关。改参数后务必确认 `effect.c/h` 已被重新生成。

---

## 7. 调试注意事项与常见问题

### 7.1 当前已验证状态

- TX 侧 `MIC_EQ`/`MIC_DRC` 在线调参**源码已确认**，但**硬件实测未走完**（当前板卡实测到 ECHO/MAGIC/Soft Gain 听感，EQ/DRC 客观指标未量化）；
- RX 侧 `MIX_EQ`/`MIX_DRC` 在线调参**源码已确认**，待双 TX 混音场景验证；
- `ECHO`、`MAGIC` 当前宏为 0，ToolKit 中灰色不可调；要调它们，先改 `WIRELESS_MIC_ECHO_EN` / `WIRELESS_MIC_MAGIC_EN` 为 1 重新构建。

### 7.2 常见坑

| 现象 | 排查方向 |
|---|---|
| ToolKit 连不上 / OK 无响应 | 波特率是否 1500000；GPIO 是否配置到 `huart_io_sel`；`xcfg_cb.huart_en` 是否 1 |
| 下发参数 NACK | 角色是否匹配（TX/RX）；算法是否已使能（`sys_cb.mic_alg_en`）；模块名是否匹配 |
| EQ/DRC 改了没听出来 | 确认 `WIRELESS_MIC_EQ_DRC_EN=1` 且 `sys_cb.mic_alg_en=1`；OBJ 级增益变化是否太小（改大点） |
| EQ 只改了 6 个 band | 确认 ToolKit 里其余 4 个 band 使能；当前 `param_cnt` 可能默认 6，`tool_eq_coef_t.coef[6][5]` 只携带 6 个 band |
| ECHO/MAGIC 灰色 | `WIRELESS_MIC_ECHO_EN` / `WIRELESS_MIC_MAGIC_EN` 当前为 0，改 1 后重新构建 |

### 7.3 与算法测试纪律的关系

按 [CLAUDE.md](../../CLAUDE.md) 和 [AB5766_LE_Mic_音频算法测试快速入门.md](AB5766_LE_Mic_音频算法测试快速入门.md) 要求：

- 每次**只改一个参数**或一个宏，独立构建/刷写/恢复；
- ToolKit 在线调参属于 **临时调参**，不替代 **宏配置的独立验证**；定型后仍要把参数固化回 `effect.bin` 重新烧录；
- Room Reverb 与 ECHO 同时开时，源码会 mute Room Reverb（[room_reverb.c:77-79](../../modules/voice/room_reverb.c#L77-L79)）；

---

## 8. 参考源码文件清单

| 主题 | 路径 |
|---|---|
| ToolKit 协议框架 | `modules/tool/toolkit.h` / `toolkit.c` |
| ToolKit 参数结构 | `modules/tool/toolkit_effect.h` |
| ToolKit 参数回调 | `modules/tool/toolkit_effect.c` |
| UART 调试驱动 | `modules/audio/eq_drc_dbg_in_uart.c/h`、`bsp/bsp_huart.c/h` |
| 初始化 | `bsp/bsp_sys.c`（`eq_drc_dbg_init`） |
| 主循环消息 | `functions/func.c`（`EVT_ONLINE_SET_EFFECT`） |
| EQ/DRC PACC 包装 | `modules/effect/mic_effect.c`、`modules/audio/mic_eq_drc.c` |
| ECHO / MAGIC | `modules/voice/echo.c/h`、`modules/voice/magic.c/h` |
| Soft Gain | `modules/audio/soft_gain.c/h` |
| 生成物 | `Output/bin/effect.c/h/bts/bsln`、`Output/bin/prebuild.bat` |
| 配置宏 | `projects/microphone/config_ab5766_le_mic.h` |

---

## 9. 附：ToolKit UART 协议帧速览

### 9.1 命令帧格式

- 前 12 字节：ASCII 模块名（`"CFG_MIC_EQ##"`、`"CFG_MIC_DRC#"` 等）；
- 13~14 字节：`size`（后续数据长度）；
- 15~16 字节：`cmd`（特殊命令，普通调参通常为 0）；
- 载荷：模块参数（按 `tool_*_coef_t` 打包）；
- 尾部：2 字节 CRC16（`calc_crc`，seed = `0xffff`）。

### 9.2 ACK 应答

- 14 字节：
  - 12 字节模块名（回显）；
  - 1 字节 `toolkit_ack`（0=OK，1=FAIL）；
  - 1 字节 `checksum`（对前 13 字节求和取负）。

### 9.3 回读 BIN

- 命令 `TOOL_CMD_GET_BIN` 会回读 `effect_res_len_get(EFFECT_IDX_MAX_NB)` 长度的 `effect.bin` 数据；
- 用于上位机确认固件里当前固化的效果参数。

---

*本文档基于当前 `app_branch01` 分支源码调研生成，不依赖未读源码位置的推测；后续固件或 ToolKit 版本变更时，按本文第 8 节清单重新核对。*

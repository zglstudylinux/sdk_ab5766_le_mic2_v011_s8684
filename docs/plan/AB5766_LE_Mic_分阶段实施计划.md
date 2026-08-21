# AB5766 LE Mic 分阶段实施计划

## 目标与边界

本计划落实 [plan.md](plan.md) 的五项任务：新手开发指南、LED 显示、按键、音频传输链路测试和数据传输链路测试。

- 不新增独立的工厂测试模式或测试框架。
- 优先复用已有 UART 日志和无线诊断能力；如需 PER/RSSI，后续仅在获得批准后启用已有的 `WIRELESS_DUMP_EN`。
- 不编造下载器、接线、进入下载模式或厂商工具安装步骤；这些信息不在当前仓库中。
- 当前仓库没有可运行的自动化测试框架。硬件相关改动由用户在真实硬件上构建、烧录和验证。

## 每个阶段的门禁

每个模块都遵循以下流程：

1. 实现一个模块，并说明改动文件、配置影响、预期现象和检查清单。
2. 用户审查该模块。
3. 若模块修改了固件、配置或构建资源，用户使用现有工程自行构建、烧录并完成硬件验证。
4. 用户明确确认后，才提交并上传该阶段内容至 GitHub。
5. 仅在上述步骤完成后，才开始下一模块。

## 阶段 1：新手开发指南

**交付物：** [../SDK/AB5766_LE_Mic_新手开发指南.md](../SDK/AB5766_LE_Mic_新手开发指南.md)

- 用中文解释目录分层、构建/打包边界、三层配置、启动状态机、LED、ADC 按键、音频与私有用户数据链路。
- 提供基于真实源码的 Mermaid 流程图、阅读路径、调试点和首次改动检查表。
- 该阶段仅修改文档，不要求构建或硬件验证。
- 完成后暂停，等待用户审查；未经确认不开始 LED 阶段，也不上传 GitHub。

## 阶段 2：LED 显示

- 已完成禁用态安全保护：`bled_set_on()`、`bled_set_off()` 与 `led_scan()` 在 `xcfg_cb.bled_disp_en=0` 时不再操作或扫描蓝灯 GPIO。
- 保持既有状态链路：`xcfg_init()` → `led_init()`，无线连接状态变化 → `led_bt_idle()` / `led_bt_connected()`；未修改无线状态机、定时器调度或默认图样。
- `BSP_LED_EN` 是编译期开关；`xcfg_cb.bled_disp_en`、`bled_io_sel` 是运行时配置。配置界面的默认 PA0 不是已确认的板级连接，未经硬件确认不得启用蓝灯或修改 GPIO 默认值。
- 红色充电指示灯原属阶段 6 P2 范围，现已在 P2 启用并实测（PB4 恒亮正常）；红灯无闪烁机制的根因见下方 B 节。共享 IO 的双色灯也须在确认电路拓扑后单独验证。
- **引脚冲突约束**：蓝灯 IO 不可选 PB3。PB3 是编译期 `BSP_UART_DEBUG_EN`（UART0 TX 交叉开关），蓝灯配 PB3 会致启动卡死，详见下方 A 节。
- 验收：在确认蓝灯 GPIO、有效电平、独立/共脚拓扑和引脚无冲突后，由用户构建、烧录，验证未连接、首条连接、最后一条断开时的状态转换，并结合现有连接日志记录结果。
- 阶段完成后等待用户确认，才提交/上传并进入按键阶段。

### 阶段 2 实测发现：PB3 蓝灯冲突卡死 与 红灯无闪烁机制

> 本节为用户在真实硬件上配置 LED 后的实测结果与根因排查，归档于阶段 2。两项发现均与 LED 引脚/驱动直接相关，其中充电红灯的实际启用发生在阶段 6 P2，但红灯「无闪烁机制」这一驱动层事实在此一并记录。

#### A. 蓝灯配置到 PB3 会卡死在启动日志处

**现象**：经 ToolKit 把「系统指示灯（蓝灯）」IO 选为 PB3（`bled_io_sel=179`）后，串口打印停在 PMU 初始化末尾，不再推进：

```text
lock wireless_mic comm
vddio:10
vddcore:20
vddbt:9
lvd:2
-->ldo_en
vddbt_capless_en:1
```

把蓝灯移出 PB3、改配到其他引脚后，打印恢复正常。充电红灯配到 PB4 目前正常。

> 注：基线启动日志该行为 `vddbt_capless_en:0`（见 [../SDK/AB5766_启动日志_每一行打印的来源与调用链路.md](../SDK/AB5766_启动日志_每一行打印的来源与调用链路.md)）；此处为 `:1` 是因为当前运行时配置 `vddbt_capless_en=True`（[wireless_mic_emit.setting](../../projects/microphone/Output/bin/Settings/wireless_mic_emit.setting)），非异常。

**根因**：PB3 在编译期同时被定义为调试串口引脚——[config_ab5766_le_mic.h:24](../../projects/microphone/config_ab5766_le_mic.h) `#define BSP_UART_DEBUG_EN GPIO_PB3`。`bsp_uart_debug_init()`（[bsp_uart_debug.c](../../bsp/bsp_uart_debug.c)）把 UART0 的 TX 经交叉开关映射到 PB3（`gpio_func_mapping_config(..., GPIO_CROSSBAR_OUT_UART0TX)`），printf 的 putchar 经 UART0 从 PB3 输出。该初始化在启动早期由库回调 `uart_debug_init_callback()`（[strong_symbol.c:66](../../projects/microphone/strong_symbol.c)）触发完成。

而在 [bsp_sys.c:224](../../bsp/bsp_sys.c) 的 `led_init()` 中，当 `xcfg_cb.bled_disp_en` 为真且 `bled_io_sel=179`(PB3) 时，`led_cfg_init(BSP_BLED_PIN, BSP_BLED_PORT, 0)` → `gpio_init(GPIOB_REG, ...)` 把 PB3 重新配置为「数字输出 GPIO」（`GPIO_FEN_GPIO` + `GPIO_FDIR_SELF` + `GPIO_MODE_DIGITAL` + `GPIO_DIR_OUTPUT`，见 [driver_gpio.c:15](../../driver/driver_gpio.c) 的 `gpio_init`），**覆盖了先前建立的 UART0 TX 交叉开关映射**。

此后 UART0 发送时，发送完成标志（`UARTxCON_TXPND`，[driver_uart.c:150](../../driver/driver_uart.c) 中 `uart_baud_config` 即以 `while ((uartx->con & UARTxCON_TXPND) == 0);` 方式等待）因 TX 物理引脚已不再是 UART 外设引脚而无法按预期翻转，putchar 的忙等待死循环，系统卡死。

**为何日志正好停在 `vddbt_capless_en:1`**：`bsp_sys_init()`（[bsp_sys.c:200](../../bsp/bsp_sys.c)）的顺序是 `xcfg_init` → `bsp_var_init` → `wireless_mic_var_init` → `pmu_init`（打印 `lock wireless_mic comm` … `vddbt_capless_en:1`）→ … → `led_init()`（L224，此处破坏 PB3 的 UART TX 映射，本身无打印）→ … → `printf("%s\n",__func__)`（L264，打印 `bsp_sys_init`）。PMU 的全部打印在 `led_init()` 之前完成，所以最后一条可见日志是 pmu_init 的末行 `vddbt_capless_en:1`；紧接着 `led_init()` 破坏引脚，下一处 printf（L264 的 `bsp_sys_init`）即死锁，故该行不再出现，与用户观察一致。

**解决路径**（二选一）：
1. 蓝灯不配 PB3，改用其他空闲引脚——当前实测改到 PA0（`bled_io_sel=160`），打印恢复正常；仅改运行时配置，无需重新编译。
2. 若一定要用 PB3 作蓝灯，须先把编译期 `BSP_UART_DEBUG_EN` 从 `GPIO_PB3` 改到其他引脚（PA0/PA1/PA4 均可，见双板测试指南「调试串口改 PA4」的先例 [../SDK/AB5766_无线麦克风_双板连接与LED测试指南.md](../SDK/AB5766_无线麦克风_双板连接与LED测试指南.md) 第 4.1 节），重新编译烧录后再把蓝灯配到 PB3。

> 本条同时作为 6.4 风险表「蓝灯配 PB3 致启动卡死」的根因依据。

#### B. 红灯（充电指示灯）没有闪烁机制，仅恒亮/恒灭

排查 [bsp_led.c](../../bsp/bsp_led.c) 与 [bsp_charge.c](../../bsp/bsp_charge.c) 后确认：红灯不存在任何闪烁驱动路径，`led_cb_t.rled_sta` 是死字段。

- `led_set_sta_p()`（[bsp_led.c:189](../../bsp/bsp_led.c)）只写 `s->bled_sta = sta.bluepat`，从不写 `s->rled_sta`；`rled_sta`（[bsp_led.h:20](../../bsp/bsp_led.h)）在整个 LED 状态链路中没有任何写入点。
- `led_scan()`（[bsp_led.c:214](../../bsp/bsp_led.c)）的模式消耗只读 `s->bled_sta & BIT(bcnt)`（L246），据此调用 `bled_set_on/off()`，**从不引用 `rled_sta`，也从不调用 `rled_set_on/rled_set_off`**。
- 因此所有 `led_bt_idle()` / `led_bt_connected()` / `led_power_up()` / `led_power_down()` 设置的图样（含 `redpat` 字节）只会影响蓝灯，红灯纹丝不动。

红灯唯一的驱动来自充电流程：`bsp_charge_process()`（[bsp_charge.c:110](../../bsp/bsp_charge.c)）在充电状态变化时，若 `charge_status >= CHARGE_STA_ON_CON_CURR` 调 `led_charge_on()`（红灯恒亮 + 蓝灯灭），否则调 `led_charge_off()`（红灯灭）。即红灯只有「充电中恒亮 / 非充电恒灭」两态，**无闪烁**。

**当前实测**：充电红灯配在 PB4（`rled_io_sel=180`，`rled_disp_en=True`，`charge_en=True`），高电平点亮（`rled_set_on()` 用 `gpio_set_bits`，见 [bsp_led.c:97](../../bsp/bsp_led.c)），充电时红灯恒亮正常。PB4 虽为 USB DM，但本工程 USB 相关宏全关，无冲突。

**若后续需要红灯闪烁**（例如充电异常提示、低电告警），需新增驱动：在 `led_set_sta_p()` 写入 `s->rled_sta`，并在 `led_scan()` 的 `charge_flag` 早退分支之外另开红灯模式消耗分支（或单独加红灯扫描入口）。该改动属于新功能，不在当前阶段范围，实施前需单独评审。

## 阶段 3：按键

**交付物：** [../SDK/AB5766_按键链路测试与验收指南.md](../SDK/AB5766_按键链路测试与验收指南.md)

- 当前阶段首轮只交付测试与验收文档，不修改固件按键逻辑、ADC 阈值、去抖/长按参数、消息表或 IO 按键配置。
- 现有基线路径为：`bsp_adkey_init()` → `bsp_key_scan()` → `key_status_process()` / `key_multi_press_process()` → 角色消息表 → `msg_enqueue()` → 角色消息处理。
- 当前配置为 `BSP_ADKEY_EN=1`、`BSP_IOKEY_EN=0`，通道为 WK0 SARADC。仓库没有板级原理图、WK0 实际引脚和电阻梯资料，未经硬件确认不启用 IO 按键、不改通道或阈值。
- 发射端默认可用证据为 PP/K1/K2 短按产生的 `MSG_VOL_MUTE`、`MSG_VOL_UP`、`MSG_VOL_DOWN` UART 日志；PP HOLD 进入既有关机流程。适配器端默认仅 PP HOLD 具有关机映射，普通短按无业务动作属于预期。
- 验收覆盖：已连接和未连接下的发射端 PP/K1/K2、PP 长按关机、重复性和静置误触发；适配器端 PP 长按及普通按键无动作；用户应记录物理键与逻辑键映射、完整 UART 证据和实际音频/关机现象。
- 不在 5 ms 扫描路径逐采样打印 ADC 值。仅当用户能稳定复现无响应、错键、连发或静置误触发时，才单独评审默认关闭、仅键状态变化输出 ADC 值/键 ID 的最小诊断补丁。
- 用户审查文档并自行构建、烧录、完成硬件验证后才提交/上传；确认完成后才进入音频传输链路测试。

## 阶段 4：音频传输链路测试

- 基于现有链路编写和执行测试：SDADC/DMA 采样 → 编码 → `wireless_d2a_put_tx_frame()` → 无线传输 → 接收/解码/PLC/混音 → DAC 或 USB Mic 输出。
- 使用已有启动、连接及虚拟音频连接/断开日志作为证据。
- 如需 PER/RSSI，先获得批准，再在诊断构建中启用 `WIRELESS_DUMP_EN`，并记录该配置是否保留。
- 用户验证连接、单/双链路声音、断连重连及可选指标后才提交/上传，并暂停。

## 阶段 5：数据传输链路测试

- 记录和测试既有链路：`wireless_tx_user_cmd()` → `PRIVATE_USER_DATA` → 命令队列/蓝牙回调 → `wireless_rx_user_cmd()`。
- 用已有 `TX_CMDn` / `RX_CMDn` 日志区分“命令传输”与“应用层已经消费数据”。
- 已知限制：`wireless_rx_user_cmd()` 当前没有应用层用户载荷处理，不能据此声称端到端用户数据验证完成。
- 如需真正的端到端验证，必须先获得用户对最小可观测接收处理、数据格式、长度/错误处理和输出方式的批准，再复用既有应用/日志模式实施。
- 覆盖正常、断连、超长和队列压力路径；用户构建、烧录和确认后才提交/上传。

## 阶段 6：TX 混响+降噪+充电+数码管评估与实现

### 6.1 评估结论

**总体结论：** 四大功能在 8 引脚约束下均可实现。Flash 资源在 ECHO+32K+AINS4 三者同开后已顶满（见 6.2），需以「关调试宏 / 关非实时音频功能」换取新功能空间。

| 功能 | 可行性 | 引脚占用 | 资源占用 |
|---|---|---|---|
| **混响（ECHO）** | ✅ 已实现 | DAC PA2（TX 本地监听） | 代码 +0.88KB，emit RAM +5.66KB，mram 复用 |
| **降噪（AINS4 32K）** | ✅ 已实现（与 ECHO/32K 同开） | 0（纯算法） | 代码 +11.9KB，emit RAM +11.0KB —— Flash 主增量 |
| **充电** | ✅ 已实现 | 0（VUSB 检测用内部 ADC，不占用 GPIO） | Flash +少量（复用库内 charge 模块） |
| **数码管** | ✅ 已评估（软件可行，待硬件） | TM1650 预留 PA3(SDA)/PB1(SCL)，走硬件 IIC | 驱动+字形约 0.6–1.0KB，须先腾 Flash；电量显示需另开 `BSP_VBAT_DETECT_EN` |

**引脚分配表：**

| 引脚 | 功能 | 备注 |
|---|---|---|
| PB0 | ADKEY（PP/K1/K2） | 不变 |
| PB3 | UART Debug | 保留打印；**该脚不可再配蓝灯**——PB3 是编译期 `BSP_UART_DEBUG_EN`，蓝灯配 PB3 会致启动卡死（见 6.4 风险表「蓝灯配 PB3 致启动卡死」、阶段 2 A 节） |
| PA2 | **DAC 输出** | TX 本地混响监听 |
| PA3 | **TM1650 SDA** | IIC 数据线，走 `FO_I2CSDA` 功能映射 |
| PB1 | **TM1650 SCL** | IIC 时钟线，走 `FO_I2CSCL` 功能映射 |
| PA0/PA1/PA4 | 预留 | 扩展用；当前蓝灯实测配在 PA0（`bled_io_sel=160`），见阶段 2 A 节 |
| PB4 | **充电红灯** | `rled_io_sel=180`(PB4)，高电平点亮，充电恒亮正常；虽为 USB DM，但本工程 USB 相关宏全关，无冲突（见阶段 2 B 节） |

### 6.2 资源评估（map.txt）

> 下表为 P1+ 裁剪后（ECHO+32K+AINS4 同开、关调音三宏+RX混音DRC）的新 map.txt 实测值，替代 P1 阶段基线。

| 区域 | 定义大小 | 当前占用 | 余量 | 说明 |
|---|---|---|---|---|
| **Flash（代码可用）** | 120 KB (`0x1e000`) | **120 KB**（`.flash` 0x12400 + `.code_comm` 0x2c64 + `.code_adapter` 0x2048 + `.code_emit` 0x3858，LMA 合计 ≈ 0x1e000） | **0 KB（顶满）** | AINS4 段 0x2f72(≈11.9KB) 是主增量；关闭调音三宏+混音DRC 才塞得下。**后续新增代码必须先腾空间** |
| **comm（公共 RAM）** | 27 KB (`0x6c00`) | `.code_comm` 11.09KB + `.data_comm` 14.88KB = **≈ 25.97 KB**（0x67a8） | **约 1.0 KB** | `.data_comm`=0x3b00(14.88KB)，余量比 P1 基线略宽松 |
| **emit（TX 代码+算法缓冲）** | 34 KB (`0x8800`) | `.code_emit` 14.08KB + `.data_emit` 18.84KB(0x4b40，含 AINS4 buf 11.0KB + ECHO buf 5.66KB + mic_effect 等) = **≈ 32.9 KB** | **约 1.1 KB** | AINS4 RAM buf 0x2be0 + ECHO RAM buf 0x161e，已接近顶满 |
| **adapter（RX 代码+缓冲）** | 34 KB (`0x8800`) | `.code_adapter` 8.07KB + `.data_adapter` 22.06KB(0x5650) = **≈ 30.1 KB** | **约 3.9 KB** | 关 `ADAPTER_MIX_DRC_EN` 后 `mic_mix_proc` 段不再链接；`.data_adapter` 占 USB/sbc/plc/lc3dec 等缓冲 |
| **mram（ECHO/Reverb 专用复用）** | 32 KB (`0x8000`) | `.mram_echo`/`.mram_reverb` 各预留 0x6000(24KB) NOLOAD | 占用 24KB，余 8KB | ECHO 启用时复用该段放 echo/reverb 大 buffer |

### 6.3 实施步骤

#### P0：基线验证

- **改动**：无
- **验证**：TX→RX→耳机链路正常
- **状态**：已完成

#### P1：ECHO 混响 + DAC PA2 监听

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`WIRELESS_MIC_ECHO_EN 0→1`
  - `projects/microphone/config_ab5766_le_mic.h`：`ECHO_DELAY_BUF_SIZE 6000→2000`（防溢出）
  - `projects/microphone/port/port_key.c`：K1 双击列 `MSG_NO→MSG_ECHO_LEVEL_UP`
  - `projects/microphone/port/port_key.c`：K2 双击列 `MSG_NO→MSG_ECHO_LEVEL_DOWN`
- **原理**：ECHO 算法在发射端 PACC 上处理，处理后的 PCM 经 `dac0_out_audio_input()` 输出到 PA2（TX 本地监听），同时经 LC3S 编码 + 无线传输到 RX 端解码输出。
- **验证**：TX 耳机和 RX 耳机同时听到混响，K1/K2 双击可调混响等级 0–8
- **状态**：✅ 已完成（用户确认能听到混响）

#### P1+：同时使能 ECHO + 32K + AINS4（Flash 裁剪）

- **背景**：P1 为隔离混响验证临时将 `WIRELESS_MIC_AINS4_32K_EN 1→0`。现要求 ECHO、32K 采样、AINS4 32K 降噪三者同时使能。
- **问题**：恢复 `WIRELESS_MIC_AINS4_32K_EN 0→1` 后 `CODE SIZE: 124 KB`，超出 120 KB 上限，链接器报 `Code [0x0-0x1EFFF] and [0x1E000-...] overlap`，构建失败。`ECHO_DELAY_BUF_SIZE` 只省 RAM 不省 Flash，对此无效。
- **方案**（一次性关闭一组调试/非实时音频宏以腾出 Flash，符合 CLAUDE.md「允许多宏同改」纪律；保留 `WIRELESS_MIC_ECHO_EN=1`、`WIRELESS_MIC_32K_EN=1`、`WIRELESS_MIC_AINS4_32K_EN=1`）：
  - `ADAPTER_MIX_DRC_EN 1→0`：关 RX 一拖二混音 DRC（单路收听不受影响，仅影响多路混音）
  - `EQ_DRC_DBG_IN_UART 1→0`：关 UART 在线调 EQ
  - `EFFECT_DBG_ADJUST_EN 1→0`：关音效离线调试
  - `EFFECT_DBG_ADJUST_IN_UART 1→0`：关 UART 在线调音效（依赖前两者）
- **构建证据**：`CODE SIZE: 120 KB`、`Process terminated with status 0`、`0 error(s), 0 warning(s)`，溢出消除。
- **新 map.txt 关键值**：`.flash`(`__bank_size`)=0x12400(≈73.25KB)、`.code_comm`(`__comm_size`)=0x2c64(≈11.09KB)、`.code_adapter`(`__comm_apapter_size`)=0x2048(≈8.07KB)、`.code_emit`(`__comm_emit_size`)=0x3858(≈14.08KB)、`.code_test`(`__comm_test_size`)=0。AINS4 代码段 0x2f72(≈11.9KB)、AINS4 RAM buf 0x2be0(≈11.0KB)、ECHO 代码段 0x388(≈0.88KB)、ECHO RAM buf 0x161e(≈5.66KB)。
- **余量**：xmaker 报告 `CODE SIZE: 120 KB`，与 120 KB(`0x1e000`) 上限持平；可加载 Flash 段累加约 107 KB，与工具报告的 120 KB 差额来自段间对齐填充与下载镜像组织。可认为 **Flash 余量极小（0–13 KB）**，后续新增代码必须先腾空间。
- **已验证未触发的两个链接风险**：
  1. `effect_update_callback_tbl`（[effect_table.c](modules/effect/effect_table.c) 无条件引用 6 个回调，回调定义在 `#if EFFECT_DBG_ADJUST_EN` 内）——构建未报 undefined reference，map.txt 中该表无符号、`toolkit.o`/`toolkit_effect.o` 的 `.text` 均为 `0x0`，证明 `--gc-sections` 已随未引用消费者将其丢弃，无需给 effect_table.c 加 `#if` 包裹。
  2. `bsp_huart.o`（受 `LE_DUT_UART_EN || EQ_DRC_DBG_IN_UART` 控制，两者均为 0）——`.text` 为 `0x0`，GC 丢弃，无 undefined reference。
- **状态**：✅ 已完成（构建通过，待刷写 + 听感/打印验证 ECHO/降噪/充电三功能同时可用）

#### P2：充电检测

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`BSP_CHARGE_EN 0→1`
  - `projects/microphone/Output/bin/xcfg.xm`：`CHARGE_WORKING_WHILE_CHARGING 0→1`（真正生效项——prebuild.bat 经 `riscv32-elf-xmaker -b xcfg.xm` 生成 `xcfg.h`，`xcfg_cb.charge_working_while_charging` 默认值由此决定）
  - `projects/microphone/Output/bin/Settings/wireless_mic_emit.setting`：`charge_working_while_charging False→True`、`mic_dig_gain 36→0`（ToolKit 默认值同步，不影响构建，仅工具界面显示用）
  - 充电红灯 IO 配置：`rled_disp_en=True`、`rled_io_sel=180`(PB4)（运行时配置，无需重编译）
- **原理**：`bsp_charge_process()` 调用 `charge_detect_dc()` 读 `RTCCON[20]VUSB/[22]INBOX` 判断充电器插入，状态机驱动涓流→恒流→恒压→充满流程。`charge_working_while_charging=True` 时，插入充电器不会阻塞主循环，可边充电边正常发射音频。
- **验证**：插充电器看到 `charge: 3`（恒流）或 `charge: 4`（恒压）打印，充满后 `charge: 2`（充满但还插着）→ 自动关机；充电过程中无线连接和音频正常
- **状态**：✅ 已完成（代码+配置已改，本次构建通过，待刷写验证 `charge: x` 打印）

> 充电红灯实测：红灯配 PB4（`rled_io_sel=180`）高电平点亮，充电时恒亮正常。注意红灯无闪烁机制，仅恒亮/恒灭（见阶段 2 B 节、6.4 风险表「红灯无闪烁机制」）。

#### P3：数码管驱动（TM1650）

**评估结论（本次补充，仅评估，未改代码）：**

- **可行性**：✅ 软件可行。AB5766 自带硬件 IIC 控制器（`driver_iic.c`）与 `FO_I2CSCL/FO_I2CSDA` GPIO 功能映射，不需要软件模拟 I2C，也无需新增底层外设驱动。当前 `driver_iic.o` 已被 `--gc-sections` 丢弃（map.txt 中该模块 `.text/.data/.bss` 全为 `0x0`），新增 `bsp_7seg` 调用 IIC 后才需重新链接进固件。
- **接线与冲突确认**：SDA→PA3、SCL→PB1。两者均未在 `WIRELESS_MIC_DAC_OUT_EN=1`（TX 本地监听）路径中占用——DAC 监听只占 PA2（`dac0_out_init`→SDDAC），IIS IOMAP_0（PA8/PA0/PA1/PA2/PA3）在本工程**未被调用**（`bsp_iis_init` 无任何 `functions/`、`modules/` 引用，对应 IIS 段 GC 掉）。PA3 是 SDADC DC 模式的可选采样通道（`DC_SAMP_CH2_PA3_EN`），但当前 MIC 模式（`SDADC_MODE_MIC`）不使用，无冲突。
- **驱动数据通路**：`bsp_7seg_init()` → `iic_master_init(IIC_REG, ...)` + `gpio_func_mapping_config(GPIOA_REG, GPIO_PIN_3, GPIO_CROSSBAR_OUT_I2CSDA)` + `gpio_func_mapping_config(GPIOB_REG, GPIO_PIN_1, GPIO_CROSSBAR_OUT_I2CSCL)` → `iic_master_send_data(IIC_REG, dev_addr, reg_addr, data, len, timeout_ms)`。
- **TM1650 协议要点**：7-bit 设备地址 0x24（默认，写地址 0x48）；两条命令——`0x48`（系统设置：位 0 显示开关、位 1~2 亮度 1~7 级、位 3 显示测试）；`0x68~0x6F`（4 个 DIG 位地址，数据字节为 7 段码）。本 SDK `iic_master_send_data` 的 `dev_addr` 参数**会被 `& 0xfe`**（见 `iic_transmit_config`），因此传 0x48（或 0x68）即可，无需手动补读写位。每次最多传 4 字节（`len<=4`），TM1650 每命令 2 字节，满足。
- **IIC 时钟**：`clk_iic_set(CLK_IIC_RC2M)`（RC2M≈2MHz）或 `CLK_IIC_X24MDIV8`（24M/8=3MHz）二选一，配合 `iic_master_init` 的 `scl_pose_div`/`sda_hold_cnt` 得到 ≤ TM1650 支持的 100k/400k 时钟。具体分频值需在实现时按 `driver_iic.c` 的 POSDIV/HOLDCNT 字段实测标定（仓库无 TM1650 现成参数，不能用 SDK 默认值照抄）。
- **显示内容映射**：
  - 混响等级：`echo_delay_level_get()`（`modules/voice/echo.c:148`，返回 0~8）→ 显示 `0~8` 或 `ECHO_DELAY_MAX_LEVEL=9` 对应档位。
  - 充电状态：`charge_get_status()`（`driver_charge.h:208`，`CHARGE_STA_OFF=1 / OFF_BUT_DC_IN=2 / ON_TRICKLE=3 / ON_CON_CURR=4 / ON_CON_VOL=5`）→ 显示 `C`（充电中）或闪烁。注：充电红灯本身无闪烁机制（见阶段 2 B 节），数码管上的充电状态闪烁需由 `bsp_7seg` 自行实现，不能复用红灯闪烁路径。
  - 电量：**当前 `BSP_VBAT_DETECT_EN=0`**，`bsp_get_vbat_level()`（`bsp_saradc_vbat.c:140`）整体不编译（`#if BSP_VBAT_DETECT_EN`）。要显示 0~9 电量，必须另行把 `BSP_VBAT_DETECT_EN 0→1`，会额外占用 SARADC 采集周期与 Flash，需单独评估（见 6.4 风险表新增项）。
- **Flash 空间约束（最关键）**：`bsp_7seg.c/h` + 7 段字形表 + IIC 驱动重链接，估算 `.text` 增量约 0.6~1.0KB。当前 Flash 已顶满 120KB（余量 0~13KB，见 6.2），直接加会冒溢出风险。**实现 P3 前必须先腾空间**，首选 P4 `WIRELESS_MIC_DAC_OUT_EN 1→0`（省 TX 本地监听 DAC 链路 `dac0_out_init/dac0_out_audio_input` 及 SDDAC 相关段），或再关一个非实时功能宏。
- **构建集成**：新增 `bsp/bsp_7seg.c` 需在 `projects/microphone/app.cbp` 的编译文件列表中加入（否则不参与链接，表现为 undefined reference）；建议把驱动段用 `AT(.text.bsp.7seg)` 独立标注，便于 `--gc-sections` 在未启用时整体回收。

- **改动（待硬件到位后实施，当前不执行）**：
  - 新增 `bsp/bsp_7seg.c/h`，用 `driver_iic.c` 的硬件 I2C 接口驱动 TM1650
  - 硬件连接：SDA→PA3、SCL→PB1
  - 在 `app.cbp` 里把 `bsp_7seg.c` 加进构建列表
- **原理**：TM1650 是 I2C 接口的 4 位 7 段数码管驱动芯片，只需 2 根线即可驱动 4 位数码管。
- **验证**：数码管显示电量 0–9、混响等级 0–9、充电状态（全亮/闪烁），无乱码
- **状态**：⏸️ 待硬件（软件评估已完成，未改代码）

#### P4：去掉 TX 本地监听

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`WIRELESS_MIC_DAC_OUT_EN 1→0`
- **原理**：关闭 TX 本地 DAC 输出，仅保留 RX 远端监听，省 1 个 DAC 引脚。
- **验证**：TX 耳机无声音，RX 耳机正常，混响功能正常
- **状态**：⏳ 待实现

### 6.4 风险与注意事项

| 风险 | 影响 | 缓解措施 |
|---|---|---|
| **Flash 余量 0–13 KB（顶满）** | P3 数码管驱动新增 `.text` 会立即溢出 | 先用 `WIRELESS_MIC_DAC_OUT_EN 1→0`（P4）或关更多调试宏腾空间再实现 P3；优先把驱动放 `AT(.text.bsp.7seg)` 便于 GC |
| **comm RAM 余量约 1.0 KB** | 数码管/充电 buffer 需严格控制 | 数码管用全局变量 < 100 字节，充电复用现有 `charge_status` 结构体 |
| **emit RAM 余量约 1.1 KB** | AINS4 buf(11KB)+ECHO buf(5.66KB) 已占大半 | ECHO buffer 已缩到 2000 样点；若开 MAGIC 需先评估 `pitch_shift` buf |
| **ECHO 与 MAGIC 互斥** | 后续若开 MAGIC 会挤占 emit 区 | 当前 MAGIC_EN=0，无风险；开 MAGIC 前评估 emit 剩余 1.1KB 是否够 |
| **ECHO buffer 溢出** | `.data_emit` 曾溢出 6872 字节 | 已通过减小 `ECHO_DELAY_BUF_SIZE` 到 2000 解决 |
| **mram 区不可执行** | 把 `delay_lbuf` 放 mram 导致 INST ERR | 已恢复 `AT(.buf.echo.delay)`，不再放 mram |
| **effect_update_callback_tbl 链接风险** | `EFFECT_DBG_ADJUST_EN=0` 后 6 个回调定义消失，但表无条件引用它们 | 本构建已验证 `--gc-sections` 丢弃了该表（map 无符号、toolkit.o/toolkit_effect.o `.text`=0），未报 undefined reference；若后续手动调整链接脚本/GC 选项需复验 |
| **数码管电量显示依赖 VBAT 检测** | 当前 `BSP_VBAT_DETECT_EN=0`，`bsp_get_vbat_level()` 不编译，数码管无法显示真实电量 | 实现 P3 电量显示时需同步评估 `BSP_VBAT_DETECT_EN 0→1` 的 SARADC 采集周期与 Flash/RAM 增量；短期可先显示混响等级+充电状态，电量留待后续 |
| **IIC 时钟参数无现成标定** | 仓库无 TM1650 现成 IIC 分频参数，SDK 默认 `scl_pose_div`/`sda_hold_cnt` 不保证匹配 TM1650 100k/400k | 实现时按 `driver_iic.c` 的 POSDIV/HOLDCNT 字段结合 `clk_iic_set` 时钟源实测标定，先逻辑分析仪/示波器看 SCL 波形再连数码管 |
| **蓝灯配 PB3 致启动卡死** | PB3 同时是编译期 `BSP_UART_DEBUG_EN`（UART0 TX 交叉开关），蓝灯配 PB3 后 `led_init()`→`gpio_init()` 把 PB3 改成数字输出 GPIO，覆盖 UART TX 映射，printf 忙等待死锁，系统卡在 PMU 末行 `vddbt_capless_en:1` 之后 | 蓝灯不配 PB3（实测改 PA0 即恢复）；若必须用 PB3，先把 `BSP_UART_DEBUG_EN` 改到 PA0/PA1/PA4 并重编译。详见阶段 2「PB3 蓝灯冲突卡死」与 6.1 引脚表注 |
| **红灯无闪烁机制（`rled_sta` 死字段）** | `led_set_sta_p()` 只写 `bled_sta`、`led_scan()` 只消耗 `bled_sta`，红灯无任何闪烁路径，`rled_sta` 从无写入点。红灯仅 `led_charge_on/off` 恒亮/恒灭 | 后续若需红灯闪烁告警/低电提示，须新增 `rled_sta` 写入点与 `led_scan()` 红灯分支，属新功能需单独评审；当前充电红灯 PB4 恒亮已实测正常 |

### 6.5 后续扩展建议

- **降噪恢复**：✅ 已在 P1+ 恢复 `WIRELESS_MIC_AINS4_32K_EN=1`，ECHO+32K+AINS4 三者同开且构建通过；待刷写听感验证
- **数码管硬件确认**：TM1650 采购后实现 P3（软件评估已完成：走硬件 IIC、PA3/PB1 无冲突；注意先腾 Flash，电量显示另需 `BSP_VBAT_DETECT_EN`）
- **按键扩展**：PA0/PA1/PA4/PB4 可留给 KEY4、LED、第二路 ADC 等扩展

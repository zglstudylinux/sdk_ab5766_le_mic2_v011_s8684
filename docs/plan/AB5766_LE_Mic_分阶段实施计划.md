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
- 当前 `BSP_CHARGE_EN=0`，红色充电指示灯不属于本阶段验证范围；共享 IO 的双色灯也须在确认电路拓扑后单独验证。
- 验收：在确认蓝灯 GPIO、有效电平、独立/共脚拓扑和引脚无冲突后，由用户构建、烧录，验证未连接、首条连接、最后一条断开时的状态转换，并结合现有连接日志记录结果。
- 阶段完成后等待用户确认，才提交/上传并进入按键阶段。

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

**总体结论：** 四大功能在 8 引脚约束下均可实现，资源充足。

| 功能 | 可行性 | 引脚占用 | 资源占用 |
|---|---|---|---|
| **混响（ECHO）** | ✅ | DAC PA2（TX 本地监听） | Flash +1.5 KB，mram +12 KB |
| **降噪** | ✅ | 0（纯算法） | 已占用，无需新增 |
| **充电** | ✅ | 0（VUSB 检测用内部 ADC，不占用 GPIO） | Flash +0.8 KB |
| **数码管** | ⏸️ 最后评估 | 暂按 TM1650 预留 PA3/PB1 | 待后续确认 |

**引脚分配表：**

| 引脚 | 功能 | 备注 |
|---|---|---|
| PB0 | ADKEY（PP/K1/K2） | 不变 |
| PB3 | UART Debug | 保留打印 |
| PA2 | **DAC 输出** | TX 本地混响监听 |
| PA3 | 预留 | 后续 TM1650 SDA |
| PB1 | 预留 | 后续 TM1650 SCL |
| PA0/PA1/PA4/PB4 | 预留 | 扩展用 |

### 6.2 资源评估（map.txt）

| 区域 | 定义大小 | 当前占用 | 余量 | 说明 |
|---|---|---|---|---|
| **Flash（代码可用）** | 120 KB (`0x1e000`) | 111 KB (`0x1bff3`) | **约 9 KB** | 由 `.flash` 段 0x10008600–0x1001a5ff + `.code_comm`/`.code_adapter`/`.code_emit` 的 LMA 合计 |
| **comm（公共 RAM）** | 27 KB (`0x6c00`) | 26.3 KB (`0x68c0`) | **0.7 KB** | 含 `.code_comm` 11.6 KB + `.data_comm` 15.4 KB；**几乎顶满** |
| **emit（TX 代码+算法缓冲）** | 34 KB (`0x8800`) | 28.4 KB (`0x6fe0`) | **约 5.6 KB** | `.code_emit` 13.5 KB + `.data_emit` 14.9 KB |
| **adapter（RX 代码+缓冲）** | 34 KB (`0x8800`) | 31.6 KB (`0x7abc`) | **约 2.4 KB** | 只影响 RX/适配器，TX 不看这块 |
| **mram（ECHO/Reverb 专用复用）** | 32 KB (`0x8000`) | 0 | **32 KB 全空闲** | 当前 ECHO/Reverb 均未使能，此段未被链接使用 |

### 6.3 实施步骤

#### P0：基线验证

- **改动**：无
- **验证**：TX→RX→耳机链路正常
- **状态**：已完成

#### P1：ECHO 混响 + DAC PA2 监听

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`WIRELESS_MIC_ECHO_EN 0→1`
  - `projects/microphone/config_ab5766_le_mic.h`：`ECHO_DELAY_BUF_SIZE 6000→2000`（防溢出）
  - `projects/microphone/config_ab5766_le_mic.h`：`WIRELESS_MIC_AINS4_32K_EN 1→0`（关降噪，避免干扰）
  - `projects/microphone/port/port_key.c`：K1 双击列 `MSG_NO→MSG_ECHO_LEVEL_UP`
  - `projects/microphone/port/port_key.c`：K2 双击列 `MSG_NO→MSG_ECHO_LEVEL_DOWN`
- **原理**：ECHO 算法在发射端 PACC 上处理，处理后的 PCM 经 `dac0_out_audio_input()` 输出到 PA2（TX 本地监听），同时经 LC3S 编码 + 无线传输到 RX 端解码输出。
- **验证**：TX 耳机和 RX 耳机同时听到混响，K1/K2 双击可调混响等级 0–8
- **状态**：✅ 已完成（用户确认能听到混响）

#### P2：充电检测

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`BSP_CHARGE_EN 0→1`
  - 确认 `xcfg_cb.charge_en=1` 在烧录配置里打开
- **原理**：`bsp_charge_process()` 调用 `charge_detect_dc()` 读 `RTCCON` 寄存器位 20/22 判断 VUSB 插入，状态机驱动充电流程。
- **验证**：插充电器看到 `charge: x` 打印，状态跳转正常，低电检测避开充电期
- **状态**：⏳ 待实现

#### P3：数码管驱动（TM1650）

- **改动**：
  - 新增 `bsp/bsp_7seg.c/h`，用 `driver_iic.c` 的 I2C 接口驱动 TM1650
  - 硬件连接：SDA→PA3、SCL→PB1
  - 在 `app.cbp` 里把 `bsp_7seg.c` 加进构建列表
- **原理**：TM1650 是 I2C 接口的 4 位 7 段数码管驱动芯片，只需 2 根线即可驱动 4 位数码管。
- **验证**：数码管显示电量 0–9、混响等级 0–9、充电状态（全亮/闪烁），无乱码
- **状态**：⏸️ 待评估（硬件未确认）

#### P4：去掉 TX 本地监听

- **改动**：
  - `projects/microphone/config_ab5766_le_mic.h`：`WIRELESS_MIC_DAC_OUT_EN 1→0`
- **原理**：关闭 TX 本地 DAC 输出，仅保留 RX 远端监听，省 1 个 DAC 引脚。
- **验证**：TX 耳机无声音，RX 耳机正常，混响功能正常
- **状态**：⏳ 待实现

### 6.4 风险与注意事项

| 风险 | 影响 | 缓解措施 |
|---|---|---|
| **comm RAM 余量 0.7 KB** | 数码管/充电 buffer 需严格控制 | 数码管用全局变量 < 100 字节，充电复用现有 `charge_status` 结构体 |
| **ECHO 与 MAGIC 互斥** | 后续若开 MAGIC 会挤占 emit 区 | 当前 MAGIC_EN=0，无风险；后续需评估 emit 剩余 5.6 KB 是否够 |
| **ECHO buffer 溢出** | `.data_emit` 溢出 6872 字节 | 已通过减小 `ECHO_DELAY_BUF_SIZE` 到 2000 解决 |
| **mram 区不可执行** | 把 `delay_lbuf` 放 mram 导致 INST ERR | 已恢复 `AT(.buf.echo.delay)`，不再放 mram |

### 6.5 后续扩展建议

- **降噪恢复**：P1 验证混响后，可单独开 `WIRELESS_MIC_AINS4_32K_EN=1` 验证降噪，或换 DNR_FRE（`WIRELESS_MIC_DNR_FRE_EN=1`）节省空间
- **数码管硬件确认**：TM1650 采购后实现 P3
- **按键扩展**：PA0/PA1/PA4/PB4 可留给 KEY4、LED、第二路 ADC 等扩展

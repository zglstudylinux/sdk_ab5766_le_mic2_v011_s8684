# 07 调试排查与设计权衡 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 核心考察：启动日志倒推、分层取证、真实项目坑（AGC 链接失败、Room Reverb 与 ECHO 互斥、USB Mic 录音为 0、解码线程饿死）、架构取舍

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，单固件双角色（TX/RX）。
- 调试工具：UART 1.5 Mbps 日志、ToolKit 在线调参、按键/LED、dump、RMS/频谱。
- 本卷重点：从启动日志/真实案例倒推根因，用“分层取证法”定位硬件/固件/配置问题。
- 核心源码入口：
  - `projects/microphone/main.c:5-22`
  - `bsp/bsp_sys.c:199-267`
  - `modules/wireless/wireless_proc.c:90-295`
  - `modules/voice/room_reverb.c:77-79`
  - `functions/func_adapter.c:33-54`
  - `docs/SDK/AB5766_LE_Mic_音频算法测试快速入门.md`
  - `docs/SDK/算法详解/12_AGC自动增益.md`

---

## 1. 概念理解题

### 题目 7.1 控制面日志证据 ≠ 算法客观指标已通过（★★★★ / 概念理解）

**题目**：解释“控制面日志证据 ≠ 算法客观指标已通过”。如何通过 RMS、频谱、失真三个维度做客观测试？

**考察点**：
- 主考点：测试证据分级；
- 副考点：算法测试方法论；
- 区分点：能否给出“听感 → 控制面日志 → RMS/频谱/失真”三个等级的定义。

**参考答案**：
项目测试纪律要求证据分三级：
1. **听感证据**：耳朵听有没有效果，最弱；
2. **控制面日志证据**：UART 打印显示参数已设置（如 `soft_gain_set_level`、`magic_audio_param_set`），只能证明命令到达；
3. **客观指标**：用固定输入（如 1 kHz 正弦）测 RMS 变化、频谱形状、THD/底噪，才能真正证明算法生效。

**评估标准**：
- ⭐⭐⭐ 能举出三级证据并说明如何量化；
- ⭐⭐ 能说两级；
- ⭐ 只强调听感。

---

## 2. 源码追踪题

### 题目 7.2 从启动日志反推复位原因（★★★★ / 源码追踪）

**题目**：从启动日志某一行 `"Hello AB5766: 00000004"` 出发，反推 `sys_rst_init()` 返回的复位原因可能是什么，以及如何分层验证。

**考察点**：
- 主考点：复位源识别、日志与寄存器对应；
- 副考点：分层验证（寄存器/驱动/应用）；
- 区分点：能否指出 `sys_cb.rst_reason` 与低功耗唤醒源的区别。

**参考答案**：
`main.c:7-12` 中：
```c
sys_cb.rst_reason = sys_rst_init(WK0_10S_RESET);
sys_cb.wakeup_reason = lowpwr_get_wakeup_source();
```
- `sys_cb.rst_reason` 是系统复位原因（上电/看门狗/软件复位等）；
- `sys_cb.wakeup_reason` 是低功耗唤醒源（GPIO/RTC/充电插入等）。

若日志出现 `00000004`，需查 `sys_rst_init` 的位定义（通常在预编译库或头文件）：bit2 可能表示看门狗复位或 GPIO 唤醒。分层验证：
1. 读复位标志寄存器；
2. 加 printf 输出 `sys_cb.rst_reason` 与 `sys_cb.wakeup_reason`；
3. 用示波器/万用表测唤醒源（如 GPIO 电平、充电插入）。

**评估标准**：
- ⭐⭐⭐ 能区分 rst_reason 与 wakeup_reason 并给分层验证；
- ⭐⭐ 只提一个；
- ⭐ 完全没概念。

---

### 题目 7.3 USB Mic 故障排查的四层法（★★★★★ / 源码追踪）

**题目**：USB Mic 故障排查的“四层法”（角色 → 枚举 → PCM → 电脑端）。每层对应什么日志/工具/源码检查点？

**考察点**：
- 主考点：USB Audio 上行链的分层调试；
- 副考点：Adapter 角色与编译宏的关系；
- 区分点：能否给出每层的具体检查点与失败特征。

**参考答案**：

| 层 | 检查点 | 失败特征 |
|---|---|---|
| 角色 | `xcfg_cb.wireless_adapter_en` 是否为 1；`wireless_role_is_adapter()`；`bsp_sys.c:139-147` `dev_init(1)` 是否被调 | 编译期 `ADAPTER_USB_MIC_RX_EN=0` 或运行时不是 Adapter，USB 初始不走 |
| 枚举 | PC 是否看到设备；USB VID/PID；`usb_device_enter(UDE_ENUM_TYPE)`；`UDE_MIC_EN` | PC 看不到设备或识别为未知 |
| PCM | `usb_mic_in_audio_input()` 是否被调（`modules/wireless/mic_proc.c:470-475,504-510`）；`mic_dec.obuf_off == WIRELESS_MIC_SAMPLES_SELECT` | 枚举成功但录音为 0 |
| 电脑端 | Windows 录音设备权限、是否独占、采样率/位深匹配 | 电脑能开录音但波表为 0 或杂音 |

**评估标准**：
- ⭐⭐⭐ 能写全四层并给出每层的检查点、日志、工具；
- ⭐⭐ 能写三层；
- ⭐ 只说“枚举”。

---

## 3. 调试排查题

### 题目 7.4 案例 A：WIRELESS_MIC_AGC_EN=1 链接失败（★★★★★ / 调试排查）

**题目**：把 `WIRELESS_MIC_AGC_EN` 从 0 改为 1 后，链接失败。从 `map.txt` 分析是缺 `.text.agc_proc` 段，如何修复？

**考察点**：
- 主考点：map.txt 链接错误分析、AT 段分配；
- 副考点：`ram.ld` 的 `.code_emit` 与 `.data_emit`；
- 区分点：能否指出“开了算法但链接段没跟上”的典型原因。

**参考答案**：
AGC 相关代码在 `modules/voice/agc.c`（或对应文件）用 `AT(.text.agc_proc)` / `AT(.buf.agc)` 标注。`ram.ld` 必须在 `.code_emit` 和 `.data_emit` 中显式匹配这些段：

```ld
*(.text.agc*)
*(.buf.agc*)
```

若 `ram.ld` 里 AGC 段被 `#if WIRELESS_MIC_AGC_EN` 条件关住，改宏为 1 后链接器找不到落点，报 overflow 或 undefined section。修复：
1. 打开 `ram.ld`，确认 `.code_emit` / `.data_emit` 里 `*(.text.agc*)` 和 `*(.buf.agc*)` 的 `#if` 也打开；
2. 重新构建，检查 `map.txt` 中 `agc` 相关符号地址是否合理。

**评估标准**：
- ⭐⭐⭐ 能准确定位 `ram.ld` 条件段并给出修改建议；
- ⭐⭐ 知道是段问题但不确定哪个文件；
- ⭐ 只说“库有问题”。

---

### 题目 7.5 案例 B：Room Reverb 无效果因 ECHO 互斥（★★★★ / 调试排查）

**题目**：测试 Room Reverb 时听到声音没有混响效果。后来发现是 ECHO 没关（自动 mute）。请推理 `room_reverb.c:77-79` 的 mute 逻辑。

**考察点**：
- 主考点：多音效互斥机制、条件编译；
- 副考点：`#if WIRELESS_MIC_ECHO_EN && WIRELESS_MIC_ROOM_REVERB_EN` 的意图；
- 区分点：能否说出“测试前必须先确认 ECHO 关闭”。

**参考答案**：
`modules/voice/room_reverb.c:77-79`：
```c
#if WIRELESS_MIC_ECHO_EN && WIRELESS_MIC_ROOM_REVERB_EN
    room_reverb_audio_mute_set(1);
#endif
```
当编译时 ECHO 和 Room Reverb 同时开启，Room Reverb 初始化后即被 mute，避免回声+混响叠加造成浑浊。测试 Room Reverb 前必须确认 `WIRELESS_MIC_ECHO_EN=0`，否则永远听不到混响。

**评估标准**：
- ⭐⭐⭐ 能引用行号并解释互斥原因；
- ⭐⭐ 知道互斥但无行号；
- ⭐ 完全没想到。

---

### 题目 7.6 案例 C：USB Mic 录音全为 0（★★★★★ / 调试排查）

**题目**：USB Mic 枚举成功，电脑能看到设备，但录音全是 0。用“分层取证法”逐步排查（说 4-5 步）。

**考察点**：
- 主考点：USB Audio 数据链调试；
- 副考点：`mic_dec_pcm_out` 到 `usb_mic_in_audio_input` 的挂接；
- 区分点：能否给出“看 USB 枚举日志 → 看 PCM 计数 → 看混音输出 → 看 PC 采样”四步。

**参考答案**：
1. **看角色与编译宏**：确认 `ADAPTER_USB_MIC_RX_EN=1`、`wireless_adapter_en=1`、`dev_init(1)` 被调；
2. **看枚举日志**：`usb_device_enter(UDE_ENUM_TYPE)` 是否打印成功；PC 设备管理器是否看到麦克风；
3. **看 PCM 源**：`mic_dec_prco_cb()` 是否被调；`lc3s_dec` 是否产出非零 PCM；用 UART 打印 `mic_dec.obuf` 前几个采样；
4. **看挂接**：`usb_mic_in_audio_input()` 是否被调（`mic_proc.c:470-475`）；`mic_dec.obuf_off` 是否满 120；
5. **看 PC 端**：录音软件采样率/位深是否与 USB 描述符一致（当前 48k/16bit 单声道）。

**评估标准**：
- ⭐⭐⭐ 5 步完整；
- ⭐⭐ 3 步；
- ⭐ 只说“看看电脑”。

---

### 题目 7.7 案例 D：一拖二解码线程饿死（★★★★★ / 调试排查）

**题目**：Adapter 一拖二时偶尔出现解码线程饿死。从 `kick_dec_prio_trans` 投递路径、`thread_dec_proc_msg_cb` 分发、`mic_dec.last_bfi` 状态机三个角度分析。

**考察点**：
- 主考点：异步解码线程可靠性；
- 副考点：双路 BFI/PLC 状态；
- 区分点：能否指出“连续 BFI 导致 PLC 打死循环”或“kick 丢失”等可能。

**参考答案**：
- `kick_dec_prio_trans(idx)` 在 `wireless_txrx_single.c:274-281` 中把解码任务投给 `thread_dec`；
- `thread_dec_proc_msg_cb()` 在 `os/thread_dec.c:4-20` 中按 `MSG_MIC_DEC0/1` 分发到 `mic_dec_prco_cb(idx)`；
- `mic_dec_prco_cb()` 中 `plc_soft_process(pcm, samples, mic_dec.last_bfi[idx] || bfi, idx)` 与 `mic_dec.last_bfi[idx] = bfi`（`mic_proc.c:579-580`）。

可能原因：
1. 连续 BFI 时 `plc_soft_process` 每次都走 concealment，若 PLC 内部有死循环/高耗时，会拖死解码线程；
2. `kick_dec_prio_trans` 若被高优先级中断反复抢占，消息队列溢出/丢失；
3. `mic_dec.last_bfi[idx]` 未正确更新，导致 PLC 认为一直丢帧而错误处理。

**评估标准**：
- ⭐⭐⭐ 能列出 3 个角度并给出合理假设；
- ⭐⭐ 2 个角度；
- ⭐ 只说“可能卡死”。

---

## 4. 设计权衡题

### 题目 7.8 super-loop + 平台伪线程 vs FreeRTOS/Zephyr（★★★★★ / 设计权衡）

**题目**：作为面试官，让候选人评价：“本项目用 super-loop + 平台库伪线程，没有上 FreeRTOS/Zephyr” 的取舍。让候选人从代码量、实时性、维护性三个角度对比。

**考察点**：
- 主考点：RTOS 选型权衡；
- 副考点：音频/无线实时性要求；
- 区分点：能否结合本项目“音频在平台线程、业务在 super-loop”的实际形态。

**参考答案**：
- **代码量**：super-loop 极小，启动简单；FreeRTOS 需要任务栈、heap、调度器，ROM/RAM 占用大。本项目 128K Flash / 61K RAM，RTOS 会很吃紧。
- **实时性**：平台库已把 BT/aupcm/driver 做成线程，音频最严苛的 deadline（2.5ms 帧）由这些线程保证；super-loop 只处理按键/消息/低功耗，实时性要求低。上 FreeRTOS 反而增加调度开销。
- **维护性**：super-loop 状态机清晰，便于定位；但扩展性差，新增并发任务困难。FreeRTOS 任务模型扩展性好，但调试复杂、容易优先级反转。

**评估标准**：
- ⭐⭐⭐ 能三角度对比并结合本项目资源限制；
- ⭐⭐ 两角度；
- ⭐ 只泛泛而谈。

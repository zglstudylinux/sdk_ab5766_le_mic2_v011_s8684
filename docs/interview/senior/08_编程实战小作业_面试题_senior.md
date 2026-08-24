# 08 编程实战小作业 面试题（Senior）

> 难度：★★★★ / ★★★★★
> 岗位：高级嵌入式工程师 / 系统架构方向
> 核心考察：现场设计/改写真实代码，要求在 20~40 分钟内完成

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风。
- 实战环境：候选人可查看工程源码与 `docs/SDK/` 文档，但需独立设计实现。
- 本卷重点：新增算法、解耦 Soft Gain、GPIO 角色判决、异步优先级、ram.ld 改分区。
- 核心源码入口：
  - `modules/wireless/mic_proc.c:277-378`
  - `modules/effect/mic_effect.c:19-103,139-187`
  - `modules/audio/soft_gain.c:110-128`
  - `modules/wireless/wireless_proc.c:5-18,24-32`
  - `driver/driver_gpio.c:15-219`
  - `projects/microphone/ram.ld`

---

## 1. 实战 1：新增"音量自动淡入"音效（30 分钟）

**题目**：在现有 SDK 框架下，新增一个"音量自动淡入"音效（连接成功 0.5s 内从 0% 渐变到 100%）。

**任务要求**：
1. 给出文件路径 + 函数签名 + `AT` 段标注；
2. 给出 `mic_enc_proc_cb()` 中的插入位置；
3. 给出 `config` 宏定义；
4. 估算计算时间复杂度（120 点 × 多少周期）。

**考察点**：
- 主考点：新音效接入现有链；
- 副考点：fade 实现与定时；
- 区分点：能否选择同步/异步+定时器或基于帧计数。

**参考答案要点**：
- 新建 `modules/voice/fade_in.c/h`，宏 `WIRELESS_MIC_FADE_IN_EN`；
- 在 `mic_emit_init()` 中初始化（记录 `sys_cb.mic_alg_en=1` 时刻或帧计数）；
- 在 `mic_enc_proc_cb()` 中 `WIRELESS_MIC_FADE_IN_EN` 分支插入 `fade_in_audio_process(pcm, samples)`；
- fade 基于帧计数：500ms / 2.5ms = 200 帧，每帧增益 = 帧号/200（Q15）；
- 时间复杂度：120 点 × 1 次乘加 ≈ 120 cycles（远低于 PACC EQ）。

**评估标准**：
- ⭐⭐⭐ 能给出完整接入方案 + 复杂度估算；
- ⭐⭐ 能给出框架；
- ⭐ 完全不知道。

---

## 2. 实战 2：解耦 Soft Gain 与 PACC（30 分钟）

**题目**：当前 Soft Gain 与 PACC 强耦合（`loc_mic_pacc_process()` 内逐点调 `soft_gain_proc_16bit()`）。请设计一种解耦方案，让 Soft Gain 可以独立于 EQ_DRC 工作。

**任务要求**：
1. 描述新的调用链；
2. 讨论解耦后的内存/性能影响；
3. 给出回滚方案。

**考察点**：
- 主考点：耦合识别与解耦设计；
- 副考点：内存、性能、可维护性；
- 区分点：能否识别 Q4.6 的"未链接"坑。

**参考答案要点**：
- 把 `soft_gain_proc_16bit()` 从 `loc_mic_pacc_process()` 移出，放回 `mic_enc_proc_cb()` 中 `mic_eq_drc_audio_input()` 之前；
- 新增 `mic_soft_gain_audio_input()` 包装函数；
- 改 `loc_mic_pacc_enable()` 的 IO 格式判断，不再依赖 `WIRELESS_MIC_SO_GAIN_EN`；
- 内存：soft_gain 状态保留在 `.buf.mic_effect` 或新段；性能：PACC 输入格式变为 16bit。

**评估标准**：
- ⭐⭐⭐ 能给出解耦方案 + 性能分析 + 回滚；
- ⭐⭐ 能给出方案；
- ⭐ 完全不知道。

---

## 3. 实战 3：GPIO 跳线判决角色（20 分钟）

**题目**：把 `wireless_mic_role_init()` 改造成"上电时通过 GPIO 跳线判决角色"，而非依赖 `xcfg_cb`。请描述：
1. GPIO 配置（参考 `driver/driver_gpio.c`）；
2. `wireless_mic_role_init()` 的修改；
3. 兼容性（如何向后兼容 `xcfg`）。

**考察点**：
- 主考点：硬件配置替代软件配置；
- 副考点：向后兼容；
- 区分点：能否给出优先级策略。

**参考答案要点**：
1. 选一个 GPIO（如 PA0）配置为上拉输入，`gpio_init()`；
2. `wireless_mic_role_init()` 中：若 `xcfg_cb.wireless_adapter_en` 或 `wireless_mic_emit_en` 已明确，沿用；否则读 GPIO：低电平 = Adapter，高电平 = Mic；
3. GPIO 提供"强制覆盖"选项：通过另一 GPIO 或长按按键进入强制模式。

**评估标准**：
- ⭐⭐⭐ 能给出完整兼容方案；
- ⭐⭐ 能给出基本方案；
- ⭐ 完全不知道。

---

## 4. 实战 4：异步线程优先级反转（30 分钟）

**题目**：现在 AINS4_32K 在低优先级算法线程跑，假设未来要加一个实时性更高的算法（如超声检测），请设计：
1. 多级消息优先级机制；
2. `kick` 宏修改；
3. 资源抢占策略。

**考察点**：
- 主考点：多算法并发调度；
- 副考点：实时性分级；
- 区分点：能否提出抢占或分线程方案。

**参考答案要点**：
1. 定义 `MSG_PRIO_HIGH/MID/LOW` 三级，高实时性算法用 `kick_alg_high_prio_trans()`；
2. `thread_alg.c` 与 `thread_dec.c` 中增加高优先级消息分支，优先处理；
3. 资源抢占：高优先级算法可中断低优先级算法的中间结果保存，或使用独立 PACC/缓冲。

**评估标准**：
- ⭐⭐⭐ 能给出三级优先级与抢占策略；
- ⭐⭐ 能给出优先级；
- ⭐ 完全不知道。

---

## 5. 实战 5：ram.ld 重新分区（15 分钟）

**题目**：当前 RAM 分区 `comm(27K) + comm_adapter/comm_emit(34K) + mram(32K)`。如果新增一个需要 16K RAM 的算法，请重新设计分区并写出 `ram.ld` 修改草稿。

**考察点**：
- 主考点：链接脚本修改；
- 副考点：内存复用；
- 区分点：能否给出可行的新分区。

**参考答案要点**：
- 复用 `mram`（32K）：新增算法 `foo` 与 echo/reverb 互斥，划 `mram_foo` 16K；
- 或压缩 `comm_adapter/comm_emit`：把新增算法放 `.code_adapter/.code_emit` 复用段；
- `ram.ld` 草稿：
  ```ld
  .mram_foo __mram_vma (NOLOAD) : { . = 0x4000; } > mram
  ```

**评估标准**：
- ⭐⭐⭐ 能给出草稿并解释复用；
- ⭐⭐ 能给出大概；
- ⭐ 完全不知道。

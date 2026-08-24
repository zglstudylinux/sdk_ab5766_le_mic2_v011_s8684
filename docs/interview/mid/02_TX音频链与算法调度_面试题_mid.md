# 02 TX 音频链与算法调度 面试题（Mid）

> 难度：★★★
> 岗位：中级嵌入式工程师
> 核心考察：mic_enc_proc_cb 每帧链、AINS4_32K 异步乒乓、SRC、Soft Gain/PACC、LC3S 编码

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风 TX 端。
- 音频链：SDADC(32k) → AINS4_32K(异步乒乓，算法线程) → SRC(32k→48k) → PACC EQ/DRC(Soft Gain 耦合) → [DNR_FRE/ECHO/MAGIC/ROBOT/ROOM_REVERB/AGC 条件编译] → LC3S 编码 → 无线发送。
- 本卷重点：`mic_enc_proc_cb()` 顺序、`ains4_32k_mic_audio_input()` 乒乓缓冲、Soft Gain 与 PACC 的耦合、`lc3s_enc()` 调用点。
- 核心源码入口：
  - `modules/wireless/mic_proc.c:277-378`
  - `modules/voice/ains4_32k.c:58-101`
  - `modules/audio/soft_gain.c:110-128`
  - `modules/effect/mic_effect.c:164-187`
  - `modules/audio/mic_eq_drc.c:19-48`

---

## 1. 概念理解题

### 题目 2.1 TX 当前默认使能的算法（★★★ / 概念理解）

**题目**：列出 TX 当前默认使能的算法（按 `mic_enc_proc_cb()` 内的源码顺序），并指出当前生效的子集与候选但被裁掉的子集。

**考察点**：
- 主考点：条件编译宏与默认配置；
- 副考点：每帧链顺序；
- 区分点：能否区分“当前生效”与“代码存在但被宏关闭”。

**参考答案**：
`mic_enc_proc_cb()` 源码顺序：
1. AINS4_32K（`WIRELESS_MIC_AINS4_32K_EN=1`）；
2. ECHO 32k 路径（`WIRELESS_MIC_ECHO_EN=0` 且 `WIRELESS_MIC_32K_EN=1`，当前关闭）；
3. SRC 32k→48k（`WIRELESS_MIC_32K_EN=1`）；
4. EQ/DRC（`WIRELESS_MIC_EQ_DRC_EN=1`）；
5. DNR_FRE（`WIRELESS_MIC_DNR_FRE_EN=0`）；
6. ECHO 48k 路径（`WIRELESS_MIC_ECHO_EN=0`）；
7. MAGIC（`WIRELESS_MIC_MAGIC_EN=0`）；
8. Robotization（`WIRELESS_MIC_ROBOTIZATION_EN=0`）；
9. Room Reverb（`WIRELESS_MIC_ROOM_REVERB_EN=0`）；
10. AGC（`WIRELESS_MIC_AGC_EN=0`）；
11. LC3S 编码。

当前生效：AINS4_32K、SRC、EQ/DRC（含 Soft Gain）、LC3S 编码。

**评估标准**：
- ⭐⭐⭐ 完整列出并区分生效/被裁；
- ⭐⭐ 只列生效的；
- ⭐ 完全不知道。

---

### 题目 2.2 AINS4_32K 与 EQ_DRC 的处理模型差异（★★★ / 概念理解）

**题目**：AINS4_32K 与 EQ_DRC 都在 TX 链中，但处理模型不同（异步 vs 同步）。请说明这两种模型的差异，并讨论各自的实时性影响。

**考察点**：
- 主考点：同步/异步算法调度；
- 副考点：乒乓缓冲与线程 kick；
- 区分点：能否指出 AINS4_32K 是“攒帧后线程处理”，EQ_DRC 是“当前帧原位处理”。

**参考答案**：
- **AINS4_32K（异步）**：`ains4_32k_mic_audio_input()` 把 SDADC 数据放入乒乓缓冲，满 240 点后 `ains4_32k_mic_proc_kick_start()` 投递到算法线程 `ains4_32k_mic_proc_cb()` 中跑 `ains4_process()`。引入了算法帧级延迟（80 点粒度），但不阻塞音频 ISR。
- **EQ_DRC（同步）**：`mic_eq_drc_audio_input()` 直接在当前帧调 `loc_mic_pacc_process()`，PACC 硬件处理完后立即返回，处理时间在几十微秒级。

**评估标准**：
- ⭐⭐⭐ 能对比并说出延迟与实时性影响；
- ⭐⭐ 能说出异步/同步；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 2.3 SDADC 到 LC3S 编码的完整调用链（★★★ / 源码追踪）

**题目**：从 SDADC DMA 完成中断开始，到 LC3S 编码完成并入 TX 队列，中间经过哪几个函数？请按调用顺序列出文件:行号。

**考察点**：
- 主考点：TX 每帧链调用顺序；
- 副考点：`mic_enc_adc_dma_kick()` 与 `mic_enc_proc_cb()` 的关系；
- 区分点：能否按文件列出关键函数。

**参考答案**：
1. `bsp_sdadc.c:26-37` SDADC DMA half/full ISR → `sdadc_done_proc_kick()`；
2. `bsp_sdadc.c:39-53` AUPCM 线程 `sdadc_proc_cb()` → `mic_enc_proc_cb()`；
3. `modules/wireless/mic_proc.c:277-378` `mic_enc_proc_cb()` 内依次：
   - `ains4_32k_mic_audio_input()`（297-299）；
   - `src_frame_resample()`（305-308）；
   - `mic_eq_drc_audio_input()`（314-316）；
   - `lc3s_enc()`（358-359）；
   - `wireless_d2a_put_tx_frame()`（363）。

**评估标准**：
- ⭐⭐⭐ 完整链 + 行号；
- ⭐⭐ 链正确但无行号；
- ⭐ 完全不知道。

---

### 题目 2.4 AINS4_32K 乒乓缓冲攒帧（★★★ / 源码追踪）

**题目**：AINS4_32K 的乒乓缓冲为什么是 480 字节（FRAME_LEN×2）？每帧回调传入 120 点，但内核帧长 240 点，单次存取 80 点。请推导攒帧的循环次数与延迟。

**考察点**：
- 主考点：乒乓缓冲尺寸与攒帧逻辑；
- 副考点：算法帧与无线帧的换算；
- 区分点：能否算出“每 3 个无线帧触发一次算法”的结论。

**参考答案**：
- 无线帧 120 点（2.5ms），AINS4 内核帧 240 点，输出块 80 点；
- 乒乓缓冲大小 = 240×2 = 480 点（不是字节，是 sample）；
- 每来一个 120 点无线帧，`ains4_32k_mic_audio_input()` 把它拆成 80+40 或 40+80 写入；
- 攒满 240 点后触发一次算法处理，同时输出 240 点（3 个 80 点块）到下游；
- 因此大约每 3 个无线帧（7.5ms）触发一次 AINS4 算法，但输出是连续的 80 点粒度。

**评估标准**：
- ⭐⭐⭐ 能推导循环次数与延迟；
- ⭐⭐ 能算出 3 帧；
- ⭐ 完全不知道。

---

### 题目 2.5 `samples=120` 时的 while 循环（★★★ / 源码追踪）

**题目**：`samples=120` 时，`ains4_32k_mic_audio_input()` 内的 while 循环会跑几次？每次 `PROCESS_OUT_SAMPLES=80` 的限制下，memcpy 的方向是什么？

**考察点**：
- 主考点：`ains4_32k_mic_audio_input()` 内部循环；
- 副考点：输入/输出缓冲的读写方向；
- 区分点：能否说出 120 = 80 + 40 的分段。

**参考答案**：
`ains4_32k_mic_audio_input()` 中 while 循环按 `PROCESS_OUT_SAMPLES=80` 分段：
- 第一次：处理前 80 点，从输入 `ptr` 拷到算法输入缓冲，可能触发算法；
- 第二次：处理剩余 40 点，继续填充；
- 若算法输出就绪，则把输出缓冲的 80 点拷回 `ptr`。

循环最多 2 次（120 = 80 + 40）。

**评估标准**：
- ⭐⭐⭐ 能说出 2 次与方向；
- ⭐⭐ 能说出 2 次；
- ⭐ 完全不知道。

---

## 3. 调试排查题

### 题目 2.6 开启 ECHO 后听感无变化（★★★ / 调试排查）

**题目**：开启 `WIRELESS_MIC_ECHO_EN=1` 后，发现听感没有变化。给出至少 4 个嫌疑原因（配置/算法/资源/链接段），并说出每个的验证手段。

**考察点**：
- 主考点：多维度排查“改宏没生效”；
- 副考点：ECHO 的资源依赖（delay buffer、初始化、链接段）；
- 区分点：能否区分编译期/链接期/运行期问题。

**参考答案**：
1. **配置**：确认 `config_ab5766_le_mic.h` 中宏已改且重新编译；验证：看 `map.txt` 或 `nm` 是否有 `echo_audio_process`；
2. **算法未初始化**：`mic_emit_init()` 中 `echo_audio_init()` 是否被调；验证：加 printf；
3. **资源不足**：`ECHO_DELAY_BUF_SIZE=6000` 是否足够，`mram` 是否被占用；验证：看 `ram.ld` 与 `map.txt`；
4. **链接段**：`AT(.text.echo_proc)` 是否正确落入 `.code_emit`；验证：看 `map.txt` 地址；
5. **运行时被 mute**：`echo_cfg.mute` 是否为 1；验证：打印或内存查看。

**评估标准**：
- ⭐⭐⭐ 4+ 点并分层；
- ⭐⭐ 2-3 点；
- ⭐ 只说“重新烧录”。

---

### 题目 2.7 主频偶尔抬不到 160M（★★★ / 调试排排查）

**题目**：连接时主频从 24M 抬到 160M，但偶尔抬不起来，导致音频卡顿。如何用 UART 日志、`sys_clk_req`、GPIO 翻转三种手段验证？

**考察点**：
- 主考点：时钟切换调试；
- 副考点：`sys_clk_req` 与 `sys_clk_free` 的配对；
- 区分点：能否给出可操作的验证方法。

**参考答案**：
1. **UART 日志**：在 `wireless_proc.c:102-110` 前后加 `printf("clk req 160M\n")` / `printf("clk free\n")`；
2. **`sys_clk_req` 返回值/内部状态**：查 `sys_clk_req` 是否因其他模块持有更高优先级请求而失败；
3. **GPIO 翻转**：在 `sys_clk_req` 前后翻转一个 GPIO，用示波器看是否真的切换主频（可测指令周期变化）。

**评估标准**：
- ⭐⭐⭐ 三种手段都可操作；
- ⭐⭐ 两种；
- ⭐ 只说一句。

---

## 4. 设计权衡题

### 题目 2.8 新增 DNR_FRE 应同步还是异步（★★★ / 设计权衡）

**题目**：当前 AINS4_32K 在内核线程跑，EQ_DRC 在 PACC 硬件跑。如果要新增一个 DNR_FRE，应该放同步还是异步？为什么？

**考察点**：
- 主考点：算法放置位置权衡；
- 副考点：DNR_FRE 的计算量与延迟要求；
- 区分点：能否结合 `dnr_fre.c` 中 `samples==120` 才处理的特点。

**参考答案**：
DNR_FRE 当前实现是同步的：`dnr_fre_mic_audio_input()` 在 `samples==120` 时直接调 `dnr_fre_process()`（`modules/voice/dnr_fre.c:85-90`）。若计算量大，同步会阻塞音频 ISR/线程，导致帧丢失；异步则引入攒帧延迟。

建议：若 DNR_FRE 耗时 > 帧预算（约 200us），应改成类似 AINS4_32K 的异步乒乓模型；若耗时小，保持同步即可。

**评估标准**：
- ⭐⭐⭐ 能给出量化判断标准与改造建议；
- ⭐⭐ 能说出权衡；
- ⭐ 完全不知道。

---

### 题目 2.9 init 顺序与每帧顺序不一致（★★★ / 设计权衡）

**题目**：`mic_enc_proc_cb()` 中 `#if` 顺序与 `mic_emit_init()` 中 init 顺序并不完全一致（如 `allpass_filter_change_set` 只在 init 出现）。这种不对称是 bug 还是设计？

**考察点**：
- 主考点：初始化与运行时路径的差异；
- 副考点：`allpass_filter_change_set` 的调用点；
- 区分点：能否说明“一次性设置”与“每帧处理”的本质区别。

**参考答案**：
这是设计，不是 bug。`allpass_filter_change_set()` 是配置函数，只需在初始化时调一次；若算法未启用，每帧链中不需要调用。`mic_emit_init()` 负责“按配置初始化所有可能用到的模块”，`mic_enc_proc_cb()` 只调用当前编译启用的模块。两者不对称是因为职责不同：init 管“准备”，每帧管“执行”。

**评估标准**：
- ⭐⭐⭐ 能指出职责差异；
- ⭐⭐ 能说出不是 bug；
- ⭐ 完全不知道。

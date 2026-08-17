# 01 系统架构与角色调度 面试题（Junior）

> 难度：★ / ★★
> 岗位：初级嵌入式工程师
> 核心考察：main 入口、super-loop、角色判定基本流程

## 0. 项目上下文速览

- 芯片/项目：AB5766 RISC-V LE Mic 无线麦克风，单固件双角色（TX Mic Emit / RX Adapter）。
- 架构特征：非 RTOS，`main()` → `bsp_sys_init()` → `func_run()` 超级循环。
- 本卷重点：上电流程、角色判定入口、`func_run()` 的基本结构。
- 核心源码入口：
  - `projects/microphone/main.c:5-22`
  - `functions/func.c:134-178`
  - `modules/wireless/wireless_proc.c:5-18`

---

## 1. 概念理解题

### 题目 1.1 `main()` 做哪三件事（★ / 概念理解）

**题目**：`main()` 函数依次调用哪三个关键函数？

**考察点**：
- 主考点：固件入口；
- 副考点：启动顺序；
- 区分点：能否按顺序说出。

**参考答案**：
`projects/microphone/main.c:5-22`：
1. `sys_rst_init(WK0_10S_RESET)`；
2. `lowpwr_get_wakeup_source()`；
3. `bsp_sys_init()`；
4. `func_run()`。

**评估标准**：
- ⭐⭐⭐ 按顺序说出；
- ⭐⭐ 说出 3 个；
- ⭐ 说不出。

---

### 题目 1.2 TX 与 RX 怎么区分（★★ / 概念理解）

**题目**：本工程同一份固件如何区分自己是 TX（麦克风发射端）还是 RX（适配器接收端）？

**考察点**：
- 主考点：角色判定机制；
- 副考点：`xcfg_cb` 字段；
- 区分点：能否说出 `wireless_adapter_en` 与 `wireless_mic_emit_en`。

**参考答案**：
`xcfg_cb.wireless_adapter_en` 为 1 时是 RX/Adapter；否则 `xcfg_cb.wireless_mic_emit_en` 为 1 时是 TX/Mic Emit；都关时默认 TX。`wireless_mic_role_init()` 根据这两个字段设置 `cfg_wireless_role`。

**评估标准**：
- ⭐⭐⭐ 能说出两个字段与优先级；
- ⭐⭐ 能说出大概；
- ⭐ 完全不知道。

---

## 2. 源码追踪题

### 题目 1.3 `func_run()` 的基本结构（★★ / 源码追踪）

**题目**：请描述 `func_run()` 的基本结构（ while 循环里做什么）。

**考察点**：
- 主考点：super-loop；
- 副考点：`func_enter()` 与 `switch(func_cb.sta)`；
- 区分点：能否说出 `func_enter()` 负责键表绑定。

**参考答案**：
`functions/func.c:134-178`：
```c
while (1) {
    func_enter();               // 同步参数、绑定按键表
    switch (func_cb.sta) {
        case FUNC_MIC_EMIT: func_mic_emit(); break;
        case FUNC_ADAPTER:  func_adapter();  break;
        case FUNC_PWROFF:   func_pwroff();   break;
    }
}
```

**评估标准**：
- ⭐⭐⭐ 能写出结构并解释；
- ⭐⭐ 能写出大概；
- ⭐ 完全不知道。

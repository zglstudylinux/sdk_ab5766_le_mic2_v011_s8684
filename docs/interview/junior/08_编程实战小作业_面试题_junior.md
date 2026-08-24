# 08 编程实战小作业 面试题（Junior）

> 难度：★ / ★★
> 岗位：初级嵌入式工程师
> 核心考察：简单的配置修改与日志调试

## 实战 1：打开一个算法开关（15 分钟）

**题目**：要开启 ECHO 混响效果，需要改哪个宏，值改成什么？

**参考答案**：
改 `projects/microphone/config_ab5766_le_mic.h` 中的 `WIRELESS_MIC_ECHO_EN`，从 `0` 改为 `1`。

**评估标准**：
- ⭐⭐⭐ 准确定位；
- ⭐⭐ 说出大概；
- ⭐ 不知道。

## 实战 2：加一个打印（15 分钟）

**题目**：在 TX 的 `mic_enc_proc_cb()` 中加一条打印，每次进入该函数输出 `"TX frame processed"`。

**参考答案**：
在 `modules/wireless/mic_proc.c` 第 278 行函数体开始处加 `printf("TX frame processed\n");`。

**评估标准**：
- ⭐⭐⭐ 能定位位置；
- ⭐⭐ 能说出大概；
- ⭐ 不知道。

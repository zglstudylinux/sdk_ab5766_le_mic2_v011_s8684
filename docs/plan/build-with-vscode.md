# 在 VSCode 中用命令行编译 AB5766 固件

> 适用仓库：`app_branch02`（产品配置 `CONFIG_AB5766_LE_MIC`，编译器配置 `riscv32-v3`）。
> 目标：脱离 Code::Blocks GUI，用 PowerShell 脚本 + VSCode 任务在命令行完成与 GUI 等价的构建，并提供 IntelliSense（跳转/补全）。

## 1. 背景

此前固件只能在 Code::Blocks GUI 里编译。`.cbp` 工程文件是唯一的构建描述，仓库内没有任何 Makefile / `.vscode/` / CI 配置，脱离 GUI 就无法构建。本方案提供一套命令行构建脚本，并在 VSCode 里配置 `Ctrl+Shift+B` 一键编译与 IntelliSense，使日常开发不必再开 Code::Blocks。

### 1.1 调研结论

- 工程引用编译器配置 `riscv32-v3`，其 master_path 记录在 `%APPDATA%\codeblocks\default.conf`：本机实际工具链在 `D:\software_download\work\rv32-toolchain_v1.5.9\rv32-toolchain_v1.5.9\install\RV32-V3\bin`。
- 该 bin 目录含 `riscv32-elf-gcc/ld/objcopy/xmaker`，**但无 objdump**；系统 PATH 里的工具链条目是失效的（指向不存在的 `install\bin`）。因此脚本必须用绝对路径定位工具链，并把 bin 加入 `$env:Path`（`prebuild.bat`/`postbuild.bat` 用裸名调用工具）。
- 构建存在隐式顺序：`prebuild`（生成 `effect.c/h`、`res.bin/xcfg.bin`、`res.h/xcfg.h`）→ 预处理 `ram.ld`→`ram.o`、`app.xm`→`appxm.o` → 编译所有 `.c` → 链接 `app.rv32` → `postbuild`（生成 `app.bin/app.dcf/download 镜像`，可选 `app.lst`）。
- 产品配置由 `config.h` 的 `USER_CONFIG` 选择 `CONFIG_AB5766_LE_MIC`，无需命令行 `-D`。
- `objdump` 缺失 → `app.lst` 不生成（postbuild 静默跳过，非致命）。

## 2. 交付物

| 文件 | 作用 |
| --- | --- |
| [projects/microphone/build.ps1](../../projects/microphone/build.ps1) | 主构建脚本，解析 `app.cbp` 并复现 CB 的 Debug 构建流程 |
| [.vscode/tasks.json](../../.vscode/tasks.json) | `build` / `clean` / `rebuild` 任务，`build` 绑定 `Ctrl+Shift+B` |
| [.vscode/c_cpp_properties.json](../../.vscode/c_cpp_properties.json) | IntelliSense 配置（include 路径、compilerPath、intelliSenseMode） |
| `docs/plan/build-with-vscode.md` | 本文档 |

复用、不修改的现有文件：`projects/microphone/app.cbp`（源清单与 flags 来源）、`projects/microphone/Output/bin/prebuild.bat`、`projects/microphone/Output/bin/postbuild.bat`、`projects/microphone/ram.ld`、`projects/microphone/Output/bin/app.xm`。构建产物路径与 Code::Blocks 完全一致，便于双路径混用。

## 3. 快速使用

### 3.1 命令行

在仓库根或任意目录执行（脚本内部会 `Set-Location` 到 `projects\microphone`）：

```powershell
# 全量构建
powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1

# 清理（仅清理，不构建）
powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1 -Clean

# 清理 + 构建
powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1 -Rebuild

# 显式指定工具链根目录（绕过自动探测）
powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1 -ToolchainRoot D:\...\RV32-V3
```

也可设置环境变量 `RV32_TOOLCHAIN_ROOT` 指向工具链根目录，脚本会优先采用。

### 3.2 VSCode 任务

- `Ctrl+Shift+B` → 执行 `build` 任务（默认构建任务）。
- 命令面板 → `Tasks: Run Task` → 选择 `clean` 或 `rebuild`。
- 输出面板的编译错误可点击跳转源码（`problemMatcher: $gcc`）。

### 3.3 期望产物

构建成功后 `projects/microphone/Output/bin/` 下生成：

- `app.rv32` —— 链接后的 ELF（约 192 KB，与 GUI 构建一致）
- `app.bin` —— objcopy 生成的裸二进制（约 110 KB）
- `app.dcf` —— xmaker 生成的下载镜像（约 120 KB CODE SIZE）
- `map.txt` —— 链接 map
- `Output/obj/` 下各级 `.o` —— 中间产物

`app.lst`（反汇编）仅在工具链含 `objdump.exe` 时生成；本机 RV32-V3 缺失该工具，故不生成（非致命）。

## 4. 构建流程详解（与 Code::Blocks 对齐）

build.ps1 严格复现 `app.cbp` 描述的 Debug 构建顺序：

1. **工具链探测**：按优先级自动定位工具链根目录（详见 §5），把 `bin` 前置进 `$env:Path`。
2. **prebuild**：`cmd /c "Output\bin\prebuild.bat app"`。复用现有脚本，生成 `res.bin/xcfg.bin`，并把 `effect.c/effect.h` 复制到项目根、`res.h/xcfg.h` 放到工程目录。
3. **解析 app.cbp**：用 `[xml]` 加载，枚举所有 `Unit` 节点，仅取含子节点 `<Option compilerVar="CC">` 的——这天然排除 `app.xm`/`download.xm`/`xcfg.xm`/`res.xm`/`ram.ld`（它们是 `compile="1"` 但无 `compilerVar="CC"`）。每个源的 `.o` 输出路径 = `Output\obj\<相对仓库根的源路径去 .c 加 .o>`。
4. **预处理 ram.ld**：`riscv32-elf-gcc $CFLAGS $INCLUDES -E -P -x c -c ram.ld -o Output\obj\ram.o`。
5. **预处理 app.xm**：`riscv32-elf-gcc $CFLAGS $INCLUDES -E -P -x c -c Output\bin\app.xm -o Output\bin\appxm.o`。
6. **编译所有 .c**：循环 `riscv32-elf-gcc $CFLAGS $INCLUDES -c <src> -o <obj>`，每条检查 `$LASTEXITCODE`，非 0 立即报错退出。
7. **链接**：`riscv32-elf-ld -o Output\bin\app.rv32 <所有 .o，按 cbp 顺序> -TOutput\obj\ram.o --gc-sections -Map=Output\bin\map.txt --no-warn-rwx-segments <libs>`（参数顺序与编译 log 一致）。
8. **postbuild**：`cmd /c "Output\bin\postbuild.bat app"`。复用现有脚本，生成 `app.bin`、`app.dcf`、download 镜像；`C:\upload\upload.bat` 不存在则跳过上传；objdump 不存在则跳过 `app.lst`。

### 4.1 关键常量（取自 app.cbp，工作目录 = projects\microphone）

- **CFLAGS**：`-Os -Wall -march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_xbs1 -Wno-array-bounds -Wno-address-of-packed-member -ffunction-sections -mjump-tables-in-text`
- **INCLUDES**（18 个 `-I`）：`-I. -I..\..\header -I..\..\libs -I..\..\libs\ble -I..\..\libs\cpu -I..\..\libs\usb -I..\..\driver -I..\..\modules\fota -I..\..\modules\utils -I..\..\modules\ble -I..\..\modules\audio\dec -Iproduction_test -Iproduction_test\iodm -Iproduction_test\tbox -I..\..\functions -I..\..\bsp -I..\..\modules -I..\..\modules\wireless`
- **LIBS**：`..\..\libs\cpu\libplatform.a ..\..\libs\cpu\libm.a ..\..\libs\cpu\libc.a ..\..\libs\cpu\libgcc.a ..\..\libs\ble\libbtstack.a`

### 4.2 .o 路径映射规则

| 源文件（相对 projects\microphone） | .o 输出 |
| --- | --- |
| `../../bsp/bsp_charge.c` | `Output\obj\bsp\bsp_charge.o` |
| `main.c` | `Output\obj\projects\microphone\main.o` |
| `port/port_key.c` | `Output\obj\projects\microphone\port\port_key.o` |

与 `log/compile_log.txt` 完全一致。脚本据此为每个 `.o` 的父目录自动 `New-Item -ItemType Directory -Force`。

### 4.3 清理范围（-Clean / -Rebuild）

- 删除：`Output\obj\`（递归）+ `Output\bin\` 下 `app.rv32, app.bin, app.dcf, app.lst, appxm.o, map.txt`。
- 保留：`res.bin, xcfg.bin, res.h, xcfg.h, effect.c, effect.h, *.xm, header.bin, updater.bin, unpack.bin, res\, Settings\, *.bat, ringtone.*`。

`-Clean` 后不自动构建；`-Rebuild` = 清理后再构建。

## 5. 工具链定位

脚本按以下优先级探测 RV32 工具链根目录（含 `bin\riscv32-elf-gcc.exe`）：

1. 参数 `-ToolchainRoot`；
2. 环境变量 `RV32_TOOLCHAIN_ROOT`；
3. 解析 `%APPDATA%\codeblocks\default.conf` 中 `riscv32_v3` 编译器集的 `MASTER_PATH`（CDATA）；
4. 硬编码本机历史路径 `D:\software_download\work\rv32-toolchain_v1.5.9\rv32-toolchain_v1.5.9\install\RV32-V3`；
5. 旧默认 `C:\Program Files (x86)\RV32-Toolchain\RV32-V3`。

探测到后校验 `bin\riscv32-elf-gcc.exe` 存在，并把 `bin` 前置进 `$env:Path`，使 `prebuild.bat`/`postbuild.bat` 的裸名工具调用（`riscv32-elf-xmaker`、`riscv32-elf-objcopy`）可解析。

> 若 Code::Blocks 安装在非默认路径，或工具链未登记到 default.conf，请用 `-ToolchainRoot` 或环境变量显式指定。

## 6. IntelliSense 配置说明

`.vscode/c_cpp_properties.json`（配置名 `AB5766`）：

- `includePath`：将 build.ps1 的 18 个 `-I` 路径转为相对 `${workspaceFolder}` 的形式。
- `compilerPath`：默认指向探测到的 RV32-V3 路径 `D:/.../RV32-V3/bin/riscv32-elf-gcc.exe`；安装位置不同请改此字段。
- `compilerArgs`：`-march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_xbs1 -ffunction-sections -mjump-tables-in-text`，确保 `__riscv_*` 等架构宏被正确展开。
- `intelliSenseMode`: `gcc-x86`（rv32 为 32 位 ilp32，C/C++ 扩展无 riscv 专用模式，x86 最接近）。
- `cStandard`: `c11`；纯 C 工程，未设 cppStandard 的 C++ 特定值。
- 不加额外 `defines`：产品选择已在 `config.h` 内部用 `USER_CONFIG` 完成，IntelliSense 可自行展开。

## 7. 排错

| 现象 | 原因 / 处理 |
| --- | --- |
| `未找到 RV32 工具链` | 用 `-ToolchainRoot` 指定，或设 `RV32_TOOLCHAIN_ROOT` 环境变量。确认 `bin\riscv32-elf-gcc.exe` 存在。 |
| prebuild 报 `riscv32-elf-xmaker 不是内部或外部命令` | 工具链 bin 未进 PATH；脚本已自动前置，若手动调用 .bat 需先 `set PATH=<bin>;%PATH%`。 |
| 编译报找不到头文件 | 确认从 `projects\microphone` 目录调用，`-I` 路径基于此目录。脚本内部已 `Set-Location`。 |
| 链接报 `undefined reference` | 多为源文件未加入 `.cbp`；脚本以 .cbp 为唯一来源，请先在 Code::Blocks 里加入该源再重建。 |
| 无 `app.lst` | RV32-V3 缺 `objdump.exe`，postbuild 静默跳过。如需反汇编，安装含 objdump 的 RV32 工具链并放入 PATH。 |
| `C:\upload\upload.bat` 报错 | 该上传脚本由硬件烧录流程提供，不存在则 postbuild 自动跳过；存在且需上传时由其自身行为决定。 |
| 与 GUI 构建产物大小略有差异 | effect.c/effect.h 由 prebuild 根据 res/xcfg 重新生成，内容可能随资源改动微调；链接产物在量级一致即正常。 |

## 8. 验证清单

1. 在 VSCode 终端运行 `powershell -ExecutionPolicy Bypass -File projects/microphone/build.ps1`，期望：prebuild 打印 `make hfile/cfgfile successful` → 预处理 → 约 90 个 `.c` 编译 0 错 0 警 → 链接 `Output file is ... app.rv32 with size ~192 KB` → postbuild `CODE SIZE: 120 KB`、生成 `app.dcf`。
2. 对比 `log/compile_log.txt`：产物 `app.rv32`/`app.bin`/`app.dcf`/`map.txt` 大小与 CB 构建一致（rv32≈196968 B、bin≈110611 B 量级，允许 effect/xcfg 变更导致的微小差异）。
3. `Ctrl+Shift+B` 触发 build 任务，输出面板可点编译错误跳转源码。
4. `./build.ps1 -Clean` 后 `Output\obj\` 与 app.* 产物被清除，预置 bin 与 res/xcfg/effect 生成物保留；再跑 `-Rebuild` 全量重建成功。
5. IntelliSense：打开 `bsp/bsp_sys.c` 等文件，`#include` 不报红、可跳转到 SDK 头文件。

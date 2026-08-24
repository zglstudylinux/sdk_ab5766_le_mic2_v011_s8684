<#
.SYNOPSIS
    AB5766 麦克风固件命令行构建脚本（脱离 Code::Blocks GUI）。

.DESCRIPTION
    解析 projects/microphone/app.cbp，复现 Code::Blocks 的 Debug 构建流程：
    prebuild -> 预处理 ram.ld/app.xm -> 编译所有 .c -> 链接 app.rv32 -> postbuild。
    构建产物路径与 Code::Blocks 完全一致，可与 GUI 构建混用。

.PARAMETER ToolchainRoot
    RV32 工具链根目录（含 bin\ 下 riscv32-elf-*.exe）。不指定则按优先级自动探测：
    1. 环境变量 RV32_TOOLCHAIN_ROOT
    2. Code::Blocks 全局配置 %APPDATA%\codeblocks\default.conf 中 riscv32_v3 的 master_path
    3. 本机历史路径 D:\...\rv32-toolchain_v1.5.9\install\RV32-V3
    4. 旧默认 C:\Program Files (x86)\RV32-Toolchain\RV32-V3

.PARAMETER Clean
    清理中间与产物（Output\obj\ 及 Output\bin\ 下 app.* 等），保留 prebuild 生成物与预置 bin。
    仅清理，不构建。

.PARAMETER Rebuild
    先清理再构建。

.EXAMPLE
    ./build.ps1
    ./build.ps1 -Clean
    ./build.ps1 -Rebuild
    ./build.ps1 -ToolchainRoot D:\...\RV32-V3
#>
[CmdletBinding()]
param(
    [string]$ToolchainRoot,
    [switch]$Clean,
    [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'

# ----------------------------------------------------------------------
# 定位脚本所在目录（projects\microphone）并切换为工作目录，
# 使相对路径基准与 app.cbp 一致。
# ----------------------------------------------------------------------
$ProjDir = $PSScriptRoot
Set-Location $ProjDir
$CbpFile  = Join-Path $ProjDir 'app.cbp'
$ObjDir   = Join-Path $ProjDir 'Output\obj'
$BinDir   = Join-Path $ProjDir 'Output\bin'
$AppRv32  = Join-Path $BinDir 'app.rv32'

function Write-Stage([string]$msg) { Write-Host "`n==== $msg ====" -ForegroundColor Cyan }
function Write-OK([string]$msg)    { Write-Host "  [OK] $msg" -ForegroundColor Green }
function Fail([string]$msg) {
    Write-Host "  [FAIL] $msg" -ForegroundColor Red
    exit 1
}

# ----------------------------------------------------------------------
# 工具链探测
# ----------------------------------------------------------------------
function Find-ToolchainRoot([string]$Override) {
    if ($Override) { return $Override }

    # 1. 环境变量
    if ($env:RV32_TOOLCHAIN_ROOT -and (Test-Path (Join-Path $env:RV32_TOOLCHAIN_ROOT 'bin\riscv32-elf-gcc.exe'))) {
        return $env:RV32_TOOLCHAIN_ROOT
    }

    # 2. Code::Blocks 全局配置（user_sets -> riscv32_v3 -> MASTER_PATH）
    $cbConf = Join-Path $env:APPDATA 'codeblocks\default.conf'
    if (Test-Path $cbConf) {
        $conf = Get-Content $cbConf -Raw
        # 截取 <riscv32_v3>...</riscv32_v3> 块，取其中第一个 MASTER_PATH 的 CDATA 值
        if ($conf -match '(?s)<riscv32_v3>(.*?)</riscv32_v3>') {
            $block = $Matches[1]
            if ($block -match '<MASTER_PATH>\s*<str>\s*<!\[CDATA\[(.*?)\]\]>\s*</str>') {
                $mp = $Matches[1].Trim()
                if ($mp -and (Test-Path (Join-Path $mp 'bin\riscv32-elf-gcc.exe'))) {
                    return $mp
                }
            }
        }
    }

    # 3. 本机历史安装路径
    $legacy = 'D:\software_download\work\rv32-toolchain_v1.5.9\rv32-toolchain_v1.5.9\install\RV32-V3'
    if (Test-Path (Join-Path $legacy 'bin\riscv32-elf-gcc.exe')) { return $legacy }

    # 4. 旧默认路径
    $old = 'C:\Program Files (x86)\RV32-Toolchain\RV32-V3'
    if (Test-Path (Join-Path $old 'bin\riscv32-elf-gcc.exe')) { return $old }

    return $null
}

$ToolchainRoot = Find-ToolchainRoot $ToolchainRoot
if (-not $ToolchainRoot -or -not (Test-Path (Join-Path $ToolchainRoot 'bin\riscv32-elf-gcc.exe'))) {
    Write-Host "未找到 RV32 工具链。请用 -ToolchainRoot 指定，或设置环境变量 RV32_TOOLCHAIN_ROOT。" -ForegroundColor Red
    Write-Host "示例: ./build.ps1 -ToolchainRoot D:\...\RV32-V3" -ForegroundColor Yellow
    exit 1
}
$BinToolchain = Join-Path $ToolchainRoot 'bin'
$Gcc     = Join-Path $BinToolchain 'riscv32-elf-gcc.exe'
$Ld      = Join-Path $BinToolchain 'riscv32-elf-ld.exe'
$Objcopy = Join-Path $BinToolchain 'riscv32-elf-objcopy.exe'
$Xmaker  = Join-Path $BinToolchain 'riscv32-elf-xmaker.exe'

# 把工具链 bin 前置进 PATH，使 prebuild.bat / postbuild.bat 的裸名工具调用可解析
$env:Path = $BinToolchain + [System.IO.Path]::PathSeparator + $env:Path
Write-Host "工具链: $ToolchainRoot" -ForegroundColor DarkGray
Write-Host "工程目录: $ProjDir" -ForegroundColor DarkGray

# 仓库根（projects\microphone 的上两级），用于推算 .o 输出路径
$RepoRoot = (Resolve-Path (Join-Path $ProjDir '..\..')).Path

# ----------------------------------------------------------------------
# 清理
# ----------------------------------------------------------------------
function Invoke-Clean {
    Write-Stage 'Clean'
    if (Test-Path $ObjDir) {
        Remove-Item $ObjDir -Recurse -Force
        Write-OK "已删除 Output\obj"
    } else {
        Write-Host '  Output\obj 不存在，跳过'
    }
    $binCleanups = @('app.rv32','app.bin','app.dcf','app.lst','appxm.o','map.txt')
    foreach ($f in $binCleanups) {
        $p = Join-Path $BinDir $f
        if (Test-Path $p) { Remove-Item $p -Force; Write-OK "已删除 $f" }
    }
    Write-Host '保留: res.bin/xcfg.bin/res.h/xcfg.h/effect.c/effect.h/*.xm/header.bin/updater.bin/unpack.bin/res\/*.bat/ringtone.*' -ForegroundColor DarkGray
}

if ($Clean) { Invoke-Clean; exit 0 }
if ($Rebuild) { Invoke-Clean }

# 确保输出目录存在
New-Item -ItemType Directory -Force -Path $ObjDir | Out-Null
New-Item -ItemType Directory -Force -Path $BinDir | Out-Null

# 编译器选项与 include 路径（取自 app.cbp，相对 projects\microphone）
$CFLAGS = @(
    '-Os','-Wall','-march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_xbs1',
    '-Wno-array-bounds','-Wno-address-of-packed-member',
    '-ffunction-sections','-mjump-tables-in-text'
)
$INCLUDES = @(
    '-I.','-I..\..\header','-I..\..\libs','-I..\..\libs\ble','-I..\..\libs\cpu','-I..\..\libs\usb',
    '-I..\..\driver','-I..\..\modules\fota','-I..\..\modules\utils','-I..\..\modules\ble',
    '-I..\..\modules\audio\dec','-Iproduction_test','-Iproduction_test\iodm','-Iproduction_test\tbox',
    '-I..\..\functions','-I..\..\bsp','-I..\..\modules','-I..\..\modules\wireless'
)
$LIBS = @(
    '..\..\libs\cpu\libplatform.a','..\..\libs\cpu\libm.a','..\..\libs\cpu\libc.a',
    '..\..\libs\cpu\libgcc.a','..\..\libs\ble\libbtstack.a'
)

# ----------------------------------------------------------------------
# 1) prebuild（生成 res.bin/xcfg.bin、项目根 effect.c/effect.h/res.h/xcfg.h）
#    必须先于 .cbp 解析：effect.c 由 prebuild 生成，解析阶段需校验其存在。
# ----------------------------------------------------------------------
Write-Stage 'Prebuild'
$prebuild = Join-Path $BinDir 'prebuild.bat'
if (-not (Test-Path $prebuild)) { Fail "缺少 $prebuild" }
# prebuild.bat 内部 cd /d %~dp0 自定位到 Output\bin，传 'app' 参数避免失败时 pause 挂起
cmd /c "`"$prebuild`" app"
if ($LASTEXITCODE -ne 0) { Fail "prebuild 失败 (exit $LASTEXITCODE)" }
Write-OK 'prebuild 完成'

# ----------------------------------------------------------------------
# 2) 解析 app.cbp：提取所有 compilerVar="CC" 的源文件
#    （排除 app.xm/download.xm/xcfg.xm/ram.ld —— 它们是 compile="1" 但无 compilerVar="CC"）
# ----------------------------------------------------------------------
Write-Stage '解析 app.cbp'
if (-not (Test-Path $CbpFile)) { Fail "找不到工程文件 $CbpFile" }
[xml]$cbp = Get-Content $CbpFile
$units = $cbp.CodeBlocks_project_file.Project.Unit
if (-not $units) { Fail 'app.cbp 未包含任何 Unit' }

$Sources = New-Object System.Collections.Generic.List[object]
foreach ($u in $units) {
    $isCC = $false
    if ($u.Option) {
        foreach ($opt in @($u.Option)) {
            if ($opt.compilerVar -eq 'CC') { $isCC = $true; break }
        }
    }
    if (-not $isCC) { continue }
    $rel = $u.filename
    if (-not $rel) { continue }
    # 用 GetFullPath 做词法归一化（不要求文件已存在），再 Test-Path 校验
    $srcAbs = [System.IO.Path]::GetFullPath((Join-Path $ProjDir $rel))
    if (-not (Test-Path $srcAbs)) { Fail "源文件不存在: $rel (-> $srcAbs)" }
    # 相对仓库根，用于推算 .o 路径（与 Code::Blocks 一致）
    $relRoot = $srcAbs.Substring($RepoRoot.Length + 1)
    $objRel  = $relRoot -replace '\.c$', '.o'
    $objPath = Join-Path $ObjDir $objRel
    $Sources.Add([pscustomobject]@{ Src = $srcAbs; Obj = $objPath; Rel = $relRoot })
}
Write-OK "共 $($Sources.Count) 个源文件待编译"

# ----------------------------------------------------------------------
# 3) 预处理 ram.ld -> ram.o（链接脚本）
# ----------------------------------------------------------------------
Write-Stage '预处理链接脚本 ram.ld -> ram.o'
$ramLd   = Join-Path $ProjDir 'ram.ld'
$ramObj  = Join-Path $ObjDir 'ram.o'
if (-not (Test-Path $ramLd)) { Fail "缺少 $ramLd" }
& $Gcc $CFLAGS $INCLUDES '-E' '-P' '-x' 'c' '-c' $ramLd '-o' $ramObj
if ($LASTEXITCODE -ne 0) { Fail "预处理 ram.ld 失败 (exit $LASTEXITCODE)" }
Write-OK "ram.o 已生成"

# ----------------------------------------------------------------------
# 4) 预处理 app.xm -> appxm.o（postbuild 的 xmaker 输入）
# ----------------------------------------------------------------------
Write-Stage '预处理 app.xm -> appxm.o'
$appXm   = Join-Path $BinDir 'app.xm'
$appxmO  = Join-Path $BinDir 'appxm.o'
if (-not (Test-Path $appXm)) { Fail "缺少 $appXm" }
& $Gcc $CFLAGS $INCLUDES '-E' '-P' '-x' 'c' '-c' $appXm '-o' $appxmO
if ($LASTEXITCODE -ne 0) { Fail "预处理 app.xm 失败 (exit $LASTEXITCODE)" }
Write-OK 'appxm.o 已生成'

# ----------------------------------------------------------------------
# 5) 编译所有 .c -> .o
# ----------------------------------------------------------------------
Write-Stage "编译 $($Sources.Count) 个源文件"
$idx = 0
foreach ($s in $Sources) {
    $idx++
    $dir = Split-Path $s.Obj -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    Write-Host ("  [{0,3}/{1}] {2}" -f $idx, $Sources.Count, $s.Rel) -ForegroundColor DarkGray
    & $Gcc $CFLAGS $INCLUDES '-c' $s.Src '-o' $s.Obj
    if ($LASTEXITCODE -ne 0) { Fail "编译失败: $($s.Rel) (exit $LASTEXITCODE)" }
}
Write-OK '全部源文件编译完成'

# ----------------------------------------------------------------------
# 6) 链接 -> app.rv32
# ----------------------------------------------------------------------
Write-Stage '链接 app.rv32'
# 链接参数用相对 projects\microphone 的路径，与 Code::Blocks 编译 log 完全一致（map.txt 内路径表现形式相同）
$objListRel = $Sources | ForEach-Object { Join-Path 'Output\obj' ($_.Rel -replace '\.c$','.o') }
# 参数顺序对齐编译 log：<.o 列表> -T ram.o --gc-sections -Map=... --no-warn-rwx-segments <libs>
& $Ld '-o' 'Output\bin\app.rv32' $objListRel '-TOutput\obj\ram.o' '--gc-sections' '-Map=Output\bin\map.txt' '--no-warn-rwx-segments' $LIBS
if ($LASTEXITCODE -ne 0) { Fail "链接失败 (exit $LASTEXITCODE)" }
$size = (Get-Item $AppRv32).Length / 1KB
Write-OK ("app.rv32 已生成 ({0:N2} KB)" -f $size)

# ----------------------------------------------------------------------
# 7) postbuild（生成 app.bin/app.dcf/download 镜像，可选 app.lst/上传）
# ----------------------------------------------------------------------
Write-Stage 'Postbuild'
$postbuild = Join-Path $BinDir 'postbuild.bat'
if (-not (Test-Path $postbuild)) { Fail "缺少 $postbuild" }
cmd /c "`"$postbuild`" app"
if ($LASTEXITCODE -ne 0) { Fail "postbuild 失败 (exit $LASTEXITCODE)" }

Write-Stage '构建成功'
$dcf = Join-Path $BinDir 'app.dcf'
if (Test-Path $dcf) {
    Write-Host ("  产物: app.rv32 ({0:N2} KB)" -f ((Get-Item $AppRv32).Length/1KB)) -ForegroundColor Green
    Write-Host ("        app.bin ({0:N2} KB)" -f ((Get-Item (Join-Path $BinDir 'app.bin')).Length/1KB)) -ForegroundColor Green
    Write-Host ("        app.dcf ({0:N2} KB)" -f ((Get-Item $dcf).Length/1KB)) -ForegroundColor Green
}
Write-Host '  提示: objdump 缺失时不生成 app.lst（非致命）' -ForegroundColor DarkGray
exit 0

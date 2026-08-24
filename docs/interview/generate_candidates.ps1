$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path "docs\interview\senior\候选人版", "docs\interview\mid\候选人版", "docs\interview\junior\候选人版" | Out-Null

function Process-Level {
    param([string]$level)
    Get-ChildItem "docs\interview\$level\*_面试题_$level.md" | Where-Object { $_.Name -notlike "*候选人版*" } | ForEach-Object {
        $src = Get-Content $_.FullName -Raw -Encoding UTF8
        $base = $_.BaseName -replace "_$level$", "_$level`_候选人版"
        $dst = "docs\interview\$level\候选人版\$base.md"

        $src = $src -replace '(?s)\*\*参考答案\*\*.*?(?=(###|\Z))', ''
        $src = $src -replace '(?s)\*\*评估标准\*\*.*?(?=(###|\Z))', ''

        if ($src -notmatch '候选人版') {
            $src = $src -replace '(> 难度：[^\r\n]+)', "`$1`r`n> 候选人版（先自行作答，无答案）"
        }

        Set-Content -Path $dst -Value $src -Encoding UTF8
        Write-Host "Created: $dst"
    }
}

Process-Level -level "senior"
Process-Level -level "mid"
Process-Level -level "junior"

Write-Host "`n=== Validation ==="
$seniorCount = (Get-ChildItem "docs\interview\senior\候选人版\*.md" -ErrorAction SilentlyContinue).Count
$midCount = (Get-ChildItem "docs\interview\mid\候选人版\*.md" -ErrorAction SilentlyContinue).Count
$juniorCount = (Get-ChildItem "docs\interview\junior\候选人版\*.md" -ErrorAction SilentlyContinue).Count
Write-Host "Senior: $seniorCount"
Write-Host "Mid: $midCount"
Write-Host "Junior: $juniorCount"
Write-Host "Total: $($seniorCount + $midCount + $juniorCount)"

Write-Host "`n=== Check for leaked content ==="
Get-ChildItem "docs\interview\*\候选人版\*.md" | ForEach-Object {
    $content = Get-Content $_.FullName -Raw -Encoding UTF8
    if ($content -match '\*\*参考答案\*\*') { Write-Host "LEAK in $($_.Name): 参考答案" }
    if ($content -match '\*\*评估标准\*\*') { Write-Host "LEAK in $($_.Name): 评估标准" }
}
Write-Host "Check done."

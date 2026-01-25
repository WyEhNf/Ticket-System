# ==============================
# Ticket System Local Judge
# PowerShell Version (Windows)
# ==============================

$exePath  = Resolve-Path "build/code.exe" -ErrorAction SilentlyContinue
$testRoot = "testcases"

# ---------- 基本检查 ----------

if (-not $exePath) {
    Write-Error "Executable build/code.exe not found."
    exit 1
}

if (-not (Test-Path $testRoot)) {
    Write-Error "Directory 'testcases' not found."
    exit 1
}

# ---------- 枚举 basic_i 测试目录 ----------

$caseDirs = Get-ChildItem $testRoot -Directory |
    Where-Object { $_.Name -match '^basic_\d+$' } |
    Sort-Object { [int]($_.Name -replace 'basic_', '') }

if ($caseDirs.Count -eq 0) {
    Write-Error "No basic_i testcases found."
    exit 1
}

# ---------- 执行测试 ----------

foreach ($case in $caseDirs) {
    $id = $case.Name -replace 'basic_', ''

    $inFile  = Join-Path $case.FullName "$id.in"
    $outFile = Join-Path $case.FullName "$id.out"
    $tmpOut  = Join-Path $case.FullName "my.out"

    if (-not (Test-Path $inFile)) {
        Write-Error "Missing input file: $inFile"
        exit 1
    }
    if (-not (Test-Path $outFile)) {
        Write-Error "Missing output file: $outFile"
        exit 1
    }

    Write-Host "Running basic_$id ..."

    # 等价于： build/code.exe < i.in > my.out
    Get-Content $inFile -Raw | & $exePath > $tmpOut

    # 模拟 diff -ZB：忽略行尾空白 + 忽略空行
    $expected = Get-Content $outFile |
        ForEach-Object { $_.TrimEnd() } |
        Where-Object { $_ -ne "" }

    $actual = Get-Content $tmpOut |
        ForEach-Object { $_.TrimEnd() } |
        Where-Object { $_ -ne "" }

    if ($expected.Count -ne $actual.Count) {
        Write-Error "basic_$id FAILED (line count mismatch)"
        Write-Error "Expected $($expected.Count) lines, got $($actual.Count)"
        exit 1
    }

    for ($i = 0; $i -lt $expected.Count; $i++) {
        if ($expected[$i] -ne $actual[$i]) {
            Write-Error "basic_$id FAILED at line $($i + 1)"
            Write-Error "Expected: $($expected[$i])"
            Write-Error "Actual  : $($actual[$i])"
            exit 1
        }
    }

    Remove-Item $tmpOut
    Write-Host "basic_$id OK"
}

Write-Host "All basic testcases passed."

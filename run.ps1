param (
    [string]$TestcaseGroupName
)

# ================= 工具函数 =================

function Normalize-Lines {
    param ([string[]]$Lines)
    return $Lines `
        | ForEach-Object { $_.TrimEnd() } `
        | Where-Object { $_ -ne "" }
}

function Compare-Output {
    param (
        [string]$FileA,
        [string]$FileB
    )

    $a = Normalize-Lines (Get-Content $FileA)
    $b = Normalize-Lines (Get-Content $FileB)

    if ($a.Count -ne $b.Count) {
        return $false
    }

    for ($i = 0; $i -lt $a.Count; $i++) {
        if ($a[$i] -ne $b[$i]) {
            return $false
        }
    }
    return $true
}

function New-TmpFile {
    [System.IO.Path]::GetTempFileName()
}

function New-TmpDir {
    $dir = Join-Path $env:TEMP ("ticket." + [System.Guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Path $dir | Out-Null
    return $dir
}

# ================= 参数检查 =================

if (-not $TestcaseGroupName) {
    Write-Host "Usage: .\run-tests.ps1 <testcase_group_name>"
    Write-Host "Available testcase groups from config.json:"
    (Get-Content testcases/config.json | ConvertFrom-Json).Groups.GroupName |
        ForEach-Object { Write-Host "  $_" }
    exit 1
}

if (-not (Test-Path "testcases/config.json")) {
    Write-Host "./testcases/config.json does not exist, please extract testcases to that directory."
    exit 1
}

# ================= 读取 config.json =================

$config = Get-Content testcases/config.json | ConvertFrom-Json
$group = $config.Groups | Where-Object { $_.GroupName -eq $TestcaseGroupName }

if (-not $group) {
    Write-Host "Testcase group '$TestcaseGroupName' not found in config.json"
    Write-Host "Available testcase groups:"
    $config.Groups.GroupName | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$list = $group.TestPoints

# ================= 检查可执行文件 =================

$exename = "code.exe"
if (-not (Test-Path $exename)) {
    Write-Host "Please compile the ticket system and place the executable at '$exename'"
    exit 1
}

# ================= 测试准备 =================

$testdir = New-TmpDir
Copy-Item $exename $testdir
$exe = Join-Path $testdir $exename
$cwd = Get-Location
$basedir = Join-Path $cwd "testcases"

Set-Location $testdir

# ================= 主测试循环 =================

foreach ($i in $list) {
    Write-Host "About to run input #$i..."

    $outfile = New-TmpFile

    Measure-Command {
        Get-Content "$basedir\$i.in" | & $exe | Set-Content $outfile
    } | Out-Null

    $expected = "$basedir\$i.out"

    if (-not (Compare-Output $outfile $expected)) {
        Get-Content $outfile | Select-Object -First 5
        $timestamp = Get-Date -UFormat %s
        $backup = "test-$TestcaseGroupName-$i-$timestamp.log"
        Copy-Item $outfile (Join-Path $cwd $backup)
        Write-Host "Test failed on input #$i."
        Write-Host "Output saved to $backup"
        exit 1
    }

    Remove-Item $outfile
}

# ================= 清理 =================

Set-Location $cwd
Remove-Item $testdir -Recurse -Force

Write-Host "Testcase complete, answer correct."

# The A3 build against the final build at the two long views (on and baseline,
# 3 rounds each), alternating builds, each run after a quiet spell. Also logs
# the CPU load while each run goes (CPU-bound views are sensitive to builds).
param([string[]]$Builds = @('A3', 'final', 'A3', 'final'), [int]$QuietSec = 45, [int]$WaitLimitMin = 20)
$ErrorActionPreference = 'Continue'
$tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe'
$engine = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$caps = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\captures'
function Note($m) { "$(Get-Date -Format 'HH:mm:ss') $m" }
function IsQuiet {
    $held = (& $tri slots) -match 'slots held: 0/'
    $games = @(Get-Process PoseidonGame -ErrorAction SilentlyContinue).Count
    return ($held -and $games -eq 0)
}
function WaitQuiet {
    $deadline = (Get-Date).AddMinutes($WaitLimitMin)
    $since = $null
    while ((Get-Date) -lt $deadline) {
        if (IsQuiet) { if (-not $since) { $since = Get-Date }; if (((Get-Date) - $since).TotalSeconds -ge $QuietSec) { return $true } }
        else { $since = $null }
        Start-Sleep 5
    }
    return $false
}
Set-Location $engine
$k = 0
foreach ($b in $Builds) {
    $k++
    $dist = if ($b -eq 'final') { 'dist/graphics-remaster-rwdi' } else { "dist/graphics-remaster-rwdi-$b" }
    $q = WaitQuiet
    $stamp = "final-ab-$k-$b"
    $cpuLog = "$caps\$stamp-cpu.txt"
    $job = Start-Job -ScriptBlock { param($p) while ($true) { $l = (Get-CimInstance Win32_Processor | Measure-Object -Property LoadPercentage -Average).Average; $n = @(Get-Process clang*, ninja, lld*, cl -ErrorAction SilentlyContinue).Count; Add-Content -Path $p -Value "$(Get-Date -Format HH:mm:ss) cpu=$l compilers=$n"; Start-Sleep 3 } } -ArgumentList $cpuLog
    Note "$stamp start (quiet=$q)"
    & powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir $dist -DataJunction RemasterUD-gr-final -Stamp $stamp -Tests '../tmp/integration/FINAL/cost/short_abel.test.sqf' *> "$caps\$stamp-run.log"
    Stop-Job $job; Remove-Job $job
    Note "$stamp done"
}
Note 'ab finished'

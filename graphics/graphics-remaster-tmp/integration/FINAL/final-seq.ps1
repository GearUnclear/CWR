# FINAL check, second sequence: Eden and Sinai timing (one attempt each, the
# shots are pooled by pool_quiet.py on their own sampler windows), then the
# per-step cost test at Abel's long views, then a third Abel timing attempt.
# Each run starts after the machine has been quiet (no tri slot, no
# PoseidonGame) for $QuietSec, or after $WaitLimitMin regardless.
param([int]$QuietSec = 45, [int]$WaitLimitMin = 20)
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
foreach ($island in 'eden', 'sinai') {
    $q = WaitQuiet
    $stamp = "final-$island-a1"
    Note "$island start (quiet=$q)"
    & powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-final -Stamp $stamp -Tests "tests/rendering/captures/timing_$island.test.sqf" *> "$caps\$stamp-run.log"
    Note "$island done"
}
$q = WaitQuiet
Note "cost start (quiet=$q)"
& powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-final -Stamp 'final-cost' -Tests '../tmp/integration/FINAL/cost/cost_abel.test.sqf' *> "$caps\final-cost-run.log"
Note 'cost done'
$q = WaitQuiet
Note "abel a3 start (quiet=$q)"
& powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-final -Stamp 'final-abel-a3' -Tests 'tests/rendering/captures/timing_abel.test.sqf' *> "$caps\final-abel-a3-run.log"
Note 'abel a3 done'
Note 'sequence finished'

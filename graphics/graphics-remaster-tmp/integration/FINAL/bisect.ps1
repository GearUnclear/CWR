# Valley 05:15 on/baseline across the lane builds, to find which step added
# CPU per draw on the all-off path. One run per build, after a quiet spell.
param([int]$QuietSec = 30, [int]$WaitLimitMin = 15)
$ErrorActionPreference = 'Continue'
$tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe'
$engine = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$caps = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\captures'
$builds = [ordered]@{
    'A3'       = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi-A3'
    'A4lane'   = 'D:\Arma_CWA_worktrees\gr-A4\arma_CWR\dist\gr-A4-rwdi'
    'B1lane'   = 'D:\Arma_CWA_worktrees\gr-B1\arma_CWR\dist\gr-B1-rwdi'
    'B2lane'   = 'D:\Arma_CWA_worktrees\gr-B2\arma_CWR\dist\gr-B2-rwdi'
    'B4before' = 'D:\Arma_CWA_worktrees\gr-B4-rebase\arma_CWR\dist\gr-B4-before'
    'final'    = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi'
}
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
foreach ($k in $builds.Keys) {
    $q = WaitQuiet
    $stamp = "final-bisect-$k"
    Note "$k start (quiet=$q)"
    & powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir $builds[$k] -DataJunction RemasterUD-gr-final -Stamp $stamp -Tests '../tmp/integration/FINAL/cost/valley_abel.test.sqf' *> "$caps\$stamp-run.log"
    Note "$k done"
}
Note 'bisect finished'

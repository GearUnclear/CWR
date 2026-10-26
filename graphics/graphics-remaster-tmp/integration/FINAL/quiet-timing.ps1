# FINAL timing: one island at a time, each started only after the machine has
# been quiet (no tri slot held, no PoseidonGame) for 60 s, retried when the
# sampler saw a foreign game during the run. Output folders:
# <worktree>\tmp\captures\final-<island>-a<k>-timing.
param([int]$MaxAttempts = 6, [int]$QuietSec = 60, [int]$WaitLimitMin = 90)
$ErrorActionPreference = 'Continue'
$tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe'
$engine = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$caps = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\captures'
$log = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\FINAL\quiet-timing.log'
function Note($m) { $line = "$(Get-Date -Format 'HH:mm:ss') $m"; Add-Content -Path $log -Value $line; $line }
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
foreach ($island in 'abel', 'eden', 'sinai') {
    $done = $false
    for ($k = 1; $k -le $MaxAttempts -and -not $done; $k++) {
        if (-not (WaitQuiet)) { Note "$island a${k}: machine not quiet for $QuietSec s within $WaitLimitMin min, running anyway" }
        $stamp = "final-$island-a$k"
        Note "$island a${k}: start"
        & powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-final -Stamp $stamp -Tests "tests/rendering/captures/timing_$island.test.sqf" *> "$caps\$stamp-run.log"
        $csv = "$caps\$stamp-timing\gpu_samples.csv"
        $foreign = 0; $n = 0
        if (Test-Path $csv) {
            foreach ($r in (Import-Csv $csv)) { $n++; if ($r.poseidon_games -match 'foreign(\d+)' -and [int]$Matches[1] -gt 0) { $foreign++ } }
        }
        Note "$island a${k}: done, $foreign of $n sampler rows saw a foreign game"
        if ($n -gt 0 -and $foreign -eq 0) { $done = $true; Note "$island a${k}: QUIET" }
        elseif ($k -eq $MaxAttempts) { Note "${island}: no quiet run in $MaxAttempts attempts; keeping a$k" }
    }
}
Note 'all islands finished'

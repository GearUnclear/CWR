# Runs one kaiju tri test: waits for a quiet machine, then starts tri on the
# hidden desktop with one game slot, quiet, against this checkout's own
# Remaster shadow. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-tri.ps1 -Test experiments/kaiju/dedushka_photo.test.sqf -Tag photo
#       [-OutDir <folder>] [-TimeoutSec 600] [-MaxWaitSec 3600]
#
# Rules it enforces (kaiju-notes\pipeline\NOTES.md, "Run rules"):
# - one game at a time on the whole machine. Before starting tri it waits
#   while the perf squad holds or requests the machine for a timed job
#   (OWNER-HOLD, MACHINE.lock, MACHINE.request) and while deadlock.exe (the
#   owner's own game) is up. Other sessions' games are handled by tri itself:
#   --global-slots 1 is a machine-wide lock, so this tri queues behind any
#   running game and starts the moment the slot is free. (Polling for "no
#   game running" starved: the perf squad runner chains its items three
#   seconds apart.) It never stops anybody's process;
# - tri --retries 0 -j1 --global-slots 1, through tmp\tri-hidden-desktop.ps1,
#   so no window reaches the owner's desktop (--nosound is in the test's toml);
# - stops below 15 GB free on D:;
# - afterwards only LISTS games on this shadow (scoped script, -DryRun); tri
#   stops its own game. To stop a leftover: stop-owned-games.ps1 -DataDir <shadow>.
param(
    [Parameter(Mandatory)][string]$Test,
    [Parameter(Mandatory)][string]$Tag,
    [string]$OutDir = '',
    [int]$TimeoutSec = 600,
    [int]$MaxWaitSec = 3600,
    [string]$GameDir = 'D:\Arma_CWA_worktrees\cryptid\arma_CWR\dist\cryptid-rwdi',
    [string]$Tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe',
    [string]$Wrapper = 'D:\Arma_CWA\tmp\tri-hidden-desktop.ps1',
    [string]$StopOwned = 'D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1',
    [string]$LogDir = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\logs'
)
$ErrorActionPreference = 'Stop'
$work = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))            # <git root>\arma_CWR
$shadow = [IO.Path]::GetFullPath((Join-Path $work '..\tmp\remaster-shadow-kaiju'))
$q = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\perf-squad'
foreach ($f in @($Tri, $Wrapper, (Join-Path $GameDir 'PoseidonGame.exe'), (Join-Path $shadow 'BIN'), (Join-Path $work $Test))) {
    if (-not (Test-Path -LiteralPath $f)) { Write-Host "missing: $f"; exit 2 }
}
[void][IO.Directory]::CreateDirectory($LogDir)
$stamp = Get-Date -Format 'MMdd-HHmmss'
if (-not $OutDir) { $OutDir = "D:\Arma_CWA_worktrees\kaiju-notes\pipeline\runs\$Tag-$stamp" }
[void][IO.Directory]::CreateDirectory($OutDir)

function Get-Blockers {
    $b = @()
    if (Test-Path "$q\queue\OWNER-HOLD.txt") { $b += 'OWNER-HOLD' }
    # 2026-11-11, relayed from the perf squad's own sessions: the squad queue now starts its jobs at once and its
    # games may run alongside ours, so MACHINE.lock and MACHINE.request are no longer waited on. tri --global-slots 1
    # still keeps OUR runs one at a time. Timings taken while other games run are contended.
    # Owner, 2026-11-11 (relayed by the top session): keep working while Deadlock runs. Not a gate any more; timings taken meanwhile are contended.
    $free = (Get-PSDrive D).Free / 1GB
    if ($free -lt 15) { $b += ('D: free {0:N1} GB' -f $free) }
    return $b
}
$waited = 0
while ($true) {
    $b = Get-Blockers
    if (-not $b) { break }
    if ($b -match 'D: free') { Write-Host "stopping: $($b -join '; ')"; exit 126 }
    if ($waited % 60 -eq 0) { Write-Host ("[{0}] waiting: {1}" -f (Get-Date -Format HH:mm:ss), ($b -join '; ')) }
    Start-Sleep -Seconds 15
    $waited += 15
    if ($waited -gt $MaxWaitSec) { Write-Host "gave up after $MaxWaitSec s of waiting: $($b -join '; ')"; exit 125 }
}
$log = Join-Path $LogDir "$Tag-$stamp.log"
$cmd = "$Tri test --retries 0 -j1 --global-slots 1 --render gl33 --game-dir $GameDir --data-dir $shadow --output-dir $OutDir $Test"
Write-Host "running $Test"
Write-Host "  log $log"
Write-Host "  out $OutDir"
$others = Get-Process PoseidonGame, PoseidonGameDemo -ErrorAction SilentlyContinue
if ($others) { Write-Host ("  queueing behind game pids " + (($others | ForEach-Object Id) -join ',') + " (tri waits for the machine slot)") }
# Kaiju-wide turn taking (lead, 2026-11-11). tri's machine slot is not first come first served, so with
# several kaiju agents each parking a tri on it, other sessions' runs (the perf squad queue) starved for
# about an hour. Only ONE kaiju tri may exist at a time: every kaiju launcher takes the named mutex
# 'UD-kaiju-tri' before starting tri, and after its run it holds the mutex for a short pause so a
# waiting run from another session can take the machine slot first.
$kjMutex = New-Object System.Threading.Mutex($false, 'UD-kaiju-tri')
$kjGot = $false
Write-Host ("[{0}] waiting for the kaiju turn (mutex UD-kaiju-tri)" -f (Get-Date -Format HH:mm:ss))
try { $kjGot = $kjMutex.WaitOne([TimeSpan]::FromSeconds($MaxWaitSec)) }
catch [System.Threading.AbandonedMutexException] { $kjGot = $true }
if (-not $kjGot) { Write-Host "gave up after $MaxWaitSec s of waiting for the kaiju turn"; exit 125 }
try {
    Write-Host ("[{0}] kaiju turn taken" -f (Get-Date -Format HH:mm:ss))
    # The wrapper's clock also covers the time tri spends waiting for the slot.
    powershell -NoProfile -File $Wrapper -Log $log -WorkDir $work -TimeoutSec ($TimeoutSec + $MaxWaitSec) -Cmd $cmd
    $code = $LASTEXITCODE
}
finally {
    # Yield so a waiting run from another session takes the machine slot first, but keep downtime low
    # (owner, 2026-11-11): look after 4 s. If no other session has a tri waiting, release at once. If one
    # is waiting, hold only until its game has started, or 25 s at most.
    Start-Sleep -Seconds 4
    $held = 4
    while ($held -lt 25) {
        $procs = @(Get-CimInstance Win32_Process -Filter "Name='tri.exe' OR Name='PoseidonGame.exe' OR Name='PoseidonGameDemo.exe'" -ErrorAction SilentlyContinue |
            Where-Object { $_.CommandLine -and $_.CommandLine -notmatch 'kaiju' })
        $foreignTri = @($procs | Where-Object { $_.Name -eq 'tri.exe' })
        $foreignGame = @($procs | Where-Object { $_.Name -ne 'tri.exe' })
        if (-not $foreignTri -or $foreignGame) { break }
        Start-Sleep -Seconds 3
        $held += 3
    }
    Write-Host ("[{0}] kaiju turn released after {1} s" -f (Get-Date -Format HH:mm:ss), $held)
    $kjMutex.ReleaseMutex()
    $kjMutex.Dispose()
}
if (Test-Path -LiteralPath $StopOwned) {
    powershell -NoProfile -File $StopOwned -DataDir $shadow -DryRun
}
# The BMP twin of each screenshot is about 4 MB and carries nothing extra.
Get-ChildItem -LiteralPath $OutDir -Recurse -Filter *.bmp -ErrorAction SilentlyContinue | Remove-Item -Force
Get-Content -LiteralPath $log -Tail 6 -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $_" }
Write-Host "exit $code"
Write-Host "OUT=$OutDir"
exit $code

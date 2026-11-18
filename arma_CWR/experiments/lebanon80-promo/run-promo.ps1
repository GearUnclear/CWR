# Runs one Lebanon80 promo shoot test (experiment, not a regression lane). Windows PowerShell 5.1.
#
#   powershell -NoProfile -File run-promo.ps1 -Name promo_land [-TimeoutSec 3600] [-MaxWaitSec 10800]
#              [-DataDir <shadow>] [-GameDir <dist>] [-PromoDir <production folder>]
#
# - one game on the whole machine: tri --retries 0 -j1 --global-slots 1 (it queues behind any other
#   session's game), started through the hidden-desktop wrapper so no window lands on the owner's desktop;
# - waits while an owner hold file exists (the perf squad's OWNER-HOLD.txt), stops below 15 GB free on the
#   production folder's drive;
# - afterwards only lists games left on that shadow (scoped script, -DryRun); tri stops its own game.
#   A leftover: tools\stop-owned-games.ps1 -DataDir <shadow>
#
# Every location has an environment variable and a default (no machine path is written here):
#   -DataDir   $env:UD_SHOOT_SHADOW       <main checkout>\tmp\remaster-shadow-video (the shoot's own Remaster
#                                         shadow, @LoBo inside it; @kj_dedushka too for the kaiju takes)
#   -GameDir   $env:UD_SHOOT_GAMEDIR      <this worktree>\arma_CWR\dist\video-rwdi
#   -PromoDir  $env:UD_PROMO_DIR          <this worktree>\tmp\video\lebanon80-promo (same default as udpaths.py)
#   -Wrapper   $env:UD_TRI_HIDDEN_DESKTOP <main checkout>\tmp\tri-hidden-desktop.ps1
#   -HoldFile  $env:UD_OWNER_HOLD_FILE    <worktrees folder>\graphics-remaster\tmp\perf-squad\queue\OWNER-HOLD.txt
# Output: <PromoDir>\work\runs\<Name>-<stamp>\ (tri output), <PromoDir>\work\logs\<Name>-<stamp>.log
param(
    [Parameter(Mandatory)][string]$Name,
    [int]$TimeoutSec = 3600,
    [int]$MaxWaitSec = 10800,
    [string]$DataDir = $env:UD_SHOOT_SHADOW,
    [string]$GameDir = $env:UD_SHOOT_GAMEDIR,
    [string]$PromoDir = $env:UD_PROMO_DIR,
    [string]$Wrapper = $env:UD_TRI_HIDDEN_DESKTOP,
    [string]$HoldFile = $env:UD_OWNER_HOLD_FILE
)
$ErrorActionPreference = 'Stop'
$work = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))     # <worktree>\arma_CWR
$repo = Split-Path $work -Parent                                       # <worktree>
$main = $repo
try {
    $common = (& git -C $repo rev-parse --path-format=absolute --git-common-dir)
    if ($LASTEXITCODE -eq 0 -and $common) { $main = Split-Path ([IO.Path]::GetFullPath($common.Trim())) -Parent }
} catch { }
if (-not $DataDir) { $DataDir = Join-Path $main 'tmp\remaster-shadow-video' }
if (-not $GameDir) { $GameDir = Join-Path $work 'dist\video-rwdi' }
if (-not $PromoDir) { $PromoDir = Join-Path $repo 'tmp\video\lebanon80-promo' }
if (-not $Wrapper) { $Wrapper = Join-Path $main 'tmp\tri-hidden-desktop.ps1' }
if (-not $HoldFile) { $HoldFile = Join-Path (Split-Path $repo -Parent) 'graphics-remaster\tmp\perf-squad\queue\OWNER-HOLD.txt' }
$tri = Join-Path $work 'target\debug\tri.exe'
$stopOwned = Join-Path $work 'tools\stop-owned-games.ps1'
$notes = Join-Path $PromoDir 'work'
$test = "experiments/lebanon80-promo/$Name.test.sqf"
foreach ($f in @($tri, $Wrapper, (Join-Path $GameDir 'PoseidonGame.exe'), (Join-Path $DataDir '@LoBo'), (Join-Path $work $test))) {
    if (-not (Test-Path -LiteralPath $f)) { Write-Host "missing: $f"; exit 2 }
}
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $notes "runs\$Name-$stamp"
$logDir = Join-Path $notes 'logs'
[void][IO.Directory]::CreateDirectory($out)
[void][IO.Directory]::CreateDirectory($logDir)
$drive = New-Object IO.DriveInfo ([IO.Path]::GetPathRoot([IO.Path]::GetFullPath($out)))
$waited = 0
while ($true) {
    $free = $drive.AvailableFreeSpace / 1GB
    if ($free -lt 15) { Write-Host ("stopping: {0} has {1:N1} GB free (floor 15)" -f $drive.Name, $free); exit 126 }
    if (-not (Test-Path -LiteralPath $HoldFile)) { break }
    if ($waited % 300 -eq 0) { Write-Host ("[{0}] waiting: owner hold {1}" -f (Get-Date -Format HH:mm:ss), $HoldFile) }
    Start-Sleep -Seconds 15
    $waited += 15
    if ($waited -gt $MaxWaitSec) { Write-Host "gave up waiting"; exit 125 }
}
$log = Join-Path $logDir "$Name-$stamp.log"
$cmd = "$tri test --retries 0 -j1 --global-slots 1 --render gl33 --game-dir $GameDir --data-dir $DataDir --output-dir $out $test"
Write-Host ("[{0}] running {1}" -f (Get-Date -Format HH:mm:ss), $test)
Write-Host "  log $log"
Write-Host "  out $out"
powershell -NoProfile -File $Wrapper -Log $log -WorkDir $work -TimeoutSec ($TimeoutSec + $MaxWaitSec) -Cmd $cmd
$code = $LASTEXITCODE
if (Test-Path -LiteralPath $stopOwned) { powershell -NoProfile -File $stopOwned -DataDir $DataDir -DryRun }
Get-ChildItem -LiteralPath $out -Recurse -Filter *.bmp -ErrorAction SilentlyContinue | Remove-Item -Force
Get-Content -LiteralPath $log -Tail 8 -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $_" }
Write-Host ("[{0}] exit {1}" -f (Get-Date -Format HH:mm:ss), $code)
Write-Host "OUT=$out"
exit $code

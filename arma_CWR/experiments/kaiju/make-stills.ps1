# One command from a .blend to in-game stills of Dedushka.
#
#   powershell -NoProfile -File make-stills.ps1 -Blend <model.blend> [-Name <tag>]
#       [-OutRoot <folder>] [-Objects a,b] [-Exclude a,b] [-Yaw 180] [-SkipBuild] [-MaxWaitSec 3600]
#
# What it does:
#   1. build-uddedushka.ps1: export the model (a rigged "dedushka" collection as
#      it is, anything else wrapped into a statue), textures to PAA, clips,
#      pbo, config probe, mount copy in <git root>\tmp\@uddedushka_photo (the
#      photo test's own mount; the shared @uddedushka is not touched);
#   2. run-tri.ps1: the photo test (dedushka_photo.test.sqf) on the hidden
#      desktop, one game, quiet, on this checkout's Remaster shadow. It WAITS
#      while another game, the perf squad or the owner's own game is running;
#   3. analyze_shots.py: frames, contact sheet and measurements.
#
# Everything lands in <OutRoot>\<Name>-<stamp>\ (default OutRoot:
# kaiju-notes\pipeline\stills): build\ (p3d, paa, rtm, pbo, logs), run\ (tri
# output with the game log), shots\*.png, sheet.png, measurements.txt.
# Without -Blend it photographs the pipeline proxy.
#
# Exit code 0 only when the build, the game run and the collection all worked.
param(
    [string]$Blend = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\proxy\dedushka_proxy.blend',
    [string]$Name = '',
    [string]$OutRoot = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\stills',
    [string[]]$Objects = @(),
    [string[]]$Exclude = @(),
    [double]$Yaw = 180,
    [switch]$SkipBuild,
    [int]$MaxWaitSec = 3600,
    [int]$TimeoutSec = 600,
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..'))
$photoMount = Join-Path $gitRoot 'tmp\@uddedushka_photo'
if (-not (Test-Path -LiteralPath $Blend)) { throw "no such blend: $Blend" }
if (-not $Name) { $Name = [IO.Path]::GetFileNameWithoutExtension($Blend) }
$Name = $Name -replace '[^A-Za-z0-9_-]', '_'
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $OutRoot "$Name-$stamp"
[void][IO.Directory]::CreateDirectory($out)
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { throw ("D: has {0:N1} GB free, under the 15 GB floor" -f $free) }

if (-not $SkipBuild) {
    $argv = @('-NoProfile', '-File', (Join-Path $here 'addon\build-uddedushka.ps1'), '-Blend', $Blend,
        '-OutDir', (Join-Path $out 'build'), '-Yaw', "$Yaw", '-ModDir', $photoMount)
    if ($Objects.Count -gt 0) { $argv += @('-Objects', ($Objects -join ',')) }
    if ($Exclude.Count -gt 0) { $argv += @('-Exclude', ($Exclude -join ',')) }
    & powershell @argv
    if ($LASTEXITCODE -ne 0) { throw "build failed (exit $LASTEXITCODE), see $out\build" }
}

if (-not (Test-Path -LiteralPath (Join-Path $photoMount 'addons\uddedushka.pbo'))) { throw "no pbo in $photoMount (run without -SkipBuild first)" }
$run = Join-Path $out 'run'
& powershell -NoProfile -File (Join-Path $here 'run-tri.ps1') -Test 'experiments/kaiju/dedushka_photo.test.sqf' `
    -Tag "stills-$Name" -OutDir $run -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec
$triCode = $LASTEXITCODE

# Collect whatever the run produced, even when it failed part way.
& $Python (Join-Path $here 'analyze_shots.py') $run $out
$collectCode = $LASTEXITCODE
Write-Host "[stills] folder: $out"
if ($triCode -ne 0) { Write-Host "[stills] the game run failed (exit $triCode); frames up to the failure were kept"; exit $triCode }
exit $collectCode

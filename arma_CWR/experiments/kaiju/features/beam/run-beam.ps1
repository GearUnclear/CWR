# Runs one beam tri test: copies the script payload (features\beam\kj) into the
# test mission, then hands over to the pipeline's run-tri.ps1, which owns the
# run rules (owner hold, 15 GB disk floor, hidden desktop, tri --retries 0 -j1
# --global-slots 1 so only one game runs at a time). This script keeps no gate
# of its own, so it always follows whatever the pipeline's script enforces.
#
#   powershell -NoProfile -File run-beam.ps1 [-Test kaiju_beam] [-Tag beam] [-TimeoutSec 3600]
#
# -TimeoutSec covers the time tri spends queued behind other sessions' games.
# Output: kaiju-notes\features\beam\runs\<tag>-<stamp>\ (game log, screenshots),
# wrapper logs in kaiju-notes\features\beam\logs.
param(
    [string]$Test = 'kaiju_beam',
    [string]$Tag = 'beam',
    [int]$TimeoutSec = 3600,
    [int]$MaxWaitSec = 7200,
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\beam'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$dst = Join-Path $here 'KaijuBeam.Eden\kj\beam'
if (Test-Path -LiteralPath $dst) { Remove-Item -LiteralPath $dst -Recurse -Force }
New-Item -ItemType Directory -Force -Path $dst | Out-Null
Copy-Item -Path (Join-Path $here 'kj\beam\*') -Destination $dst -Recurse
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $Notes "runs\$Tag-$stamp"
& powershell -NoProfile -File (Join-Path $here '..\..\run-tri.ps1') -Test "experiments/kaiju/features/beam/$Test.test.sqf" `
    -Tag $Tag -OutDir $out -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec -LogDir (Join-Path $Notes 'logs')
$code = $LASTEXITCODE
Write-Host "BEAMOUT=$out"
exit $code

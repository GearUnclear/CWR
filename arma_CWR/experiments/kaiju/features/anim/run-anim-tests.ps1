# Runs the anim feature's tri tests through the pipeline's run-tri.ps1 (one
# game on the machine, hidden desktop, quiet) and reads each run with
# analyze_anim.py. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-anim-tests.ps1 -Test clips|walk|families [-Runs 3] [-Keep]
#
# Output: kaiju-notes\features\anim\runs\<test>-<MMdd-HHmmss>\ (tri output,
# game_stdout.log, screenshots, analysis.txt). The walk test's burst frames
# are deleted after the analysis unless -Keep is given; a contact sheet and
# the frames the analysis names are kept.
param(
    [Parameter(Mandatory)][ValidateSet('clips', 'walk', 'families')][string]$Test,
    [int]$Runs = 1,
    [switch]$Keep,
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\anim',
    [string]$Timing = '',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$runTri = Join-Path $here '..\..\run-tri.ps1'
if (-not $Timing) { $Timing = Join-Path $Notes 'build\anim\ded_anim_timing.json' }
foreach ($f in @($runTri, $Timing, (Join-Path $here "ded_anim_$Test.test.sqf"))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing: $f" }
}
$pass = 0
$results = @()
for ($i = 1; $i -le $Runs; $i++) {
    $stamp = Get-Date -Format 'MMdd-HHmmss'
    $out = Join-Path $Notes "runs\$Test-$stamp"
    # The wrapper's clock includes tri's wait for the one game slot, which other
    # agents share: a quarter of an hour of queueing has been seen.
    $timeout = 2400
    & powershell -NoProfile -File $runTri -Test "experiments/kaiju/features/anim/ded_anim_$Test.test.sqf" -Tag "anim-$Test" `
        -OutDir $out -LogDir (Join-Path $Notes 'logs') -TimeoutSec $timeout | ForEach-Object { Write-Host "  $_" }
    $tri = $LASTEXITCODE
    $argv = @((Join-Path $here 'analyze_anim.py'), $out, '--kind', $Test, '--timing', $Timing, '--tri-exit', "$tri")
    if ($Keep) { $argv += '--keep' }
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $Python @argv 2>&1 | ForEach-Object { Write-Host "  $_" }
    $an = $LASTEXITCODE
    $ErrorActionPreference = $old
    $ok = ($tri -eq 0 -and $an -eq 0)
    if ($ok) { $pass++ }
    $results += ("run {0}: tri exit {1}, analysis exit {2}: {3}  ({4})" -f $i, $tri, $an, $(if ($ok) { 'PASS' } else { 'FAIL' }), $out)
}
$results | ForEach-Object { Write-Host $_ }
Write-Host "${Test}: $pass of $Runs runs passed"
if ($pass -ne $Runs) { exit 1 }

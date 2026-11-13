# Runs the crush tri tests. Rebuilds the crush variant of the creature addon
# from the pipeline's current one, copies the script payload (kj\crush) into the
# test mission, then hands over to the pipeline's run-tri.ps1 (hidden desktop,
# one game at a time through tri's machine slot, quiet).
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-crush.ps1 -Name crush_walk [-Tag w1] [-TimeoutSec 600]
#   powershell -NoProfile -File run-crush.ps1 -All [-Tag gate1]     # crush_attacks, then crush_walk
#   powershell -NoProfile -File run-crush.ps1 -All -Times 3         # each test three times, one after the other
#   ... -GameDir <dist folder>                                      # another engine build (default: the pipeline's stock one)
#
# Output: kaiju-notes\features\crush\runs\<tag>-<stamp>\, log in ...\crush\logs.
# With -Times the tests are copied to tests_run\r<n>_<name> for the run (each copy
# is its own game start) and the folder is removed afterwards.
param(
    [string]$Name = '',
    [switch]$All,
    [int]$Times = 1,
    [string]$Tag = '',
    [string]$GameDir = '',
    [int]$TimeoutSec = 600,
    [int]$MaxWaitSec = 7200,
    [switch]$NoAddon
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\crush'
if (-not $All -and -not $Name) { throw 'give -Name <test> or -All' }
if (-not $Tag) { $Tag = if ($All) { 'all' } else { $Name } }
if (-not $NoAddon) {
    powershell -NoProfile -File (Join-Path $here 'addon\build-kjcrush.ps1') | Select-Object -Last 1
    if ($LASTEXITCODE -ne 0) { throw 'build-kjcrush.ps1 failed' }
}
$src = Join-Path $here 'kj\crush'
$dst = Join-Path $here 'KjCrush.Eden\kj\crush'
[void][IO.Directory]::CreateDirectory($dst)
Get-ChildItem -LiteralPath $dst -File | Remove-Item -Force
Copy-Item -Path (Join-Path $src '*') -Destination $dst -Force
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $notes "runs\$Tag-$stamp"
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\..\run-tri.ps1'))
$names = if ($All) { @('crush_attacks', 'crush_walk') } else { @($Name) }
$scratch = Join-Path $here 'tests_run'
if ($Times -gt 1) {
    if (Test-Path -LiteralPath $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force }
    [void][IO.Directory]::CreateDirectory($scratch)
    foreach ($i in 1..$Times) {
        foreach ($n in $names) {
            foreach ($ext in @('sqf', 'toml')) {
                Copy-Item -LiteralPath (Join-Path $here "tests\$n.test.$ext") -Destination (Join-Path $scratch "r${i}_$n.test.$ext")
            }
        }
    }
    $test = 'experiments/kaiju/features/crush/tests_run'
} elseif ($All) {
    $test = 'experiments/kaiju/features/crush/tests'
} else {
    $test = "experiments/kaiju/features/crush/tests/$Name.test.sqf"
}
$argv = @('-NoProfile', '-File', $runTri, '-Test', $test, '-Tag', $Tag, '-OutDir', $out,
    '-LogDir', (Join-Path $notes 'logs'), '-TimeoutSec', "$($TimeoutSec * $Times * $names.Count)", '-MaxWaitSec', "$MaxWaitSec")
if ($GameDir) { $argv += @('-GameDir', $GameDir) }
& powershell @argv
$code = $LASTEXITCODE
if ($Times -gt 1 -and (Test-Path -LiteralPath $scratch)) { Remove-Item -LiteralPath $scratch -Recurse -Force }
exit $code

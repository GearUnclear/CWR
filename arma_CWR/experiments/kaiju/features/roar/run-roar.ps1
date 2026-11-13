# Runs one roar tri test. Copies the script payload (kj\roar) into the test
# mission, then hands over to the pipeline's run-tri.ps1 (machine gates,
# hidden desktop, one game, --nosound). Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-roar.ps1 -Name roar_event [-Tag r1] [-TimeoutSec 600]
#   powershell -NoProfile -File run-roar.ps1 -Name all [-Tag a1]      every *.test.sqf in this folder, one after the other
#
# Output: kaiju-notes\features\roar\runs\<tag>-<stamp>\, wrapper log in
# kaiju-notes\features\roar\logs. Needs both mounts: tmp\@uddedushka (the
# pipeline's) and tmp\@kj_roar (addon\build-kj-roar.ps1).
param(
    [Parameter(Mandatory)][string]$Name,
    [string]$Tag = '',
    [int]$TimeoutSec = 600,
    [int]$MaxWaitSec = 7200
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\roar'
if (-not $Tag) { $Tag = $Name }
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
foreach ($m in @('tmp\@uddedushka\addons\uddedushka.pbo', 'tmp\@kj_roar\addons\kj_roar.pbo')) {
    if (-not (Test-Path -LiteralPath (Join-Path $gitRoot $m))) { Write-Host "missing mount: $gitRoot\$m"; exit 2 }
}
$src = Join-Path $here 'kj\roar'
$dst = Join-Path $here 'KaijuRoar.Eden\kj\roar'
[void][IO.Directory]::CreateDirectory($dst)
Get-ChildItem -LiteralPath $dst -File | Remove-Item -Force
Copy-Item -Path (Join-Path $src '*') -Destination $dst -Force
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $notes "runs\$Tag-$stamp"
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\..\run-tri.ps1'))
$test = "experiments/kaiju/features/roar/$Name.test.sqf"
if ($Name -eq 'all') { $test = 'experiments/kaiju/features/roar' }
powershell -NoProfile -File $runTri -Test $test -Tag $Tag -OutDir $out -LogDir (Join-Path $notes 'logs') -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec
exit $LASTEXITCODE

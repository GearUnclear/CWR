# Runs one finals test through the pipeline's run-tri.ps1 (hidden desktop, one game, --nosound in the
# toml, the UD-kaiju-tri mutex, tri --global-slots 1). Output: kaiju-notes\features\finals\runs\<tag>.
#   powershell -NoProfile -File run-finals.ps1 -Test finals_a -Tag a_photo1 [-TimeoutSec 600] [-MaxWaitSec 7200]
param(
    [Parameter(Mandatory)][string]$Test,
    [Parameter(Mandatory)][string]$Tag,
    [int]$TimeoutSec = 600,
    [int]$MaxWaitSec = 7200,
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\finals'
)
$ErrorActionPreference = 'Stop'
$runTri = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\run-tri.ps1'))
$out = Join-Path $Notes "runs\$Tag"
if (Test-Path -LiteralPath $out) { throw "run folder exists: $out (pick another -Tag)" }
& powershell -NoProfile -File $runTri -Test "experiments/kaiju/features/finals/$Test.test.sqf" -Tag $Tag `
    -OutDir $out -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec -LogDir (Join-Path $Notes 'logs')
exit $LASTEXITCODE

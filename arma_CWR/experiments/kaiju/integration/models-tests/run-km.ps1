# Runs one tri test of the MODEL FINISHING round against its own mount, tmp\@kj_models. A changed copy
# of integration\run-kj.ps1 (which it never touches): its own tests folder, its own copy of the script
# set (models-tests\kj) and its own copies of the missions, so a run here cannot collide with a run of
# the integration's on the same mission folder. One game at a time through the pipeline's run-tri.ps1
# (mutex UD-kaiju-tri, hidden desktop; --nosound is in the toml). Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-km.ps1 -Name kj_demo [-Tag d1] [-Params hound] [-Runs 1] [-TimeoutSec 1500]
#
# -Name     the test: models-tests\tests\<Name>.test.sqf (its toml names the mission and the mount).
# -Params   presets of models-tests\tests\params written into the mission's kjd_params.sqf for the run.
# -Stock    run on the stock binary (cryptid-rwdi) instead of the kaiju binary.
#
# Output: kaiju-notes\finishing\models\runs\<tag>-<stamp>\ (game_stdout.log, PNG frames), wrapper logs
# and verdicts.txt in kaiju-notes\finishing\models\logs. Needs the mount (models-tests\build-km.ps1).
param(
    [Parameter(Mandatory)][string]$Name,
    [string]$Tag = '',
    [switch]$Stock,
    [string[]]$Params = @(),
    [int]$Runs = 1,
    [int]$TimeoutSec = 1500,
    [int]$MaxWaitSec = 7200
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$notes = 'D:\Arma_CWA_worktrees\kaiju-notes\finishing\models'
if (-not $Tag) { $Tag = $Name }
# <git root>\arma_CWR\experiments\kaiju\integration\models-tests -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
if (-not (Test-Path -LiteralPath (Join-Path $gitRoot 'tmp\@kj_models\addons\uddedushka.pbo'))) {
    Write-Host "missing mount: $gitRoot\tmp\@kj_models (run models-tests\build-km.ps1)"; exit 2
}
$test = Join-Path $here "tests\$Name.test.sqf"
$toml = Join-Path $here "tests\$Name.test.toml"
if (-not (Test-Path -LiteralPath $test) -or -not (Test-Path -LiteralPath $toml)) { Write-Host "no test $Name in $here\tests"; exit 2 }
$m = [regex]::Match([IO.File]::ReadAllText($toml), 'mission\s*=\s*"([^"]+)"')
if (-not $m.Success) { Write-Host "the toml of $Name names no mission"; exit 2 }
$mdir = Join-Path (Join-Path $gitRoot 'arma_CWR') ($m.Groups[1].Value -replace '/', '\')
if (-not (Test-Path -LiteralPath (Join-Path $mdir 'mission.sqm'))) { Write-Host "no mission at $mdir"; exit 2 }
if ($mdir -notmatch 'models-tests') { Write-Host "the mission of $Name is not a models-tests copy: $mdir"; exit 2 }
# the script set: this folder's own copy (models-tests\kj), copied whole
$kj = Join-Path $mdir 'kj'
if (Test-Path -LiteralPath $kj) { Remove-Item -LiteralPath $kj -Recurse -Force }
Copy-Item -LiteralPath (Join-Path $here 'kj') -Destination $kj -Recurse
Write-Host "script set copied to $kj"
$paramFile = Join-Path $mdir 'kjd_params.sqf'
$paramKeep = $null
if ($Params.Count -gt 0 -and (Test-Path -LiteralPath $paramFile)) {
    $paramKeep = [IO.File]::ReadAllText($paramFile)
    $text = "// written by run-km.ps1 for one run: $($Params -join ', ')`r`n"
    foreach ($p in ($Params -split ',')) {
        $p = $p.Trim(); if (-not $p) { continue }
        $src = Join-Path $here "tests\params\$p.sqf"
        if (-not (Test-Path -LiteralPath $src)) { Write-Host "no preset $src"; exit 2 }
        $text += [IO.File]::ReadAllText($src) + "`r`n"
    }
    $text += "KJD_params = `"$($Params -join ',')`";`r`n"
    [IO.File]::WriteAllText($paramFile, $text, (New-Object Text.UTF8Encoding($false)))
    Write-Host "take: $($Params -join ', ')"
}
$gameDir = if ($Stock) { 'D:\Arma_CWA_worktrees\cryptid\arma_CWR\dist\cryptid-rwdi' } else { Join-Path $gitRoot 'arma_CWR\dist\kaiju-rwdi' }
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\..\run-tri.ps1'))
$worst = 0
for ($i = 1; $i -le $Runs; $i++) {
    $stamp = Get-Date -Format 'MMdd-HHmmss'
    $t = if ($Runs -gt 1) { "$Tag-r$i" } else { $Tag }
    $out = Join-Path $notes "runs\$t-$stamp"
    powershell -NoProfile -File $runTri -Test "experiments/kaiju/integration/models-tests/tests/$Name.test.sqf" -Tag $t -OutDir $out -LogDir (Join-Path $notes 'logs') -GameDir $gameDir -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec
    $code = $LASTEXITCODE
    $verdict = 'NO RESULT LINE (the test did not reach its end)'
    $glog = Get-ChildItem -LiteralPath $out -Recurse -Filter game_stdout.log -ErrorAction SilentlyContinue | Select-Object -First 1
    $failed = -1
    if ($glog) {
        $line = Select-String -LiteralPath $glog.FullName -Pattern 'K[JM][A-Z]+ RESULT: (\d+) checks failed: (.*)$' | Select-Object -Last 1
        if ($line) {
            $failed = [int]$line.Matches[0].Groups[1].Value
            $verdict = if ($failed -eq 0) { 'PASSED (0 checks failed)' } else { "FAILED ($failed checks: $($line.Matches[0].Groups[2].Value.TrimEnd('"')))" }
        }
    }
    if ($failed -ne 0 -and $code -eq 0) { $code = 1 }
    Write-Host "KMVERDICT $t : $verdict (tri exit $LASTEXITCODE)"
    Add-Content -LiteralPath (Join-Path $notes 'logs\verdicts.txt') -Value ("{0} {1} {2} params [{3}] binary {4} : {5} (tri exit {6}) {7}" -f (Get-Date -Format s), $Name, $t, ($Params -join ','), (Split-Path $gameDir -Leaf), $verdict, $LASTEXITCODE, $out) -Encoding utf8
    Write-Host "KMOUT=$out exit $code (binary: $gameDir)"
    if ($code -ne 0) { $worst = $code }
}
if ($null -ne $paramKeep) { [IO.File]::WriteAllText($paramFile, $paramKeep, (New-Object Text.UTF8Encoding($false))) }
exit $worst

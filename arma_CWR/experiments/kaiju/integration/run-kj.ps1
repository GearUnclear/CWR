# Runs one integration tri test. Copies the script set kj\ into the test's mission, then hands over
# to the pipeline's run-tri.ps1 (the kaiju-wide mutex UD-kaiju-tri, hidden desktop, one game,
# --nosound is in the toml). Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-kj.ps1 -Name kj_demo [-Tag d1] [-Stock] [-Runs 3] [-TimeoutSec 1500]
#
# -Name     the test: tests\<Name>.test.sqf (its toml names the mission and the mods).
# -Stock    run on the stock binary the pipeline uses (cryptid-rwdi) instead of the kaiju binary
#           (arma_CWR\dist\kaiju-rwdi). The script set must run on both.
# -NotesDir where runs\ and logs\ go (default: the integration's notes folder; the finishing round
#           passes kaiju-notesinishing\engine).
# -Runs     run it this many times, one after another (each takes the kaiju turn afresh).
#
# Output: kaiju-notes\integration\runs\<tag>-<stamp>\ (game_stdout.log, PNG frames), wrapper logs in
# kaiju-notes\integration\logs. Needs the mount tmp\@kj_dedushka (addon\build-kj-dedushka.ps1).
param(
    [Parameter(Mandatory)][string]$Name,
    [string]$Tag = '',
    [switch]$Stock,
    [string[]]$Params = @(),
    [int]$Runs = 1,
    [int]$TimeoutSec = 1500,
    [int]$MaxWaitSec = 7200,
    [string]$NotesDir = 'D:\Arma_CWA_worktrees\kaiju-notes\integration'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$notes = $NotesDir
# The owner of a queue can stop it without stopping any process: while the file logs\QUEUE-STOP exists
# in the notes folder, a run that has not started yet ends here (no game, no verdict line).
if (Test-Path -LiteralPath (Join-Path $notes 'logs\QUEUE-STOP')) { Write-Host "KJQUEUE stopped by its owner (logs\QUEUE-STOP): $Name $Tag not run"; exit 3 }
$notes = $NotesDir
if (-not $Tag) { $Tag = $Name }
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..'))
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
if (-not (Test-Path -LiteralPath (Join-Path $gitRoot 'tmp\@kj_dedushka\addons\uddedushka.pbo'))) {
    Write-Host "missing mount: $gitRoot\tmp\@kj_dedushka (run addon\build-kj-dedushka.ps1)"; exit 2
}
$test = Join-Path $here "tests\$Name.test.sqf"
$toml = Join-Path $here "tests\$Name.test.toml"
if (-not (Test-Path -LiteralPath $test) -or -not (Test-Path -LiteralPath $toml)) { Write-Host "no test $Name in $here\tests"; exit 2 }
$m = [regex]::Match([IO.File]::ReadAllText($toml), 'mission\s*=\s*"([^"]+)"')
if (-not $m.Success) { Write-Host "the toml of $Name names no mission"; exit 2 }
$mdir = Join-Path (Join-Path $gitRoot 'arma_CWR') ($m.Groups[1].Value -replace '/', '\')
if (-not (Test-Path -LiteralPath (Join-Path $mdir 'mission.sqm'))) { Write-Host "no mission at $mdir"; exit 2 }
# the script set: one source (integration\kj), copied whole
$kj = Join-Path $mdir 'kj'
if (Test-Path -LiteralPath $kj) { Remove-Item -LiteralPath $kj -Recurse -Force }
Copy-Item -LiteralPath (Join-Path $here 'kj') -Destination $kj -Recurse
Write-Host "script set copied to $kj"
# the take: the presets named by -Params (tests\params\<name>.sqf), written into the mission's
# kjd_params.sqf for this run; the mission's own file is put back afterwards
$paramFile = Join-Path $mdir 'kjd_params.sqf'
$paramKeep = $null
if ($Params.Count -gt 0 -and (Test-Path -LiteralPath $paramFile)) {
    $paramKeep = [IO.File]::ReadAllText($paramFile)
    $text = "// written by run-kj.ps1 for one run: $($Params -join ', ')`r`n"
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
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\run-tri.ps1'))
$worst = 0
for ($i = 1; $i -le $Runs; $i++) {
    $stamp = Get-Date -Format 'MMdd-HHmmss'
    $t = if ($Runs -gt 1) { "$Tag-r$i" } else { $Tag }
    $out = Join-Path $notes "runs\$t-$stamp"
    powershell -NoProfile -File $runTri -Test "experiments/kaiju/integration/tests/$Name.test.sqf" -Tag $t -OutDir $out -LogDir (Join-Path $notes 'logs') -GameDir $gameDir -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec
    $code = $LASTEXITCODE
    # The verdict. tri says whether the game ran the test to its end; the test's own checks are soft
    # and summed up in one line of the game's log ("KJ? RESULT: n checks failed: [...]").
    $verdict = 'NO RESULT LINE (the test did not reach its end)'
    $glog = Get-ChildItem -LiteralPath $out -Recurse -Filter game_stdout.log -ErrorAction SilentlyContinue | Select-Object -First 1
    $failed = -1
    if ($glog) {
        $line = Select-String -LiteralPath $glog.FullName -Pattern 'KJ[A-Z]+ RESULT: (\d+) checks failed: (.*)$' | Select-Object -Last 1
        if ($line) {
            $failed = [int]$line.Matches[0].Groups[1].Value
            $verdict = if ($failed -eq 0) { 'PASSED (0 checks failed)' } else { "FAILED ($failed checks: $($line.Matches[0].Groups[2].Value.TrimEnd('"')))" }
        }
    }
    if ($failed -ne 0 -and $code -eq 0) { $code = 1 }
    Write-Host "KJVERDICT $t : $verdict (tri exit $LASTEXITCODE)"
    Add-Content -LiteralPath (Join-Path $notes 'logs\verdicts.txt') -Value ("{0} {1} {2} params [{3}] binary {4} : {5} (tri exit {6}) {7}" -f (Get-Date -Format s), $Name, $t, ($Params -join ','), (Split-Path $gameDir -Leaf), $verdict, $LASTEXITCODE, $out) -Encoding utf8
    Write-Host "KJOUT=$out exit $code (binary: $gameDir)"
    if ($code -ne 0) { $worst = $code }
}
if ($null -ne $paramKeep) { [IO.File]::WriteAllText($paramFile, $paramKeep, (New-Object Text.UTF8Encoding($false))) }
exit $worst

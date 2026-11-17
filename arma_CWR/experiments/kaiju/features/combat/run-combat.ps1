# Runs the combat tri tests. Copies the script payload (kj\combat) into the test
# mission, then hands over to the pipeline's run-tri.ps1 (the kaiju-wide mutex
# UD-kaiju-tri, hidden desktop, one game at a time through tri's machine slot;
# --nosound is in each toml). These tests FIRE WEAPONS (owner, 2026-11-12:
# "shoot at it in tests it has to be killable too you know"); they stay silent.
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-combat.ps1 -Name combat_smallarms [-Tag s1]
#   powershell -NoProfile -File run-combat.ps1 -All [-Tag gate1]            # the five tests of tests\, one game each
#   powershell -NoProfile -File run-combat.ps1 -All -Times 3                # each test three times
#   powershell -NoProfile -File run-combat.ps1 -Name combat_death -Stock    # on the stock binary (cryptid-rwdi)
#   ... -Addon      rebuild tmp\@kj_combat first (addon\build-kjcombat.ps1; about 110 MB, half a minute)
#   ... -Class UDDedushkaProxy | UDDedushka | UDDedushkaHound   which creature the tests make (default: the proxy)
#   ... -Probe      take the test from probes\ instead of tests\ (diagnostic runs, not part of the gate)
#
# Default binary: the kaiju build (arma_CWR\dist\kaiju-rwdi), because the health
# model reads the engine's hit reports there. -Stock runs the script fallback.
# Output: kaiju-notes\features\combat\runs\<tag>-<stamp>\, wrapper log in ...\combat\logs.
# With -Times the tests are copied to tests_run\r<n>_<name> for the run (each copy
# is its own game start) and the folder is removed afterwards.
param(
    [string]$Name = '',
    [switch]$All,
    [int]$Times = 1,
    [string]$Tag = '',
    [switch]$Stock,
    [switch]$Addon,
    [switch]$Probe,
    [string]$Class = 'UDDedushkaProxy',
    [int]$TimeoutSec = 900,
    [int]$MaxWaitSec = 7200
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\combat'
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
if (-not $All -and -not $Name) { throw 'give -Name <test> or -All' }
if (-not $Tag) { $Tag = if ($All) { 'all' } else { $Name } }
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
if ($Addon) {
    powershell -NoProfile -File (Join-Path $here 'addon\build-kjcombat.ps1') | Select-Object -Last 1
    if ($LASTEXITCODE -ne 0) { throw 'build-kjcombat.ps1 failed' }
}
if (-not (Test-Path -LiteralPath (Join-Path $gitRoot 'tmp\@kj_combat\addons\uddedushka.pbo'))) { throw "missing mount $gitRoot\tmp\@kj_combat (run with -Addon)" }
# the payload, copied whole; the creature class the tests make is one line beside it
$dst = Join-Path $here 'KjCombat.Eden\kj\combat'
[void][IO.Directory]::CreateDirectory($dst)
Get-ChildItem -LiteralPath $dst -File | Remove-Item -Force
Copy-Item -Path (Join-Path $here 'kj\combat\*') -Destination $dst -Force
[IO.File]::WriteAllText((Join-Path $here 'KjCombat.Eden\kjt_class.sqf'), "kjtClass = `"$Class`";`n")
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $notes "runs\$Tag-$stamp"
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\..\run-tri.ps1'))
$srcDir = if ($Probe) { 'probes' } else { 'tests' }
$names = if ($All) { @(Get-ChildItem -LiteralPath (Join-Path $here 'tests') -Filter '*.test.sqf' | ForEach-Object { $_.Name -replace '\.test\.sqf$', '' } | Sort-Object) } else { @($Name) }
$scratch = Join-Path $here 'tests_run'
if ($Times -gt 1) {
    if (Test-Path -LiteralPath $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force }
    [void][IO.Directory]::CreateDirectory($scratch)
    foreach ($i in 1..$Times) {
        foreach ($n in $names) {
            foreach ($ext in @('sqf', 'toml')) {
                Copy-Item -LiteralPath (Join-Path $here "$srcDir\$n.test.$ext") -Destination (Join-Path $scratch "r${i}_$n.test.$ext")
            }
        }
    }
    $test = 'experiments/kaiju/features/combat/tests_run'
} elseif ($All) {
    $test = 'experiments/kaiju/features/combat/tests'
} else {
    $test = "experiments/kaiju/features/combat/$srcDir/$Name.test.sqf"
}
$gameDir = if ($Stock) { 'D:\Arma_CWA_worktrees\cryptid\arma_CWR\dist\cryptid-rwdi' } else { Join-Path $gitRoot 'arma_CWR\dist\kaiju-rwdi' }
$argv = @('-NoProfile', '-File', $runTri, '-Test', $test, '-Tag', $Tag, '-OutDir', $out,
    '-LogDir', (Join-Path $notes 'logs'), '-GameDir', $gameDir, '-TimeoutSec', "$($TimeoutSec * $Times * $names.Count)", '-MaxWaitSec', "$MaxWaitSec")
& powershell @argv
$code = $LASTEXITCODE
if ($Times -gt 1 -and (Test-Path -LiteralPath $scratch)) { Remove-Item -LiteralPath $scratch -Recurse -Force }
# The verdict. The tests take their checks softly (a tri assert is retried for the whole assert_timeout
# with the game running on, so a failed one would park the machine's game slot) and end on one line,
# "KJT VERDICT PASS|FAIL ...". A run counts as passed only when tri passed it AND that line says PASS;
# no such line (a crash, a timeout, a script error) is a failure.
$pass = 0
$fail = 0
$rows = @()
if (-not $Probe) {
    # what tri itself failed (it prints only FAIL lines): a crash on the way out after the verdict is one of them
    $triFailed = @{}
    $triLog = Get-ChildItem -LiteralPath (Join-Path $notes 'logs') -Filter "$Tag-*.log" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1
    if ($triLog) {
        foreach ($m in @(Select-String -LiteralPath $triLog.FullName -Pattern 'FAIL\S*\s+(\S+)\s+--\s+(.*)$')) {
            $triFailed[$m.Matches[0].Groups[1].Value] = $m.Matches[0].Groups[2].Value
        }
    }
    foreach ($d in @(Get-ChildItem -LiteralPath $out -Directory -ErrorAction SilentlyContinue | Sort-Object Name)) {
        $logFile = Join-Path $d.FullName 'game_stdout.log'
        $line = ''
        if (Test-Path -LiteralPath $logFile) {
            $hit = Select-String -LiteralPath $logFile -Pattern 'KJT VERDICT (PASS|FAIL).*' | Select-Object -Last 1
            if ($hit) { $line = $hit.Matches[0].Value.TrimEnd('"') }
        }
        $ok = $line -like 'KJT VERDICT PASS*'
        if (-not $line) { $line = 'NO VERDICT (the run did not reach its end)' }
        if ($triFailed.ContainsKey($d.Name)) { $ok = $false; $line = "$line; BUT tri failed it: $($triFailed[$d.Name])" }
        if ($ok) { $pass++ } else { $fail++ }
        $rows += ('  {0,-28} {1}' -f $d.Name, $line)
    }
    $rows | ForEach-Object { Write-Host $_ }
    Write-Host ("KJVERDICT passed {0} of {1} runs (tri exit {2})" -f $pass, ($pass + $fail), $code)
    $rows + ("KJVERDICT passed {0} of {1} runs (tri exit {2}; binary {3}; creature {4})" -f $pass, ($pass + $fail), $code, $gameDir, $Class) | Set-Content -LiteralPath (Join-Path $out 'verdict.txt') -Encoding utf8
    if ($fail -gt 0 -and $code -eq 0) { $code = 1 }
}
Write-Host "KJOUT=$out exit $code (binary: $gameDir, creature: $Class)"
exit $code

<#
.SYNOPSIS
    The quiet-GPU timing run of plan section 8 item 7: the GPU clock sampler plus
    run-captures.ps1 on the timing tests (gen_timing.py), then
    summarize_timing.py.

.DESCRIPTION
    Check first that the machine is quiet (`tri slots`: no other slot holders,
    no other PoseidonGame); the sampler logs foreign games for the whole run, so
    a run that was not quiet shows it. Output: <OutRoot>\<stamp>-timing\ with
    the run-captures tree, gpu_samples.csv, timing.md and timing.json.

    POSEIDON_OBSERVER_SAMPLE is set to 1 for the run by default (PERF-PLAN 2.5
    and 3.3: the observer probe samples 1 frame in 16, as in every round-1 and M2
    timing). The engine tests only that the variable is present
    (WorldFrameObserver.cpp), so -NoObserverSample unsets it; it is never set to
    0, which would still turn sampling on. The caller's value is restored after.

.EXAMPLE
    powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/x64-win-rwdi -DataJunction RemasterUD-timing
#>
[CmdletBinding()]
param(
    [string]$GameDir = 'dist/x64-win-rwdi',
    [string]$DataJunction = 'RemasterUD-gr-timing',
    [string]$OutRoot = '',
    [string]$Stamp = (Get-Date -Format 'yyyyMMdd-HHmmss'),
    [string[]]$Tests = @(
        'tests/rendering/captures/timing_abel.test.sqf',
        'tests/rendering/captures/timing_eden.test.sqf',
        'tests/rendering/captures/timing_sinai.test.sqf'
    ),
    [string]$Tri = '',
    [string]$Python = 'python',
    [int]$Seconds = 7200,
    [switch]$NoObserverSample
)
$ErrorActionPreference = 'Continue'
$obsBefore = [Environment]::GetEnvironmentVariable('POSEIDON_OBSERVER_SAMPLE', 'Process')
if ($NoObserverSample) { Remove-Item Env:\POSEIDON_OBSERVER_SAMPLE -ErrorAction SilentlyContinue }
else { $env:POSEIDON_OBSERVER_SAMPLE = '1' }
"observer sample: $(if ($NoObserverSample) { 'off (unset)' } else { 'on (POSEIDON_OBSERVER_SAMPLE=1)' })"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$engineRoot = (Resolve-Path (Join-Path $here '..\..\..')).Path
$gitRoot = (Resolve-Path (Join-Path $engineRoot '..')).Path
if (-not $Tri) {
    # this tree's tri build, else the main checkout's (the git common dir's parent)
    $Tri = Join-Path $engineRoot 'target\debug\tri.exe'
    if (-not (Test-Path -LiteralPath $Tri)) {
        $common = & git -C $engineRoot rev-parse --path-format=absolute --git-common-dir 2>$null
        if ($LASTEXITCODE -eq 0 -and $common) {
            $Tri = Join-Path (Split-Path -Parent ($common | Select-Object -First 1).Trim()) 'arma_CWR\target\debug\tri.exe'
        }
    }
}
if (-not $OutRoot) { $OutRoot = Join-Path $gitRoot 'tmp\captures' }
$out = Join-Path $OutRoot "$Stamp-timing"
New-Item -ItemType Directory -Force $out | Out-Null
$samples = Join-Path $out 'gpu_samples.csv'
"slots before: " + ((& $Tri slots) -join ' | ') | Tee-Object -FilePath (Join-Path $out 'quiet.txt')
$sampler = Start-Process powershell -ArgumentList @('-NoProfile', '-File', (Join-Path $here 'gpu_sampler.ps1'), '-Out', $samples,
    '-Seconds', "$Seconds", '-Every', '2', '-Junction', $DataJunction) -WindowStyle Hidden -PassThru
"sampler pid $($sampler.Id)"
& powershell -NoProfile -File (Join-Path $here 'run-captures.ps1') -GameDir $GameDir -DataJunction $DataJunction -OutDir $out -Tri $Tri -Tests ($Tests -join ',')
$runExit = $LASTEXITCODE
New-Item -ItemType File -Force "$samples.stop" | Out-Null
"slots after: " + ((& $Tri slots) -join ' | ') | Tee-Object -FilePath (Join-Path $out 'quiet.txt') -Append
& $Python (Join-Path $here 'summarize_timing.py') $out --out (Join-Path $out 'timing.md') --json (Join-Path $out 'timing.json')
"run-captures exit: $runExit; out: $out"
[Environment]::SetEnvironmentVariable('POSEIDON_OBSERVER_SAMPLE', $obsBefore, 'Process')
exit $runExit

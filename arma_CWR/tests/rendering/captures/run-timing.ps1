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

.EXAMPLE
    powershell -NoProfile -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1
#>
[CmdletBinding()]
param(
    [string]$GameDir = 'dist/graphics-remaster-rwdi',
    [string]$DataJunction = 'RemasterUD-gr-timing',
    [string]$OutRoot = '',
    [string]$Stamp = (Get-Date -Format 'yyyyMMdd-HHmmss'),
    [string[]]$Tests = @(
        'tests/rendering/captures/timing_abel.test.sqf',
        'tests/rendering/captures/timing_eden.test.sqf',
        'tests/rendering/captures/timing_sinai.test.sqf'
    ),
    [string]$Tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe',
    [string]$Python = 'python',
    [int]$Seconds = 7200
)
$ErrorActionPreference = 'Continue'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$engineRoot = (Resolve-Path (Join-Path $here '..\..\..')).Path
$gitRoot = (Resolve-Path (Join-Path $engineRoot '..')).Path
if (-not $OutRoot) { $OutRoot = Join-Path $gitRoot 'tmp\captures' }
$out = Join-Path $OutRoot "$Stamp-timing"
New-Item -ItemType Directory -Force $out | Out-Null
$samples = Join-Path $out 'gpu_samples.csv'
"slots before: " + ((& $Tri slots) -join ' | ') | Tee-Object -FilePath (Join-Path $out 'quiet.txt')
$sampler = Start-Process powershell -ArgumentList @('-NoProfile', '-File', (Join-Path $here 'gpu_sampler.ps1'), '-Out', $samples,
    '-Seconds', "$Seconds", '-Every', '2', '-Junction', $DataJunction) -WindowStyle Hidden -PassThru
"sampler pid $($sampler.Id)"
& powershell -NoProfile -File (Join-Path $here 'run-captures.ps1') -GameDir $GameDir -DataJunction $DataJunction -OutDir $out -Tests ($Tests -join ',')
$runExit = $LASTEXITCODE
New-Item -ItemType File -Force "$samples.stop" | Out-Null
"slots after: " + ((& $Tri slots) -join ' | ') | Tee-Object -FilePath (Join-Path $out 'quiet.txt') -Append
& $Python (Join-Path $here 'summarize_timing.py') $out --out (Join-Path $out 'timing.md') --json (Join-Path $out 'timing.json')
"run-captures exit: $runExit; out: $out"
exit $runExit

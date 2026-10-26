# A0 quiet-GPU timing run: GPU sampler + run-captures on the gen_timing.py tests.
param([string]$Stamp = (Get-Date -Format 'yyyyMMdd-HHmmss'))
$ErrorActionPreference = 'Continue'
$a0 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A0'
$out = Join-Path $a0 "$Stamp-gr-timing"
New-Item -ItemType Directory -Force $out | Out-Null
$samples = Join-Path $out 'gpu_samples.csv'
$sampler = Start-Process powershell -ArgumentList @('-NoProfile', '-File', "$a0\timing\gpu_sampler.ps1", '-Out', $samples, '-Seconds', '5400', '-Every', '2') -WindowStyle Hidden -PassThru
"sampler pid $($sampler.Id)"
"slots before: " + ((& 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe' slots) -join ' | ')
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
& powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A0 -OutDir $out -Tests '../tmp/integration/A0/timing/abel_timing.test.sqf,../tmp/integration/A0/timing/eden_timing.test.sqf,../tmp/integration/A0/timing/sinai_timing.test.sqf'
"run-captures exit: $LASTEXITCODE"
New-Item -ItemType File -Force "$samples.stop" | Out-Null
"slots after: " + ((& 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe' slots) -join ' | ')

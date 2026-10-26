<#
  fable-1 (fogwind), from agent 1's run_rt.ps1: run look_probe_fable1_rt_<Tag> on the mist-1 recorder build
  (= d91a08e0 + the in-memory recorder) with a shader override dir, then mkv the raw dumps and measure.
  powershell -NoProfile -File run_rt_fable1.ps1 -Tag q2z -ShaderOverrideDir <dir>
#>
param([Parameter(Mandatory = $true)][string]$Tag, [string]$ShaderOverrideDir = '', [string]$Junction = 'RemasterUD-fog-F1')
$ErrorActionPreference = 'Continue'
$base = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind'
$out = "$base\fable-1\runs\rt-$Tag"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$runner = 'D:\Arma_CWA_worktrees\video-work\arma_CWR\tests\rendering\captures\run-captures.ps1'
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', "tests/rendering/captures/video/look_probe_fable1_rt_$Tag.test.sqf",
    '-GameDir', 'D:\Arma_CWA_worktrees\mist-1\arma_CWR\dist\mist-1-rwdi', '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderOverrideDir) { $runArgs += @('-ShaderOverrideDir', $ShaderOverrideDir) }
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
"start $(Get-Date -Format HH:mm:ss) override=$ShaderOverrideDir" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
& powershell @runArgs *> (Join-Path $out 'run.log')
"run exit=$LASTEXITCODE $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
$rec = "$base\fable-1\rec"
foreach ($s in @('rt60', 'rtst')) {
    $raw = Join-Path $rec "${Tag}_$s.raw"
    if (Test-Path -LiteralPath $raw) {
        & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\1\rec2mkv.py' $raw 2>&1 | Out-File -FilePath (Join-Path $out "rec2mkv_$s.txt") -Encoding utf8
    }
}
Get-ChildItem -LiteralPath $out -Recurse -Include '*.log' -ErrorAction SilentlyContinue | Select-String -Pattern 'shader override|shader compile error' | ForEach-Object { $_.Line } | Out-File -FilePath (Join-Path $out 'override.txt') -Encoding utf8
$kit = "$base\1\kit\fogmotion.py"
& python $kit (Join-Path $rec "${Tag}_rtst.mkv") --from 10 --static --noband --regions bay,valley,farhill,haze 2>&1 | Out-File -FilePath (Join-Path $out 'rt_rtst.txt') -Encoding utf8
& python $kit (Join-Path $rec "${Tag}_rt60.mkv") --from 0 --to 150 --noband --regions bay,valley,farhill,haze 2>&1 | Out-File -FilePath (Join-Path $out 'rt_rt60.txt') -Encoding utf8
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8

# Runs one texture-resolution tri test through the pipeline's run-tri.ps1 (which
# waits for the machine, uses the hidden desktop, one game, --global-slots 1 and
# never stops anybody's process), with this feature's own output folders, and
# samples the game's GPU memory while it runs.
#
#   powershell -NoProfile -File run-texres.ps1 -Test texres_charts [-Tag charts] [-TimeoutSec 700]
#       [-ShaderOverrideDir detail_demo]
#
# Output: kaiju-notes\features\texres\runs\<tag>-<stamp>\ (game log, screenshots,
# gpu_mem.csv), wrapper logs in kaiju-notes\features\texres\logs.
#
# The sampler only READS Windows performance counters ("GPU Process Memory",
# dedicated and shared bytes) for PoseidonGame processes whose command line
# names @kj_texres, and counts (never touches) the other games and deadlock.exe
# on the machine, so that a timing taken beside them can be marked as contended.
# It starts and stops nothing but itself.
#
# One kaiju tri at a time: the pipeline's run-tri.ps1, which this script calls, takes the
# named mutex UD-kaiju-tri before it starts tri and gives it up after the run by its own
# turn-taking rule (a short look for a waiting non-kaiju tri, 25 s at most). This
# script does not take it itself (the child would then wait on its own parent). Start one
# run-texres at a time and wait for it; never park two.
param(
    [Parameter(Mandatory)][string]$Test,
    [string]$Tag = '',
    [int]$TimeoutSec = 700,
    [int]$MaxWaitSec = 7200,
    [string]$ShaderOverrideDir = '',
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\texres'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
if (-not $Tag) { $Tag = $Test -replace '^texres_', '' }
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\..\run-tri.ps1'))
$rel = "experiments/kaiju/features/texres/$Test.test.sqf"
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $Notes "runs\$Tag-$stamp"
$logs = Join-Path $Notes 'logs'
[void][IO.Directory]::CreateDirectory($out)
[void][IO.Directory]::CreateDirectory($logs)

$csv = Join-Path $out 'gpu_mem.csv'
$stop = Join-Path $out 'sampler.stop'
$sampler = {
    param($csv, $stop, $limitSec)
    'time,pid,dedicated_bytes,shared_bytes,games_on_machine,deadlock_up' | Out-File -FilePath $csv -Encoding ascii
    $sw = [Diagnostics.Stopwatch]::StartNew()
    while (-not (Test-Path -LiteralPath $stop) -and $sw.Elapsed.TotalSeconds -lt $limitSec) {
        $all = @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue)
        $games = $all | Where-Object { $_.CommandLine -match 'kj_texres' }
        # Only counted, never touched: timings taken beside other games or Deadlock are contended.
        $dl = [int][bool](Get-Process deadlock -ErrorAction SilentlyContinue)
        foreach ($g in $games) {
            try {
                $c = Get-Counter -Counter "\GPU Process Memory(pid_$($g.ProcessId)_*)\Dedicated Usage", "\GPU Process Memory(pid_$($g.ProcessId)_*)\Shared Usage" -ErrorAction Stop
                $ded = ($c.CounterSamples | Where-Object { $_.Path -match 'dedicated' } | Measure-Object CookedValue -Sum).Sum
                $shr = ($c.CounterSamples | Where-Object { $_.Path -match 'shared' } | Measure-Object CookedValue -Sum).Sum
                ('{0},{1},{2},{3},{4},{5}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss.fff'), $g.ProcessId, [int64]$ded, [int64]$shr, $all.Count, $dl) | Out-File -FilePath $csv -Encoding ascii -Append
            } catch { }
        }
        Start-Sleep -Milliseconds 400
    }
}
# Detail layer demo only: the game reads CWR_SHADER_OVERRIDE_DIR at start and prefers
# <dir>/<name>.glsl over its built-in shader source. The child processes inherit it.
if ($ShaderOverrideDir) {
    if (-not [IO.Path]::IsPathRooted($ShaderOverrideDir)) { $ShaderOverrideDir = Join-Path $here $ShaderOverrideDir }
    $env:CWR_SHADER_OVERRIDE_DIR = [IO.Path]::GetFullPath($ShaderOverrideDir)
    Write-Host "shader override dir: $env:CWR_SHADER_OVERRIDE_DIR"
}
else { Remove-Item Env:CWR_SHADER_OVERRIDE_DIR -ErrorAction SilentlyContinue }
$job = Start-Job -ScriptBlock $sampler -ArgumentList $csv, $stop, ($TimeoutSec + $MaxWaitSec + 60)
try {
    powershell -NoProfile -File $runTri -Test $rel -Tag $Tag -OutDir $out -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec -LogDir $logs
    $code = $LASTEXITCODE
} finally {
    'stop' | Out-File -FilePath $stop -Encoding ascii
    Wait-Job $job -Timeout 20 | Out-Null
    Remove-Job $job -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $stop -Force -ErrorAction SilentlyContinue
}
Write-Host "TEXRESOUT=$out"
exit $code

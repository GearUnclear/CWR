<#
    fogwind kit (agent 1): run one look_probe_<Probe> test on the overhaul build (hidden desktop, via
    run-captures.ps1), keep the frames as a near-lossless mkv, delete the PNGs, measure with fogmotion.py.

    powershell -NoProfile -File run_probe_fog.ps1 -Probe fog1_static -Tag static-percell `
        -OutRoot D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\<N>\runs `
        -Junction RemasterUD-fog-<N> [-ShaderOverrideDir <abs dir with psFogInject.glsl ...>]

    Probes (video-work/arma_CWR/tests/rendering/captures/video/):
      fog1_static   the valley_mist staging, clock and props at k = 270..404 (135 shots, exact 1/60 s step
                    per shot), the camera HELD at the frame 328 pose (path index 327). Any change in the
                    fog is an artefact (the soldiers still animate: keep them out of the regions).
      fog1_moving   the same shots with the camera on the real path (frames 270..404 of the clip).
    Output: <OutRoot>\<Tag>\ (run.log, status.txt, <Tag>.mkv, motion.txt/json, keep\ f0060 PNG,
    shader-override lines from the game log in override.txt).
    Use your own -Junction; one game at a time per agent. Cleanup only with
    D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -DataDir <engine root>\packages\<junction>.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Probe,
    [Parameter(Mandatory = $true)][string]$Tag,
    [Parameter(Mandatory = $true)][string]$OutRoot,
    [Parameter(Mandatory = $true)][string]$Junction,
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi',
    [string]$ShaderOverrideDir = '',
    [int]$From = 15,
    [string]$Prefix = '',
    [switch]$CollectOnly
)
$ErrorActionPreference = 'Continue'
if (-not $Prefix) { $Prefix = "look_probe_${Probe}__ud__f" }
$kit = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\1\kit'
$out = Join-Path $OutRoot $Tag
if (-not $CollectOnly -and (Test-Path -LiteralPath $out)) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/look_probe_$Probe.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
"start $(Get-Date -Format HH:mm:ss) test=$testPath override=$ShaderOverrideDir junction=$Junction game=$GameDir" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderOverrideDir) { $runArgs += @('-ShaderOverrideDir', $ShaderOverrideDir) }
$code = 0
if (-not $CollectOnly) { & powershell @runArgs *> (Join-Path $out 'run.log'); $code = $LASTEXITCODE }
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*${Prefix}*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1))" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
Get-ChildItem -LiteralPath $out -Recurse -Include '*.log', '*.rpt' -ErrorAction SilentlyContinue |
    Select-String -Pattern 'shader override|override' -ErrorAction SilentlyContinue |
    ForEach-Object { $_.Line } | Out-File -FilePath (Join-Path $out 'override.txt') -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ($p.Name -replace '^\d+_', '')) }
    $startNo = [int](($pngs[0].BaseName -replace '^.*__f', ''))
    $mkv = Join-Path $out "$Tag.mkv"
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames "${Prefix}%04d.png") -c:v libx264rgb -crf 4 -preset fast $mkv
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    $f = Join-Path $frames "${Prefix}0060.png"
    if (Test-Path -LiteralPath $f) { Copy-Item -LiteralPath $f -Destination $keep }
    if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath $mkv)) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
        & python (Join-Path $kit 'fogmotion.py') $mkv --from $From --json (Join-Path $out 'motion.json') 2>&1 | Out-File -FilePath (Join-Path $out 'motion.txt') -Encoding utf8
    }
}
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8

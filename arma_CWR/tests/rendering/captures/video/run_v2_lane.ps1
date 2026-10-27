<#
.SYNOPSIS
    Shoots v2 comparison-video clips one game at a time and collects their
    frames into tmp\video\v2\<side>\<clip>\ (gen_video_v2.py writes the tests).

.DESCRIPTION
    For every clip, in order: run-captures.ps1 (hidden desktop, scoped
    cleanup by the junction) on the clip's test, then collect_frames.py
    (vanilla: gamma 1.2 applied; ud: copied as shot). One game at a time.
    Progress lines go to tmp\video\v2\runs\lane-<side>.txt.

    Vanilla object detail: the vanilla engine picks its object detail from the
    last four frame times (Scene::AdjustComplexity) and will not refine for
    5 s after it coarsened. Every video frame's screenshot write stalls one
    frame by about 0.3 s, so with the default "frame rate" preference (15) the
    forests fall to their coarse block LOD a frame or two into the clip. The
    vanilla side therefore runs each clip with its own copy of -UserDirSeed as
    POSEIDON_USER_DIR (tri honours a caller's user dir), whose profile sets the
    player option frameRate=3 in UserInfo.cfg: the detail then stays at its
    finest, as the fork's triPinObjectDetail 1 holds it on the ud side. A
    player setting, not an engine change.

    vanilla side: the vanilla upstream build and its vanilla-safe tests
      -Side vanilla  (defaults below)
    ud side (the final overhaul build, or any fork build from A1 on):
      -Side ud -GameDir <dist folder of that build> -DataJunction RemasterUD-video-<name>

.EXAMPLE
    powershell -NoProfile -File run_v2_lane.ps1 -Side vanilla
.EXAMPLE
    powershell -NoProfile -File run_v2_lane.ps1 -Side vanilla -Clips opening -Expect 1056
    (a clip written longer with gen_video_v2.py --clip opening --frames 1056; collect_frames.py gets --expect 1056)
.EXAMPLE
    powershell -NoProfile -File run_v2_lane.ps1 -Side ud -GameDir <arma_CWR>\dist\graphics-remaster-rwdi -DataJunction RemasterUD-video-final
#>
[CmdletBinding()]
param(
    [ValidateSet('vanilla', 'ud', 'pv')][string]$Side = 'vanilla',
    [string[]]$Clips = @('opening', 'valley_mist', 'pine_shafts', 'bay_reflections', 'outpost', 'cloud_shadows',
        'sinai_camp', 'ambush', 'convoy', 'checkpoint', 'town_patrol', 'rain_town', 'supply_run', 'firefight'),
    [string]$GameDir = '',
    [string]$DataJunction = '',
    [string]$V2 = '',
    [string]$VanillaRoot = '',
    [string]$Python = 'python',
    [string]$UserDirSeed = '',
    [string]$LaneTag = '',
    [int]$Expect = 0,
    [switch]$KeepRaw
)
$ErrorActionPreference = 'Continue'
$Clips = @($Clips | ForEach-Object { $_ -split '[,\s]+' } | Where-Object { $_ })
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $here '..\..\..\..\..')).Path   # the checkout root
# Defaults follow udpaths.py: UD_VIDEO_WORK and UD_VANILLA_ROOT, else the repo layout.
if (-not $V2) { $V2 = if ($env:UD_VIDEO_WORK) { $env:UD_VIDEO_WORK } else { Join-Path $repoRoot 'tmp\video\v2' } }
if (-not $VanillaRoot) {
    $VanillaRoot = if ($env:UD_VANILLA_ROOT) { $env:UD_VANILLA_ROOT } else { Join-Path (Split-Path -Parent $repoRoot) 'vanilla-upstream' }
}
if ($Side -eq 'vanilla') {
    $engineRoot = $VanillaRoot
    $runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
    if (-not $GameDir) { $GameDir = Join-Path $VanillaRoot 'dist\vanilla-upstream-rwdi' }
    if (-not $DataJunction) { $DataJunction = 'RemasterUD-video-vanilla' }
    if (-not $UserDirSeed) { $UserDirSeed = Join-Path $VanillaRoot 'tmp\video-userseed' }
    $prefix = 'van'
} else {
    $engineRoot = (Resolve-Path (Join-Path $here '..\..\..\..')).Path   # arma_CWR of this worktree
    $runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
    if (-not $GameDir) { Write-Error '-GameDir is required for the ud and pv sides (the build to shoot)'; exit 2 }
    if (-not $DataJunction) { $DataJunction = 'RemasterUD-video-ud' }
    $prefix = $Side   # ud: the clips; pv: 3-frame framing previews (gen_video_v2.py --side pv --frames 3), not collected
}
$runs = Join-Path $V2 'runs'
New-Item -ItemType Directory -Force -Path $runs | Out-Null
$lane = if ($LaneTag) { Join-Path $runs "lane-$Side-$LaneTag.txt" } else { Join-Path $runs "lane-$Side.txt" }
function Note($msg) { "$(Get-Date -Format 'HH:mm:ss') $msg" | Out-File -FilePath $lane -Append -Encoding utf8 }
Note "lane start side=$Side game=$GameDir junction=$DataJunction userseed=$UserDirSeed clips=$($Clips -join ',')"
foreach ($clip in $Clips) {
    $test = "tests/rendering/captures/video/${prefix}_$clip.test.sqf"
    $out = Join-Path $runs "$prefix-$clip"
    if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
    Note "start $clip"
    if ($UserDirSeed) {
        $userDir = Join-Path $runs "$prefix-$clip-user"
        if (Test-Path -LiteralPath $userDir) { Remove-Item -LiteralPath $userDir -Recurse -Force }
        Copy-Item -LiteralPath $UserDirSeed -Destination $userDir -Recurse
        $env:POSEIDON_USER_DIR = $userDir
    } else {
        Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
    }
    & powershell -NoProfile -ExecutionPolicy Bypass -File $runner -Tests $test -GameDir $GameDir -DataJunction $DataJunction -OutDir $out *> (Join-Path $runs "$prefix-$clip.log")
    $code = $LASTEXITCODE
    Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
    $n = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*__${Side}__f*.png" -ErrorAction SilentlyContinue).Count
    Note "shot $clip exit=$code frames=$n"
    if ($Side -eq 'pv') { continue }
    $collectArgs = @((Join-Path $here 'collect_frames.py'), $out, '--side', $Side, '--clip', $clip)
    if ($KeepRaw) { $collectArgs += '--keep-raw' }
    # -Expect N: a clip reshot longer than its table length (gen_video_v2.py --frames N, owner_vo.py fit)
    if ($Expect -gt 0) { $collectArgs += @('--expect', "$Expect") }
    $res = & $Python @collectArgs 2>&1 | Out-String
    Note "collect $clip exit=$LASTEXITCODE $($res.Trim())"
}
Note 'lane finished'

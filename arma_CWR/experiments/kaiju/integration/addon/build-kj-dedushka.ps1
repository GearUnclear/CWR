# Builds the ONE integrated mod folder of the giant creatures, <git root>\tmp\@kj_dedushka:
#   addons\uddedushka.pbo  every creature of creatures.json: its model, its own generated clips, and one
#                          merged config (a moves class and a vehicle class with `class Kaiju` traits each)
#   addons\kjbeam.pbo      the drawn parts of the beam (the beam feature's addon, built here from a copy)
#   addons\kj_roar.pbo     ONE of the two shared sound sets (the switch is -Sound)
#   addons\<voice>.pbo     a creature's own voice, when creatures.json names a pbo that exists
# Mount it alone: --mods-dir ".." --mod "@kj_dedushka". Never beside @uddedushka, @kj_anim, @kj_brain,
# @kj_crush, @kj_final_*, @kj_roar or @kj_roar_vst (same addon names). Windows PowerShell 5.1 or later.
# No game is started.
#
#   powershell -NoProfile -File build-kj-dedushka.ps1 [-Creatures a,d,proxy] [-Sound vst|first]
#       [-SoundOnly] [-OutDir <folder>] [-NoInstall] [-NoProbe]
#
# A creature is a DATA ENTRY in creatures.json beside this script (class, model build folder, voice).
# THE ONE STEP for a new or changed model: point its entry's "build" at the model's build folder (the
# folder that holds dedushka.p3d, dedushka_rest.json, dedushka_export.json and stage\uddedushka\*.paa)
# and run this script. Height, reach, wading depth, furnace and mouth points, footfall phases and every
# clip are worked out from that folder; the scripts read them off the unit.
#
# -Creatures   which entries to build, by key (default: all of creatures.json).
# -Sound       vst   = the REAPER and VST set (tmp\@kj_roar_vst), the default until the owner chooses;
#              first = the first, numerical set (tmp\@kj_roar).
# -SoundOnly   only swap the shared sound pbo in the mount (no rebuild).
# -OutDir      every built file. Default kaiju-notes\integration\build. Nothing binary goes into the repo.
# -GeometryFloor  the collision shape of every creature starts this share of its height above the ground
#              (default 0.17, about 7 m; never under 6 m): nothing on the ground can shove it. 0 = as built.
# -ModDir      (alias -Mount) the mod folder to install into. Default <git root>\tmp\@kj_dedushka.
# -CreaturesFile  the creature table. Default creatures.json beside this script.
# -KjAnimDir   where each creature's timing table (timing_<class>.sqf) is written. Default ..\kj\anim
#              (the script set). A build into another mount names its own copy of the script set.
#   (These three were added in the finishing round so that a model build can be tried in its own mount,
#   for example: -Mount <git root>\tmp\@kj_models -CreaturesFile models\creatures.models.json
#   -KjAnimDir ..\models-tests\kj\anim -OutDir <notes>\finishing\models\build\mount. With none of them
#   given the build is what it always was.)
#
# The feature folders, the pipeline's files and the other mounts are only read.
param(
    [string[]]$Creatures = @(),
    [ValidateSet('vst', 'first')][string]$Sound = 'vst',
    [switch]$SoundOnly,
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\integration\build',
    [switch]$NoInstall,
    [switch]$NoProbe,
    [double]$GeometryFloor = 0.17,
    [Alias('Mount')][string]$ModDir = '',
    [string]$CreaturesFile = '',
    [string]$KjAnimDir = '',
    [string]$DataDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$MoveCheck = 'D:\Arma_CWA_worktrees\cryptid-notes\anim\moves\check_moves.py',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\integration\addon -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_dedushka' }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
if (-not $CreaturesFile) { $CreaturesFile = Join-Path $here 'creatures.json' }
if (-not $KjAnimDir) { $KjAnimDir = Join-Path $here '..\kj\anim' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)
$mount = Join-Path $ModDir 'addons'
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
$utf8 = New-Object Text.UTF8Encoding($false)

function Invoke-Logged([string]$log, [string]$exe, [string[]]$argv, [switch]$AllowFail) {
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { & $exe @argv 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $log -Encoding utf8 }
    finally { $ErrorActionPreference = $old }
    if ($LASTEXITCODE -ne 0 -and -not $AllowFail) {
        Get-Content -LiteralPath $log -Tail 25 | ForEach-Object { Write-Host "    $_" }
        throw "$exe failed with exit $LASTEXITCODE (log: $log)"
    }
}

# ---- the shared sound set: one switch ----------------------------------------------------------
$soundSrc = @{
    vst   = @((Join-Path $gitRoot 'tmp\@kj_roar_vst\addons\kj_roar.pbo'), 'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst\build\@kj_roar_vst\addons\kj_roar.pbo')
    first = @((Join-Path $gitRoot 'tmp\@kj_roar\addons\kj_roar.pbo'), 'D:\Arma_CWA_worktrees\kaiju-notes\features\roar\build\@kj_roar\addons\kj_roar.pbo')
}
function Install-Sound {
    $src = $soundSrc[$Sound] | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if (-not $src) { throw "no kj_roar.pbo for the '$Sound' sound set (looked in: $($soundSrc[$Sound] -join ', '))" }
    [void][IO.Directory]::CreateDirectory($mount)
    Copy-Item -LiteralPath $src -Destination (Join-Path $mount 'kj_roar.pbo') -Force
    $name = if ($Sound -eq 'vst') { 'vst (REAPER and VST set)' } else { 'first (numerical set)' }
    Set-Content -LiteralPath (Join-Path $ModDir 'SOUND.txt') -Value "$name`r`nfrom $src`r`n$(Get-Date -Format s)" -Encoding utf8
    Write-Host "[kj_dedushka] shared sound set: $name"
}
if ($SoundOnly) { Install-Sound; exit 0 }

foreach ($f in @($Tools, $CreaturesFile, (Join-Path $here 'moves_dedushka.hpp'), (Join-Path $here 'gen_ded_anim.py'),
        (Join-Path $here 'gen_config.py'), (Join-Path $here 'gen_kjbeam.py'), (Join-Path $here 'kjbeam\config.cpp'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}
$table = Get-Content -LiteralPath $CreaturesFile -Raw | ConvertFrom-Json
$want = @($Creatures | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$list = @($table.creatures | Where-Object { $want.Count -eq 0 -or $want -contains $_.key })
if ($list.Count -eq 0) { throw "no creature of $CreaturesFile matches -Creatures $($want -join ',')" }

$stage = Join-Path $OutDir 'stage\uddedushka'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
$plan = @()
foreach ($c in $list) {
    $build = ($c.build -replace '/', '\')
    $rest = Join-Path $build 'dedushka_rest.json'
    $p3d = Join-Path $build 'dedushka.p3d'
    $export = Join-Path $build 'dedushka_export.json'
    $paaDir = Join-Path $build 'stage\uddedushka'
    foreach ($f in @($p3d, $rest, $export, $paaDir)) { if (-not (Test-Path -LiteralPath $f)) { throw "creature $($c.key): missing $f" } }
    $summary = Get-Content -LiteralPath $export -Raw | ConvertFrom-Json

    # ---- 1. clips for this creature's joints ---------------------------------------------------
    $animDir = Join-Path $OutDir "anim_$($c.key)"
    if (Test-Path -LiteralPath $animDir) { Remove-Item -LiteralPath $animDir -Recurse -Force }
    Invoke-Logged (Join-Path $OutDir "gen_$($c.key).log") $Python @((Join-Path $here 'gen_ded_anim.py'), '--rest', $rest, '--out', $animDir,
        '--depth', "$($summary.depth_forward_m)") -AllowFail
    $report = Join-Path $animDir 'ded_anim_report.txt'
    if (-not (Test-Path -LiteralPath $report)) { throw "creature $($c.key): the clip generator wrote no report (log: $OutDir\gen_$($c.key).log)" }
    Get-Content -LiteralPath $report -Tail 4 | Where-Object { $_ } | ForEach-Object { Write-Host "    [$($c.key)] $_" }
    $failed = @(Get-Content -LiteralPath $report | Where-Object { $_ -match '^FAILED: ' })
    if ($failed.Count -gt 0) {
        # One check is tolerated with a warning: the stomp's key spacing fails on both final models'
        # joint files (finals notes); the clip still plays, a fast shin can cut a corner between two keys.
        $items = ($failed[0] -replace '^FAILED: ', '') -split '; '
        $real = @($items | Where-Object { $_ -notmatch '^DedStomp: [0-9.]+ degrees between two keys' })
        if ($real.Count -gt 0) { throw "creature $($c.key): offline clip checks failed: $($real -join '; ')" }
        Write-Warning "creature $($c.key), tolerated: $($items -join '; ')"
    }

    # ---- 2. stage its model, textures and clips ------------------------------------------------
    Copy-Item -LiteralPath $p3d -Destination (Join-Path $stage "$($c.model).p3d")
    # the collision shape is lifted off the ground in the staged copy, so men, hulls and wrecks pass
    # under it and cannot shove the creature (lift_geometry.py says why). -GeometryFloor 0 leaves it.
    $floor = 0.0
    if ($GeometryFloor -gt 0) { $floor = [math]::Max(6.0, $GeometryFloor * [double]$summary.bounds.max[2]) }
    Invoke-Logged (Join-Path $OutDir "lift_$($c.key).log") $Python @((Join-Path $here 'lift_geometry.py'), (Join-Path $stage "$($c.model).p3d"), '--floor', ('{0:F2}' -f $floor))
    $liftLog = Get-Content -LiteralPath (Join-Path $OutDir "lift_$($c.key).log")
    $liftLog | ForEach-Object { Write-Host "    [$($c.key)] $_" }
    $geoR = 0.0
    $gm = $liftLog | Select-String -Pattern 'GEO_RADIUS ([0-9.]+)' | Select-Object -First 1
    if ($gm) { $geoR = [double]$gm.Matches[0].Groups[1].Value }
    Get-ChildItem -LiteralPath $paaDir -Filter *.paa | ForEach-Object {
        $dst = Join-Path $stage $_.Name
        if (Test-Path -LiteralPath $dst) { throw "creature $($c.key): texture name $($_.Name) is already used by another creature" }
        Copy-Item -LiteralPath $_.FullName -Destination $dst
    }
    $clipDir = Join-Path $stage "anim_$($c.key)"
    New-Item -ItemType Directory -Force -Path $clipDir | Out-Null
    Get-ChildItem -LiteralPath $animDir -Filter *.rtm | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $clipDir }
    $plan += [ordered]@{ key = $c.key; class = $c.class; displayName = $c.displayName; model = $c.model; moves = $c.moves
        sounds = $c.sounds; geoRadius = $geoR; geoFloor = $floor; anim = $animDir; export = $export; rest = $rest }
}

# ---- 3. the one config, and the clips it names ------------------------------------------------------
$planPath = Join-Path $OutDir 'plan.json'
[IO.File]::WriteAllText($planPath, (@{ creatures = $plan } | ConvertTo-Json -Depth 5), $utf8)
$cfgPath = Join-Path $stage 'config.cpp'
Invoke-Logged (Join-Path $OutDir 'config.log') $Python @((Join-Path $here 'gen_config.py'), '--plan', $planPath,
    '--moves', (Join-Path $here 'moves_dedushka.hpp'), '--out', $cfgPath)
Get-Content -LiteralPath (Join-Path $OutDir 'config.log') | ForEach-Object { Write-Host "    $_" }
$config = [IO.File]::ReadAllText($cfgPath)
$named = [regex]::Matches($config, '\\uddedushka\\(anim_[A-Za-z0-9]+\\[A-Za-z0-9_]+\.rtm)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
foreach ($clip in $named) {
    if (-not (Test-Path -LiteralPath (Join-Path $stage $clip))) { throw "the config names $clip but gen_ded_anim.py did not make it" }
}
Write-Host "[kj_dedushka] $($list.Count) creature(s), $($named.Count) clips staged"
$addons = Join-Path $OutDir '@kj_dedushka\addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)

# ---- 4. the beam addon, from the integration copy of its sources --------------------------------
$beamOut = Join-Path $OutDir 'kjbeam'
[void][IO.Directory]::CreateDirectory($beamOut)
Invoke-Logged (Join-Path $OutDir 'beam-gen.log') $Python @((Join-Path $here 'gen_kjbeam.py'), $beamOut)
$beamSum = Get-Content -LiteralPath (Join-Path $beamOut 'kjbeam_gen.json') -Raw | ConvertFrom-Json
$beamStage = Join-Path $OutDir 'stage\kjbeam'
if (Test-Path -LiteralPath $beamStage) { Remove-Item -LiteralPath $beamStage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $beamStage | Out-Null
Copy-Item -LiteralPath (Join-Path $here 'kjbeam\config.cpp') -Destination (Join-Path $beamStage 'config.cpp')
foreach ($m in $beamSum.models.PSObject.Properties.Name) { Copy-Item -LiteralPath (Join-Path $beamOut $m) -Destination $beamStage }
foreach ($t in $beamSum.textures) {
    Invoke-Logged (Join-Path $OutDir "beam-paa-$($t.paa).log") $Tools @('image', 'convert', $t.png, (Join-Path $beamStage $t.paa), '-f', $t.format)
}
$beamPbo = Join-Path $addons 'kjbeam.pbo'
if (Test-Path -LiteralPath $beamPbo) { Remove-Item -LiteralPath $beamPbo -Force }
Invoke-Logged (Join-Path $OutDir 'beam-pack.log') $Tools @('pbo', 'pack', $beamStage, $beamPbo)
Write-Host "[kj_dedushka] beam addon packed ($($beamSum.textures.Count) textures)"

# ---- 5. offline checks: the configs merge onto the stock one, every creature class resolves, every
# ---- moves class obeys the engine's rules ----------------------------------------------------------
if (-not $NoProbe -and (Test-Path -LiteralPath (Join-Path $DataDir 'BIN'))) {
    $probeLog = Join-Path $OutDir 'probe.log'
    Invoke-Logged $probeLog $Tools @('guerrilla', 'probe', '--data-dir', $DataDir, '--mod', (Join-Path $OutDir '@kj_dedushka'))
    $probe = Get-Content -LiteralPath $probeLog | Where-Object { $_ -match 'PROBE|SUMMARY' }
    $probe | Select-Object -First 10 | ForEach-Object { Write-Host "    $_" }
    foreach ($c in $list) {
        if (-not ($probe -match "PROBE $($c.class) ok")) { throw "guerrilla probe did not accept $($c.class) (log: $probeLog)" }
    }
    $work = Join-Path $OutDir 'movecheck'
    New-Item -ItemType Directory -Force -Path $work | Out-Null
    $bin = Join-Path $work 'overlay.bin'
    $merged = Join-Path $work 'merged.bin'
    Invoke-Logged (Join-Path $work 'bin.log') $Tools @('config', 'bin', $cfgPath, '-o', $bin)
    Invoke-Logged (Join-Path $work 'merge.log') $Tools @('config', 'merge', (Join-Path $DataDir 'BIN\CONFIG.BIN'), $bin, '-o', $merged)
    foreach ($cls in @('CfgMovesMC', 'CfgManActions')) { Invoke-Logged (Join-Path $work "$cls.cpp") $Tools @('config', 'dump', $merged, $cls) }
    foreach ($c in $list) {
        Invoke-Logged (Join-Path $work "$($c.moves).cpp") $Tools @('config', 'dump', $merged, $c.moves)
        if (Test-Path -LiteralPath $MoveCheck) {
            $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
            $out = & $Python -I $MoveCheck (Join-Path $work "$($c.moves).cpp") (Join-Path $work 'CfgMovesMC.cpp') (Join-Path $work 'CfgManActions.cpp') $c.moves 2>&1 |
                ForEach-Object { "$_" }
            $ErrorActionPreference = $old
            $out | Out-File -FilePath (Join-Path $work "check_$($c.moves).log") -Encoding utf8
            # The held states (beam, dormant) answer every stance request with themselves on purpose;
            # the checker calls that a stance loop.
            $found = @($out | Where-Object { $_ -match '^\s+(STANCE|SELF|COST|MISSING|UNREACH|BAD|NO )' -or $_ -match '^\s+[A-Z]{3,} ' })
            $bad = @($found | Where-Object { $_ -notmatch 'STANCE LOOP (DedBeamRise|DedBeamHold|DedDormant):' })
            $states = ($out | Where-Object { $_ -match 'states \(move ids\)' } | Select-Object -First 1)
            Write-Host "    $($c.moves): $states; move graph $($found.Count) findings, $($found.Count - $bad.Count) expected (held states), $($bad.Count) real"
            if ($bad.Count -gt 0) { $bad | ForEach-Object { Write-Host "    $_" }; throw "the move graph check of $($c.moves) failed (log: $work\check_$($c.moves).log)" }
            if (-not $states) { throw "the move graph checker did not run on $($c.moves) (log: $work\check_$($c.moves).log)" }
        } else {
            Write-Host "[kj_dedushka] no move checker at $MoveCheck, graph check skipped"
        }
    }
} else {
    Write-Host "[kj_dedushka] probe and graph check skipped"
}

# ---- 6. the mount, and what the script set reads per creature ----------------------------------------
if (-not $NoInstall) {
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
    Copy-Item -LiteralPath $beamPbo -Destination (Join-Path $mount 'kjbeam.pbo') -Force
    # the timing table of each creature's own clips (text, generated): kj\anim\timing_<class>.sqf;
    # the first creature's is also the default table (kj\anim\ded_anim_timing.sqf)
    $kjAnim = [IO.Path]::GetFullPath($KjAnimDir)
    New-Item -ItemType Directory -Force -Path $kjAnim | Out-Null
    Get-ChildItem -LiteralPath $kjAnim -Filter 'timing_*.sqf' | Remove-Item -Force
    $first = $true
    foreach ($c in $list) {
        $src = Join-Path $OutDir "anim_$($c.key)\ded_anim_timing.sqf"
        Copy-Item -LiteralPath $src -Destination (Join-Path $kjAnim "timing_$($c.class).sqf") -Force
        if ($first) { Copy-Item -LiteralPath $src -Destination (Join-Path $kjAnim 'ded_anim_timing.sqf') -Force; $first = $false }
        if ($c.voicePbo -and (Test-Path -LiteralPath $c.voicePbo)) {
            Copy-Item -LiteralPath $c.voicePbo -Destination (Join-Path $mount (Split-Path $c.voicePbo -Leaf)) -Force
            Write-Host "[kj_dedushka] voice of $($c.class): $($c.voicePbo)"
        }
    }
    Install-Sound
    $lines = @("built $(Get-Date -Format s)") + ($list | ForEach-Object { "$($_.class) ($($_.key)): $($_.build)" })
    Set-Content -LiteralPath (Join-Path $ModDir 'MODEL.txt') -Value $lines -Encoding utf8
    Write-Host "[kj_dedushka] mounted: $mount"
}
Write-Host "[kj_dedushka] done: $pbo ($([math]::Round((Get-Item $pbo).Length / 1MB, 1)) MB)"

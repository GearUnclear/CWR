# Builds the creature addon with the anim feature's clips and moves class and
# mounts it as its own mod folder, @kj_anim, beside the Remaster shadow. The
# pipeline's files and its @uddedushka are only read, never written.
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-kj-anim.ps1 [-ModelBuild <folder>] [-Rest <joint file>]
#       [-OutDir <folder>] [-NoInstall] [-NoProbe]
#
# -ModelBuild  a finished build of the pipeline's build-uddedushka.ps1: the
#              folder that holds dedushka.p3d, dedushka_rest.json and
#              stage\uddedushka\*.paa. Default: kaiju-notes\pipeline\build
#              (the proxy). For the final model, point this at its build.
# -Rest        the model's joint file, when it is not <ModelBuild>\dedushka_rest.json.
# -OutDir      every built file (clips, staging, pbo, logs). Default:
#              kaiju-notes\features\anim\build. Nothing binary goes into the repo.
#
# What it does: runs gen_ded_anim.py on the joint file; takes the pipeline's
# config.cpp and puts ded_anim_gen.hpp + moves_dedushka.hpp in place of its
# CfgMovesDedushka (CfgPatches, CfgModels and CfgVehicles stay the pipeline's);
# packs <OutDir>\@kj_anim\addons\uddedushka.pbo (the pbo keeps the name
# uddedushka, because a pbo's file name is its path prefix); checks the move
# graph offline; copies the pbo to <git root>\tmp\@kj_anim. A test mounts it
# with --mods-dir ".." --mod "@kj_anim" IN PLACE OF @uddedushka (both carry
# the addon uddedushka, so never mount the two together).
param(
    [string]$ModelBuild = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\build',
    [string]$Rest = '',
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\features\anim\build',
    [switch]$NoInstall,
    [switch]$NoProbe,
    [string]$DataDir = '',
    [string]$ModDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$MoveCheck = 'D:\Arma_CWA_worktrees\cryptid-notes\anim\moves\check_moves.py',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\features\anim -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$pipeConfig = Join-Path $here '..\..\addon\uddedushka\config.cpp'
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_anim' }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
if (-not $Rest) { $Rest = Join-Path $ModelBuild 'dedushka_rest.json' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)
$p3d = Join-Path $ModelBuild 'dedushka.p3d'
$paaDir = Join-Path $ModelBuild 'stage\uddedushka'
foreach ($f in @($Tools, $pipeConfig, $p3d, $Rest, $paaDir, (Join-Path $here 'moves_dedushka.hpp'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}

function Invoke-Logged([string]$log, [string]$exe, [string[]]$argv) {
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { & $exe @argv 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $log -Encoding utf8 }
    finally { $ErrorActionPreference = $old }
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $log -Tail 25 | ForEach-Object { Write-Host "    $_" }
        throw "$exe failed with exit $LASTEXITCODE (log: $log)"
    }
}

# 1. Clips for this model's joints.
$animDir = Join-Path $OutDir 'anim'
if (Test-Path -LiteralPath $animDir) { Remove-Item -LiteralPath $animDir -Recurse -Force }
Invoke-Logged (Join-Path $OutDir 'gen.log') $Python @((Join-Path $here 'gen_ded_anim.py'), '--rest', $Rest, '--out', $animDir)
Get-Content -LiteralPath (Join-Path $animDir 'ded_anim_report.txt') -Tail 6 | ForEach-Object { Write-Host "    $_" }

# 2. Config: the pipeline's, with its moves class swapped for ours.
$pipe = [IO.File]::ReadAllText($pipeConfig)
$a = $pipe.IndexOf('class CfgPatches')
$b = $pipe.IndexOf('class CfgManActions')
$c = $pipe.IndexOf('class CfgVehicles')
if ($a -lt 0 -or $b -le $a -or $c -le $b) { throw "the pipeline config.cpp no longer has CfgPatches, CfgManActions, CfgVehicles in that order" }
$head = "// uddedushka with the anim feature's moves class. Built by features\anim\build-kj-anim.ps1:`r`n" +
    "// CfgPatches, CfgModels and CfgVehicles are the pipeline's config.cpp, CfgMovesDedushka is`r`n" +
    "// ded_anim_gen.hpp (generated) + moves_dedushka.hpp. Do not edit this copy.`r`n" +
    "#define private   0`r`n#define protected 1`r`n#define public    2`r`n`r`n"
$config = $head + $pipe.Substring($a, $b - $a) +
    [IO.File]::ReadAllText((Join-Path $animDir 'ded_anim_gen.hpp')) + "`r`n" +
    [IO.File]::ReadAllText((Join-Path $here 'moves_dedushka.hpp')) + "`r`n" + $pipe.Substring($c)

# 3. Stage.
$stage = Join-Path $OutDir 'stage\uddedushka'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path (Join-Path $stage 'anim') | Out-Null
[IO.File]::WriteAllText((Join-Path $stage 'config.cpp'), $config, (New-Object Text.UTF8Encoding($false)))
Copy-Item -LiteralPath $p3d -Destination (Join-Path $stage 'dedushka.p3d')
Get-ChildItem -LiteralPath $paaDir -Filter *.paa | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $stage }
$clips = [regex]::Matches($config, '\\uddedushka\\anim\\([A-Za-z0-9_]+\.rtm)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
foreach ($clip in $clips) {
    $src = Join-Path $animDir $clip
    if (-not (Test-Path -LiteralPath $src)) { throw "the moves class names $clip but gen_ded_anim.py did not make it" }
    Copy-Item -LiteralPath $src -Destination (Join-Path $stage "anim\$clip")
}
Write-Host "[kj_anim] staged $($clips.Count) clips"

# 4. Pack.
$addons = Join-Path $OutDir '@kj_anim\addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)

# 5. Offline checks: the config merges onto the stock one, the class resolves,
# and the move graph obeys the engine's rules.
if (-not $NoProbe -and (Test-Path -LiteralPath (Join-Path $DataDir 'BIN'))) {
    $probeLog = Join-Path $OutDir 'probe.log'
    Invoke-Logged $probeLog $Tools @('guerrilla', 'probe', '--data-dir', $DataDir, '--mod', (Join-Path $OutDir '@kj_anim'))
    $probe = Get-Content -LiteralPath $probeLog | Where-Object { $_ -match 'PROBE|SUMMARY' }
    $probe | ForEach-Object { Write-Host "    $_" }
    if (-not ($probe -match 'PROBE UDDedushka ok')) { throw "guerrilla probe did not accept UDDedushka (log: $probeLog)" }
    $work = Join-Path $OutDir 'movecheck'
    New-Item -ItemType Directory -Force -Path $work | Out-Null
    $bin = Join-Path $work 'overlay.bin'
    $merged = Join-Path $work 'merged.bin'
    Invoke-Logged (Join-Path $work 'bin.log') $Tools @('config', 'bin', (Join-Path $stage 'config.cpp'), '-o', $bin)
    Invoke-Logged (Join-Path $work 'merge.log') $Tools @('config', 'merge', (Join-Path $DataDir 'BIN\CONFIG.BIN'), $bin, '-o', $merged)
    foreach ($cls in @('CfgMovesDedushka', 'CfgMovesMC', 'CfgManActions')) {
        Invoke-Logged (Join-Path $work "$cls.cpp") $Tools @('config', 'dump', $merged, $cls)
    }
    if (Test-Path -LiteralPath $MoveCheck) {
        $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
        $out = & $Python -I $MoveCheck (Join-Path $work 'CfgMovesDedushka.cpp') (Join-Path $work 'CfgMovesMC.cpp') (Join-Path $work 'CfgManActions.cpp') CfgMovesDedushka 2>&1 |
            ForEach-Object { "$_" }
        $ErrorActionPreference = $old
        $out | Out-File -FilePath (Join-Path $work 'check_moves.log') -Encoding utf8
        $out | Where-Object { $_ -match 'states|edges|spawn default' } | ForEach-Object { Write-Host "    $_" }
        # The held states (beam, dormant) answer every stance request with
        # themselves on purpose; the checker calls that a stance loop.
        $found = @($out | Where-Object { $_ -match '^\s+(STANCE|SELF|COST|MISSING|UNREACH|BAD|NO )' -or $_ -match '^\s+[A-Z]{3,} ' })
        $bad = @($found | Where-Object { $_ -notmatch 'STANCE LOOP (DedBeamRise|DedBeamHold|DedDormant):' })
        Write-Host "    move graph: $($found.Count) findings, $($found.Count - $bad.Count) expected (held states), $($bad.Count) real"
        if ($bad.Count -gt 0) { $bad | ForEach-Object { Write-Host "    $_" }; throw "the move graph check failed (log: $work\check_moves.log)" }
        if (-not ($out -match 'states \(move ids\)')) { throw "the move graph checker did not run (log: $work\check_moves.log)" }
    } else {
        Write-Host "[kj_anim] no move checker at $MoveCheck, graph check skipped"
    }
} else {
    Write-Host "[kj_anim] probe and graph check skipped"
}

# 6. The timing table for mission scripts (text, generated): beside the payload and in the test mission.
foreach ($dst in @((Join-Path $here 'kj\anim'), (Join-Path $here 'DedAnim.Eden\kj\anim'))) {
    New-Item -ItemType Directory -Force -Path $dst | Out-Null
    Copy-Item -LiteralPath (Join-Path $animDir 'ded_anim_timing.sqf') -Destination $dst -Force
}
Copy-Item -LiteralPath (Join-Path $animDir 'FOOTFALLS.md') -Destination (Join-Path $here 'FOOTFALLS.md') -Force

# 7. Mount copy beside the shadow, in our own mod folder.
if (-not $NoInstall) {
    $mount = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
    Write-Host "[kj_anim] mounted copy: $(Join-Path $mount 'uddedushka.pbo')"
}
Write-Host "[kj_anim] done: $pbo"

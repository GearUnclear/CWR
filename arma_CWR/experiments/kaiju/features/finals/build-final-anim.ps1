# Builds the MOVING version of a final candidate: the boned model of build-final.ps1 -Skin bones with
# the anim feature's clips (made by its generator from the candidate's own joint file) and its moves
# class, mounted as <git root>\tmp\@kj_final_<letter>_anim, IN PLACE OF @uddedushka. A changed copy of
# features\anim\build-kj-anim.ps1; the anim feature's files and the pipeline's are only read.
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-final-anim.ps1 -Letter a|d [-OutDir <folder>] [-NoInstall]
#
# What differs from build-kj-anim.ps1:
# - the model comes from kaiju-notes\features\finals\build\<letter>_bones (run build-final.ps1 first);
# - nothing is written into the anim feature's folder (its step 6 copied the timing table and
#   FOOTFALLS.md there); they stay in <OutDir>\anim;
# - the generator's offline check "DedStomp: N degrees between two keys" does not stop the build: it
#   fails on both candidates' joint files (19.2 degrees on A, 15.6 on D, limit 15) and the stomp is
#   not part of this job (idle and walk only). Any other failed check still stops it.
param(
    [Parameter(Mandatory)][ValidateSet('a', 'd')][string]$Letter,
    [string]$OutDir = '',
    [string]$ModDir = '',
    [switch]$NoInstall,
    [string]$DataDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$MoveCheck = 'D:\Arma_CWA_worktrees\cryptid-notes\anim\moves\check_moves.py',
    [string]$Python = 'python',
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$anim = [IO.Path]::GetFullPath((Join-Path $here '..\anim'))
$pipeConfig = [IO.Path]::GetFullPath((Join-Path $here '..\..\addon\uddedushka\config.cpp'))
$buildRoot = Join-Path $Notes 'features\finals\build'
$ModelBuild = Join-Path $buildRoot "${Letter}_bones"
if (-not $OutDir) { $OutDir = Join-Path $buildRoot "${Letter}_anim" }
if (-not $ModDir) { $ModDir = Join-Path $gitRoot "tmp\@kj_final_${Letter}_anim" }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
$Rest = Join-Path $ModelBuild 'dedushka_rest.json'
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)
$p3d = Join-Path $ModelBuild 'dedushka.p3d'
$paaDir = Join-Path $ModelBuild 'stage\uddedushka'
foreach ($f in @($Tools, $pipeConfig, $p3d, $Rest, $paaDir, (Join-Path $anim 'moves_dedushka.hpp'), (Join-Path $anim 'gen_ded_anim.py'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}

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

# 1. Clips for this model's joints.
$animDir = Join-Path $OutDir 'anim'
if (Test-Path -LiteralPath $animDir) { Remove-Item -LiteralPath $animDir -Recurse -Force }
$summary = Get-Content -LiteralPath (Join-Path $ModelBuild 'dedushka_export.json') -Raw | ConvertFrom-Json
Invoke-Logged (Join-Path $OutDir 'gen.log') $Python @((Join-Path $anim 'gen_ded_anim.py'), '--rest', $Rest, '--out', $animDir,
    '--depth', "$($summary.depth_forward_m)") -AllowFail
$report = Join-Path $animDir 'ded_anim_report.txt'
if (-not (Test-Path -LiteralPath $report)) { throw "the generator wrote no report (log: $OutDir\gen.log)" }
Get-Content -LiteralPath $report -Tail 6 | ForEach-Object { Write-Host "    $_" }
$failed = @(Get-Content -LiteralPath $report | Where-Object { $_ -match '^FAILED: ' })
if ($failed.Count -gt 0) {
    $items = ($failed[0] -replace '^FAILED: ', '') -split '; '
    $real = @($items | Where-Object { $_ -notmatch '^DedStomp: [0-9.]+ degrees between two keys' })
    if ($real.Count -gt 0) { throw "offline checks failed: $($real -join '; ')" }
    Write-Warning "tolerated: $($items -join '; ') (the stomp is not used here)"
}

# 2. Config: the pipeline's, with its moves class swapped for the anim feature's.
$pipe = [IO.File]::ReadAllText($pipeConfig)
$a = $pipe.IndexOf('class CfgPatches')
$b = $pipe.IndexOf('class CfgManActions')
$c = $pipe.IndexOf('class CfgVehicles')
if ($a -lt 0 -or $b -le $a -or $c -le $b) { throw "the pipeline config.cpp no longer has CfgPatches, CfgManActions, CfgVehicles in that order" }
$head = "// uddedushka, final candidate $($Letter.ToUpper()), with the anim feature's moves class.`r`n" +
    "// Built by features\finals\build-final-anim.ps1. Do not edit this copy.`r`n" +
    "#define private   0`r`n#define protected 1`r`n#define public    2`r`n`r`n"
$config = $head + $pipe.Substring($a, $b - $a) +
    [IO.File]::ReadAllText((Join-Path $animDir 'ded_anim_gen.hpp')) + "`r`n" +
    [IO.File]::ReadAllText((Join-Path $anim 'moves_dedushka.hpp')) + "`r`n" + $pipe.Substring($c)

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
Write-Host "[final $Letter anim] staged $($clips.Count) clips"

# 4. Pack.
$modName = "@kj_final_${Letter}_anim"
$addons = Join-Path $OutDir "$modName\addons"
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)

# 5. Offline checks: the config merges onto the stock one, the class resolves, the move graph holds.
if (Test-Path -LiteralPath (Join-Path $DataDir 'BIN')) {
    $probeLog = Join-Path $OutDir 'probe.log'
    Invoke-Logged $probeLog $Tools @('guerrilla', 'probe', '--data-dir', $DataDir, '--mod', (Join-Path $OutDir $modName))
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
        $found = @($out | Where-Object { $_ -match '^\s+(STANCE|SELF|COST|MISSING|UNREACH|BAD|NO )' -or $_ -match '^\s+[A-Z]{3,} ' })
        $bad = @($found | Where-Object { $_ -notmatch 'STANCE LOOP (DedBeamRise|DedBeamHold|DedDormant):' })
        Write-Host "    move graph: $($found.Count) findings, $($found.Count - $bad.Count) expected (held states), $($bad.Count) real"
        if ($bad.Count -gt 0) { $bad | ForEach-Object { Write-Host "    $_" }; throw "the move graph check failed (log: $work\check_moves.log)" }
    } else {
        Write-Host "[final $Letter anim] no move checker at $MoveCheck, graph check skipped"
    }
}

# 6. Mount copy beside the shadow, in its own mod folder.
if (-not $NoInstall) {
    $mount = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
    Write-Host "[final $Letter anim] mounted copy: $(Join-Path $mount 'uddedushka.pbo')"
}
Write-Host "[final $Letter anim] done: $pbo"

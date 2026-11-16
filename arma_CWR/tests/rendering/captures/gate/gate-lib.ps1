# gate-lib.ps1: shared pieces of perf-bench.ps1 and run-vgate.ps1 (dot-source it).
# Windows PowerShell 5.1. Launches nothing by itself; never kills anything except, through
# stop-owned-games.ps1 -DataDir, the games of the caller's own data junction.

$script:GateDir = $PSScriptRoot
$script:CapturesDir = Split-Path -Parent $script:GateDir
$script:EngineRoot = (Resolve-Path (Join-Path $script:GateDir '..\..\..\..')).Path   # arma_CWR
$script:GitRoot = Split-Path -Parent $script:EngineRoot
$script:MainRoot = $script:GitRoot
try {
    $common = & git -C $script:EngineRoot rev-parse --path-format=absolute --git-common-dir 2>$null
    if ($LASTEXITCODE -eq 0 -and $common) { $script:MainRoot = Split-Path -Parent ($common | Select-Object -First 1).Trim() }
} catch { }
# The squad folder (tmp\perf-squad of the integration worktree) holds the frozen tool copies.
$script:SquadRoot = $env:PERF_SQUAD_ROOT
if (-not $script:SquadRoot) { $script:SquadRoot = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\perf-squad' }

function Get-FirstExisting([string[]]$Candidates) {
    foreach ($c in $Candidates) { if ($c -and (Test-Path -LiteralPath $c)) { return $c } }
    return $Candidates[0]
}

function Resolve-GateTools {
    param([string]$Tri = '', [string]$HiddenDesktop = '', [string]$StopGames = '', [string]$ShadowDir = '', [string]$Python = '')
    $bin = Join-Path $script:SquadRoot 'bin'
    $t = [ordered]@{
        tri           = $(if ($Tri) { $Tri } else { Get-FirstExisting @((Join-Path $bin 'tri.exe'), (Join-Path $script:MainRoot 'arma_CWR\target\debug\tri.exe')) })
        hidden        = $(if ($HiddenDesktop) { $HiddenDesktop } else { Get-FirstExisting @((Join-Path $bin 'tri-hidden-desktop.ps1'), (Join-Path $script:MainRoot 'tmp\tri-hidden-desktop.ps1')) })
        stop          = $(if ($StopGames) { $StopGames } else { Get-FirstExisting @((Join-Path $bin 'stop-owned-games.ps1'), (Join-Path $script:MainRoot 'arma_CWR\tools\stop-owned-games.ps1')) })
        shadow        = $(if ($ShadowDir) { $ShadowDir } else { Get-FirstExisting @((Join-Path $script:GitRoot 'tmp\remaster-shadow-67'), (Join-Path $script:MainRoot 'tmp\remaster-shadow-67')) })
        python        = $(if ($Python) { $Python } else { 'python' })
        sampler       = (Join-Path $script:CapturesDir 'gpu_sampler.ps1')
    }
    return $t
}

function Write-GateLog([string]$Path, [string]$Line) {
    $l = "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') $Line"
    for ($i = 0; $i -lt 20; $i++) { try { [IO.File]::AppendAllText($Path, "$l`n"); break } catch { Start-Sleep -Milliseconds 100 } }
    Write-Host $l
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding $false))
}

# ---------------------------------------------------------------- the fixed configuration

function New-SeededUserDir {
    # A fresh user dir with graphics.cfg pinned (PERF-PLAN 3.3, P0-7). tri passes an inherited
    # POSEIDON_USER_DIR to the game (integration.rs run_sqf_test) and sets POSEIDON_CACHE_DIR to
    # the same dir, so the program cache (gl33_shaders.bin) is per boot too. Config 'fixed':
    # Ultra preset, fpsCap 0 (Unlimited), vsync 0; 'legacy': round 1's fpsCap 60 and vsync on
    # (what a first boot autodetects on this 60 Hz display, GameApplication.cpp:394-411).
    param([Parameter(Mandatory)][string]$Dir, [ValidateSet('fixed', 'legacy')][string]$Config = 'fixed')
    if (Test-Path -LiteralPath $Dir) { throw "user dir already exists (a boot must start fresh): $Dir" }
    New-Item -ItemType Directory -Force -Path $Dir | Out-Null
    $cap = if ($Config -eq 'legacy') { 60 } else { 0 }
    $vs = if ($Config -eq 'legacy') { 1 } else { 0 }
    # GraphicsConfig.hpp: Preset Ultra=3; Tier Ultra=4, High=3 (the Ultra bundle caps shadow and
    # particles at High); VsyncMode Off=0 On=1; kVersion 3 (so Migrate leaves fpsCap 0 alone).
    $cfg = @(
        'qualityPreset=3;', 'terrainDetail=4;', 'objectLod=4;', 'shadowQuality=3;', 'particlesQuality=3;',
        "vsync=$vs;", "fpsCap=$cap;", 'alphaToCoverage=1;', 'renderScale=1.000000;', 'msaaSamples=4;',
        'multitexturing=1;', 'lighting=1;', 'brightness=1.600000;', 'gamma=1.200000;', 'version=3;'
    ) -join "`r`n"
    Write-Utf8NoBom (Join-Path $Dir 'graphics.cfg') ($cfg + "`r`n")
    return @{ dir = $Dir; config = $Config; fpsCap = $cap; vsync = $vs }
}

# ---------------------------------------------------------------- what a dist can do

function Get-ExeInfo {
    param([Parameter(Mandatory)][string]$Dist)
    $exe = Join-Path $Dist 'PoseidonGame.exe'
    if (-not (Test-Path -LiteralPath $exe)) { throw "no PoseidonGame.exe in $Dist" }
    $fi = Get-Item -LiteralPath $exe
    $sha = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
    # Verbs register by string literal (GGameState.NewFunction(... "triGpuRing" ...)), so their
    # names sit in the binary as ASCII. A byte scan tells which gate verbs this dist has.
    $bytes = [IO.File]::ReadAllBytes($exe)
    $text = [Text.Encoding]::GetEncoding(28591).GetString($bytes)
    $verbs = [ordered]@{}
    foreach ($v in 'triPerfRing', 'triGpuRing', 'triSetFpsCap', 'triResetTemporal', 'triSeedRandom', 'triDeterministic', 'triBuildId', 'triRecord', 'triSetVsync', 'triPerfReset') {
        $verbs[$v] = $text.Contains('"' + $v) -or $text.Contains($v + [char]0)
    }
    $bytes = $null; $text = $null
    $wt = $null; $describe = $null
    $p = $Dist
    for ($i = 0; $i -lt 6 -and $p; $i++) {
        if (Test-Path -LiteralPath (Join-Path $p '.git')) { $wt = $p; break }
        $p = Split-Path -Parent $p
    }
    if ($wt) { try { $describe = (& git -C $wt describe --always --dirty --abbrev=10 2>$null | Select-Object -First 1) } catch { } }
    return [ordered]@{ exe = $exe; sha256 = $sha; size = $fi.Length; mtime = $fi.LastWriteTimeUtc.ToString('o');
        build_id = "exe-$($sha.Substring(0, 12))"; worktree = $wt; git_describe = $describe; verbs = $verbs }
}

# ---------------------------------------------------------------- quiet probe (PERF-PLAN 2.5)

$script:QuietPrevCpu = @{}
function Get-QuietReasons {
    # Reasons the machine is not quiet right now (empty when quiet). Takes about 2 s (two CPU
    # samples 1.5 s apart, one pmon). $OwnDataDir: our games' -C path; $OwnTag: a string in
    # our tri command line (the boot's output dir). Reads only. $SmOverrides: per-process SM limits
    # (name prefix=percent; default Discord=40: Discord-only windows measured within the clean spread,
    # STEWARD-1 2026-10-31, perf-squad RESUME.md 6); a process under its override is not a reason.
    param([string]$OwnDataDir = '', [string]$OwnTag = '', [int]$SmThreshold = 15, [string]$Tri = '',
          [string]$SmOverrides = 'Discord=40')
    $smOv = @{}
    foreach ($kv in @($SmOverrides -split '[;,]' | Where-Object { $_ -match '=' })) { $k, $v = $kv -split '=', 2; $smOv[$k.Trim()] = [int]$v }
    $r = @()
    $pats = @{ owner = '^(deadlock|project8)'; wsl = '^vmmem' }
    $c0 = @{}
    foreach ($k in $pats.Keys) { $c0[$k] = 0.0; foreach ($p in @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match $pats[$k] })) { try { $c0[$k] += $p.TotalProcessorTime.TotalSeconds } catch { } } }
    Start-Sleep -Milliseconds 1500
    foreach ($k in $pats.Keys) {
        $c1 = 0.0; $n = 0
        foreach ($p in @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match $pats[$k] })) { $n++; try { $c1 += $p.TotalProcessorTime.TotalSeconds } catch { } }
        $d = $c1 - $c0[$k]
        if ($k -eq 'owner' -and $n -gt 0 -and $d -gt 0.3) { $r += "owner-cpu:$([math]::Round($d, 2))" }
        if ($k -eq 'wsl' -and $d -gt 0.6) { $r += "vmmem-cpu:$([math]::Round($d, 2))" }
    }
    try {
        foreach ($l in @(& nvidia-smi pmon -c 1 2>$null)) {
            if ($l -match '^\s*#' -or -not $l.Trim()) { continue }
            $f = $l.Trim() -split '\s+'
            if ($f.Count -lt 5) { continue }
            $sm = $f[3]; $name = $f[-1]
            if ($sm -notmatch '^\d+$' -or $name -match '^(PoseidonGam|claude)') { continue }
            $lim = $SmThreshold
            foreach ($k in $smOv.Keys) { if (($name -replace '\.exe$', '') -like "$k*") { $lim = $smOv[$k] } }
            if (($name -match '^(deadlock|project8)' -and [int]$sm -ge 3) -or [int]$sm -ge $lim) { $r += "gpu:$name=$sm" }
        }
    } catch { }
    $b = @(Get-Process ninja, cmake, clang-cl, cl, lld-link, link, cargo, rustc, ffmpeg, glslangValidator -ErrorAction SilentlyContinue | ForEach-Object { $_.ProcessName } | Select-Object -Unique)
    if ($b.Count) { $r += "build:$($b -join '+')" }
    $own = if ($OwnDataDir) { [regex]::Escape($OwnDataDir.TrimEnd('\', '/')) } else { $null }
    foreach ($g in @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue)) {
        if (-not $own -or $g.CommandLine -notmatch $own) { $r += "poseidon-foreign:$($g.ProcessId)" }
    }
    foreach ($t in @(Get-CimInstance Win32_Process -Filter "Name='tri.exe'" -ErrorAction SilentlyContinue)) {
        if ($t.CommandLine -match '\bslots\b') { continue }
        if (-not $OwnTag -or $t.CommandLine -notmatch [regex]::Escape($OwnTag)) { $r += "tri-foreign:$($t.ProcessId)" }
    }
    $m2 = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -like '*measure2*queue.ps1*' })
    if ($m2.Count) { $r += "m2-runner-alive:$($m2[0].ProcessId)" }
    $locks = Join-Path $script:SquadRoot 'locks'
    if (Test-Path -LiteralPath $locks) {
        foreach ($f in @(Get-ChildItem -LiteralPath $locks -Filter 'HEAVY.*.lock' -File -ErrorAction SilentlyContinue)) {
            try { $rec = Get-Content -Raw -LiteralPath $f.FullName | ConvertFrom-Json } catch { $rec = $null }
            if ($rec -and $rec.pid -and (Get-Process -Id ([int]$rec.pid) -ErrorAction SilentlyContinue)) { $r += "heavy:$($f.Name)" }
        }
    }
    if ($Tri -and (Test-Path -LiteralPath $Tri)) {
        try {
            $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
            $held = (& $Tri slots 2>$null) -join ' '
            $ErrorActionPreference = $eap
            if ($held -and $held -notmatch 'slots held: 0/') { $r += 'tri-slot-held' }
        } catch { $ErrorActionPreference = $eap }
    }
    return , $r
}

function Wait-Quiet {
    # Waits for QuietSec of continuous quiet. The owner's Deadlock pauses the job and is re-checked
    # every 2 min (PERF-PLAN 2.5); anything else every 15 s. Returns $true when quiet, $false when
    # MaxPauseMin ran out (the caller marks the job contended). Never kills anything.
    param([string]$Log, [int]$QuietSec = 30, [int]$MaxPauseMin = 120, [string]$OwnDataDir = '', [string]$Tri = '')
    $deadline = (Get-Date).AddMinutes($MaxPauseMin)
    $since = $null; $last = ''
    while ((Get-Date) -lt $deadline) {
        $r = Get-QuietReasons -OwnDataDir $OwnDataDir -Tri $Tri
        if ($r.Count -eq 0) {
            if (-not $since) { $since = Get-Date }
            if (((Get-Date) - $since).TotalSeconds -ge $QuietSec) { return $true }
            Start-Sleep -Seconds 3
            continue
        }
        $since = $null
        $s = ($r -join ' ')
        if ($s -ne $last) { Write-GateLog $Log "not quiet: $s"; $last = $s }
        if ($s -match 'owner-|deadlock|project8') { Start-Sleep -Seconds 120 } else { Start-Sleep -Seconds 15 }
    }
    Write-GateLog $Log "gave up waiting for quiet after $MaxPauseMin min: $last"
    return $false
}

# ---------------------------------------------------------------- one boot

function Set-BootEnv {
    # Applies an env map; returns the previous values for Restore-BootEnv. A value of $null or ''
    # UNSETS the variable (POSEIDON_OBSERVER_SAMPLE is tested for presence only).
    param([hashtable]$Env)
    $prev = @{}
    foreach ($k in $Env.Keys) {
        $prev[$k] = [Environment]::GetEnvironmentVariable($k, 'Process')
        $v = $Env[$k]
        if ($null -eq $v -or "$v" -eq '') { [Environment]::SetEnvironmentVariable($k, $null, 'Process') }
        else { [Environment]::SetEnvironmentVariable($k, "$v", 'Process') }
    }
    return $prev
}
function Restore-BootEnv([hashtable]$Prev) {
    foreach ($k in $Prev.Keys) { [Environment]::SetEnvironmentVariable($k, $Prev[$k], 'Process') }
}

function Invoke-GateBoot {
    # One game boot: fresh seeded user dir, env, the GPU/foreign sampler, tri on the hidden
    # desktop with -j 1 and TRI_GLOBAL_SLOTS=1, then the scoped cleanup of our own games.
    param(
        [Parameter(Mandatory)][string]$BootDir, [Parameter(Mandatory)][string]$Dist, [Parameter(Mandatory)][string[]]$Tests,
        [Parameter(Mandatory)][string]$DataDir, [Parameter(Mandatory)]$Tools, [hashtable]$Env = @{},
        [ValidateSet('fixed', 'legacy')][string]$Config = 'fixed', [string]$Log, [int]$TimeoutSec = 5400,
        [switch]$DryRun, [int]$Retries = 0, [string]$UserDir = ''
    )
    New-Item -ItemType Directory -Force -Path $BootDir | Out-Null
    # -UserDir: reuse an earlier boot's user dir (and so its program cache), for the warm half of a
    # boot-cache pair; otherwise a fresh seeded one (P0-7).
    if ($UserDir) { $user = @{ dir = $UserDir; config = $Config; shared = $true } }
    else { $user = New-SeededUserDir -Dir (Join-Path $BootDir 'user') -Config $Config }
    $triOut = Join-Path $BootDir 'tri'
    New-Item -ItemType Directory -Force -Path $triOut | Out-Null
    $triLog = Join-Path $BootDir 'tri.log'
    $samples = Join-Path $BootDir 'gpu_samples.csv'
    foreach ($p in @($Tools.tri, $triOut, $Dist, $triLog, $DataDir) + $Tests) {
        if ($p -match '\s') { throw "path with whitespace is not supported by the hidden-desktop wrapper: $p" }
    }
    $testArgs = ($Tests | ForEach-Object { $_ -replace '\\', '/' }) -join ' '
    $cmd = "$($Tools.tri) test --retries $Retries -j 1 --global-slots 1 --timeout 120 --game-dir $Dist --data-dir $DataDir --output-dir $triOut $testArgs"
    $envAll = @{}
    foreach ($k in $Env.Keys) { $envAll[$k] = $Env[$k] }
    $envAll['TRI_GLOBAL_SLOTS'] = '1'
    $envAll['POSEIDON_USER_DIR'] = $user.dir
    # The program cache dir: tri sets POSEIDON_CACHE_DIR to the user dir (integration.rs), so each boot's
    # fresh user dir is also its own program cache dir (P0-7). TRI_POSEIDON_CACHE_DIR (P0-14) is NOT set:
    # the frozen tri ignores it and INFRA-C1's bootstrap gate asks for no new variables.
    $rec = [ordered]@{ boot_dir = $BootDir; dist = $Dist; data_dir = $DataDir; config = $Config; user_dir = $user.dir;
        cmd = $cmd; env = $envAll; tests = @($Tests | ForEach-Object { [ordered]@{ path = $_; sha256 = (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant() } });
        start = (Get-Date).ToString('o'); exit = $null; end = $null; dry_run = [bool]$DryRun }
    if ($DryRun) {
        if ($Log) { Write-GateLog $Log "DRY RUN boot $BootDir`n    env: $(($envAll.Keys | Sort-Object | ForEach-Object { "$_=$($envAll[$_])" }) -join ' ')`n    cmd: $cmd" }
        $rec.exit = 0; $rec.end = (Get-Date).ToString('o')
        return $rec
    }
    $prev = Set-BootEnv $envAll
    $sampler = $null
    try {
        Remove-Item "$samples.stop" -ErrorAction SilentlyContinue
        $sampler = Start-Process powershell -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $Tools.sampler, '-Out', $samples,
            '-Seconds', "$($TimeoutSec + 600)", '-Every', '3', '-Junction', $DataDir) -WindowStyle Hidden -PassThru
        if ($Log) { Write-GateLog $Log "boot start: $cmd" }
        & powershell -NoProfile -ExecutionPolicy Bypass -File $Tools.hidden -Log $triLog -WorkDir $script:EngineRoot -Cmd $cmd -TimeoutSec $TimeoutSec | Out-Host
        $rec.exit = $LASTEXITCODE
    } finally {
        Restore-BootEnv $prev
        New-Item -ItemType File -Force "$samples.stop" | Out-Null
        if (Test-Path -LiteralPath $Tools.stop) {
            & powershell -NoProfile -ExecutionPolicy Bypass -File $Tools.stop -DataDir $DataDir | Out-File -FilePath (Join-Path $BootDir 'stop-owned-games.txt') -Encoding utf8
        }
        if ($sampler) { try { $null = $sampler.WaitForExit(15000) } catch { } }
        $rec.end = (Get-Date).ToString('o')
    }
    if ($Log) { Write-GateLog $Log "boot end: exit=$($rec.exit) $BootDir" }
    return $rec
}

function Resolve-DataDir {
    # The data junction the games use (`-C`); created on demand as a junction to the shadow, as
    # run-captures.ps1 does. Never a recursive delete through it.
    param([string]$Junction = '', [string]$DataDir = '', $Tools, [switch]$NoCreate)
    if ($DataDir) { return $DataDir }
    if (-not $Junction) { throw 'need -Junction or -DataDir' }
    if (-not [IO.Path]::IsPathRooted($Junction) -and $Junction -notmatch '^[A-Za-z0-9_-]+$') { throw "-Junction must be a plain name or an absolute path: $Junction" }
    if ([IO.Path]::IsPathRooted($Junction)) { $j = $Junction } else {
        # RESUME.md 4: junction names live under the integration worktree's arma_CWR\packages
        $integ = Join-Path (Split-Path -Parent (Split-Path -Parent $script:SquadRoot)) "arma_CWR\packages\$Junction"
        $j = Get-FirstExisting @($integ, (Join-Path $script:EngineRoot "packages\$Junction"))
    }
    if ($NoCreate) { return $j }
    if (-not (Test-Path -LiteralPath $j)) {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $j) | Out-Null
        & cmd /c mklink /J "$j" "$($Tools.shadow)" | Out-Null
    }
    if (-not (Test-Path -LiteralPath (Join-Path $j 'dtaExt'))) { throw "$j is not the Remaster shadow (no dtaExt)" }
    return $j
}

function ConvertTo-EnvMap($Obj) {
    # A JSON object, a hashtable or 'K=V' strings -> hashtable.
    $h = @{}
    if ($null -eq $Obj) { return $h }
    if ($Obj -is [hashtable]) { foreach ($k in $Obj.Keys) { $h[$k] = $Obj[$k] }; return $h }
    if ($Obj -is [string] -or $Obj -is [array]) {
        foreach ($s in @($Obj)) { if ("$s" -match '^([^=]+)=(.*)$') { $h[$Matches[1]] = $Matches[2] } }
        return $h
    }
    foreach ($p in $Obj.PSObject.Properties) { $h[$p.Name] = $p.Value }
    return $h
}

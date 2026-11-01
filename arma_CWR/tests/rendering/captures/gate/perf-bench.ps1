<#
.SYNOPSIS
    The tracked perf bench (PERF-PLAN P0-3, 3.3 "Perf gate"): one job = one set of boots of one
    or two builds at the fixed configuration, then bench_report.py. Called by the squad queue
    runner (tmp\perf-squad\bin\squad-queue.ps1), which holds the machine lock and did the 30 s
    quiet check. Nobody else runs it (only the runner launches games, PERF-PLAN 2.5).

.DESCRIPTION
    perf-bench.ps1 -Set quick|abba|abba-scoped|full|obs|hitch|legacy -Out <caller folder>
                   -Candidate <dist> [-Baseline <dist>] [-Views v1,v2] [-Rounds n] [-Boots n]
                   [-Cycles n] [-Env K=V,...] [-CandidateEnv K=V,...] [-BaselineEnv K=V,...]
                   [-Read auto|meas|ring] [-Junction <name> | -DataDir <path>] [-NoObserverSample]
                   [-QuietSec 30] [-MaxPauseMin 120] [-TimeoutSec 5400] [-DryRun]
    perf-bench.ps1 -Job <job.json>          (the runner's form; same fields, see interface.md)
    GNU-style spellings (--set quick --out D:\x) are accepted too.

    Sets and boots (A = baseline, B = candidate; each boot is one tri invocation of every
    world file of the set, i.e. one game launch per island):
      quick        A, B (2 rounds per boot, 1 window per view): the screen.
      abba         A B B A, A B B A (-Cycles 2) over the 8 standard views + pin-0 rows.
      abba-scoped  the same 8 boots over -Views (the claimed views) + A, B over the other
                   standard views (the regression screen).
      full         B x -Boots (1): one build, 8 views + pin-0 rows.
      obs          one build: observer sampling on, off, off, on (env only).
      hitch        B x -Boots (1): H-steady, 1,024 frames per view.
      legacy       B x -Boots (2): round 1's corrected baseline at round 1's configuration
                   (fpsCap 60, vsync on), for the one-time reconciliation (3.1 PI-exit 2).
    Every boot: a fresh seeded user dir (graphics.cfg: Ultra, fpsCap 0, vsync 0, MSAA 4, scale
    1.0; legacy: fpsCap 60, vsync 1) that is also its program cache dir (tri forces
    POSEIDON_CACHE_DIR to the user dir; TRI_POSEIDON_CACHE_DIR is set for when P0-14 lands),
    POSEIDON_OBSERVER_SAMPLE=1 (or unset with -NoObserverSample; never set to 0, the engine
    tests presence only), TRI_GLOBAL_SLOTS=1, tri -j 1 --retries 0, the hidden-desktop wrapper,
    gpu_sampler.ps1 every 3 s (clock, P-state, foreign GPU users, Deadlock, vmmem, builds),
    then stop-owned-games.ps1 -DataDir <our junction>. Before every boot: 30 s of quiet
    (Deadlock pauses the job, re-checked every 2 min, up to -MaxPauseMin); after the last boot
    one more probe.
    Reading: -Read auto picks ring when every dist has triPerfRing and triGpuRing (a byte scan
    of PoseidonGame.exe), else meas (event=meas lines + triGpuTimes, the only reading on
    afdb4d60). Both sides always use the same reading; EMA and ring numbers are never mixed.

    Output in -Out: bench-plan.json (boots, dists, exe sha256 and verbs, env, config),
    tests\ (the generated files, frozen with the job), boots\<nn>-<side>\ (tri tree, user dir,
    gpu_samples.csv, tri.log, manifest.json), bench.json, bench.md, bench.summary.txt (one
    line), perf-bench.log. Exit: 0 done (verdicts are in bench.json), 2 bad arguments,
    3 contended (pause limit hit or too few clean windows), 4 a boot failed, 5 refused (mixed
    readings).
#>
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'gate-lib.ps1')

# ------------------------------------------------------------------ arguments (by hand: PS 5.1
# -File mode cannot bind GNU-style "--name", and the runner may use either spelling)
$o = @{ set = ''; out = ''; candidate = ''; baseline = ''; views = ''; rounds = 0; boots = 0; cycles = 2; env = @();
    candidateenv = @(); baselineenv = @(); read = 'auto'; junction = ''; datadir = ''; noobserversample = $false;
    quietsec = 30; maxpausemin = 120; timeoutsec = 5400; dryrun = $false; job = ''; candidatelabel = 'candidate';
    baselinelabel = 'baseline'; tri = ''; python = ''; expectdiffer = $null; jobfile = ''; outdir = ''; foreignlog = '';
    shaderoverride = ''; shaderoverridebaseline = ''; modes = ''; claim = 'pit'; scale2 = $false; strictcounts = $false;
    noise = ''; tier = 'T1' }
$switches = @('noobserversample', 'dryrun', 'scale2', 'strictcounts')
function Read-Args([object[]]$argv) {
    $i = 0
    while ($i -lt $argv.Count) {
        $a = [string]$argv[$i]
        if ($a -notmatch '^--?([A-Za-z0-9-]+)$') { Write-Host "perf-bench: unexpected argument '$a'"; exit 2 }
        $n = $Matches[1].ToLowerInvariant().Replace('-', '')
        if (-not $script:o.ContainsKey($n)) { Write-Host "perf-bench: unknown option '$a'"; exit 2 }
        if ($script:switches -contains $n) { $script:o[$n] = $true; $i++; continue }
        if ($i + 1 -ge $argv.Count) { Write-Host "perf-bench: $a needs a value"; exit 2 }
        $v = $argv[$i + 1]
        if ($script:o[$n] -is [array]) { $script:o[$n] = @($script:o[$n]) + @(([string]$v) -split ',' | Where-Object { $_ }) } else { $script:o[$n] = $v }
        $i += 2
    }
}
Read-Args @($args)
# The squad runner's spelling (RESUME.md section 5): -JobFile {job} -OutDir {out} -ForeignLog {foreign} -Tri {tri}
if ($o.jobfile) { $o.job = $o.jobfile }
if ($o.outdir) { $o.out = $o.outdir }
if (-not $o.foreignlog -and $env:SQUAD_FOREIGN_LOG) { $o.foreignlog = $env:SQUAD_FOREIGN_LOG }
if (-not $o.tri -and $env:SQUAD_TRI) { $o.tri = $env:SQUAD_TRI }
if (-not $o.datadir -and $env:SQUAD_JUNCTION) { $o.datadir = $env:SQUAD_JUNCTION }
$jobRounds = 0
$script:jobCommand = ''
$script:capScript = ''
if ($o.job) {
    $j = Get-Content -Raw -LiteralPath $o.job | ConvertFrom-Json
    $map = @{ set = 'set'; views = 'views'; boots = 'boots'; cycles = 'cycles'; read = 'read'; data_dir = 'datadir';
        quiet_sec = 'quietsec'; max_pause_min = 'maxpausemin'; timeout_sec = 'timeoutsec'; dry_run = 'dryrun'; expect_differ = 'expectdiffer' }
    foreach ($k in $map.Keys) { if ($j.PSObject.Properties[$k] -and $null -ne $j.$k -and "$($j.$k)" -ne '') { if (-not $o[$map[$k]] -or $o[$map[$k]] -eq 'auto' -or $o[$map[$k]] -eq 2 -or $o[$map[$k]] -eq 0) { $o[$map[$k]] = $j.$k } } }
    if (-not $o.out -and $j.PSObject.Properties['out'] -and $j.out) { $o.out = if ([IO.Path]::IsPathRooted($j.out)) { $j.out } else { Join-Path $script:SquadRoot $j.out } }
    if ($j.PSObject.Properties['junction'] -and $j.junction -and -not $o.datadir) {
        if ([IO.Path]::IsPathRooted([string]$j.junction)) { $o.datadir = $j.junction } else { $o.junction = $j.junction }
    }
    if ($j.PSObject.Properties['rounds'] -and $j.rounds) { $jobRounds = [int]$j.rounds }
    if ($j.PSObject.Properties['claimed_views'] -and $j.claimed_views) { $o.views = $j.claimed_views }
    if ($j.PSObject.Properties['modes'] -and $j.modes) { $o.modes = (@($j.modes) -join ',') }
    if ($j.PSObject.Properties['tier'] -and $j.tier) { $o.tier = $j.tier }
    if ($j.PSObject.Properties['command'] -and $j.command) { $script:jobCommand = [string]$j.command }
    # `env` is the candidate side (RESUME.md 4); POSEIDON_OBSERVER_SAMPLE and TRI_GLOBAL_SLOTS in it apply
    # to both sides (an observer on/off difference between A and B would be measured as the item's).
    if ($j.PSObject.Properties['env'] -and $j.env) { $o.candidateenv = $j.env }
    if ($j.PSObject.Properties['env_baseline'] -and $j.env_baseline) { $o.baselineenv = $j.env_baseline }
    if ($j.PSObject.Properties['shader_override_dir'] -and $j.shader_override_dir) { $o.shaderoverride = $j.shader_override_dir }
    if ($j.PSObject.Properties['shader_override_dir_baseline'] -and $j.shader_override_dir_baseline) { $o.shaderoverridebaseline = $j.shader_override_dir_baseline }
    foreach ($side in 'candidate', 'baseline') {
        if ($j.PSObject.Properties[$side] -and $j.$side) {
            $s = $j.$side
            if ($s -is [string]) { $o[$side] = $s } else {
                if ($s.dist) { $o[$side] = $s.dist }
                if ($s.label) { $o["${side}label"] = $s.label }
            }
        }
    }
    if ($j.PSObject.Properties['bench_args'] -and $j.bench_args) { Read-Args @($j.bench_args) }
}
# `rounds` (job field or -Rounds): ABBA cycles for abba sets, boots for legacy, in-file rounds otherwise.
if (-not $jobRounds -and $o.rounds) { $jobRounds = [int]$o.rounds }
$o.rounds = 0
if ($jobRounds) {
    switch ($o.set) {
        { $_ -in 'abba', 'abba-scoped' } { $o.cycles = $jobRounds }
        'legacy' { $o.boots = $jobRounds }
        default { $o.rounds = $jobRounds }
    }
}if ($o.views -is [array]) { $o.views = ($o.views -join ',') }
# ------------------------------------------------------------------ sets
$sets = @('quick', 'abba', 'abba-scoped', 'full', 'obs', 'hitch', 'legacy', 'stills', 'counts', 'smoke', 'motion')
if (-not ($sets -contains $o.set)) { Write-Host "perf-bench: -Set must be one of $($sets -join '|')"; exit 2 }
if (-not $o.out) { Write-Host 'perf-bench: -Out <caller folder> is required'; exit 2 }
if (-not ([IO.Path]::IsPathRooted($o.out))) { $o.out = Join-Path (Get-Location) $o.out }
New-Item -ItemType Directory -Force -Path $o.out | Out-Null
$log = Join-Path $o.out 'perf-bench.log'
function Stop-Unsupported([string]$why) {
    # The job asked for something this bench cannot do yet: say so and fail; never substitute.
    Write-GateLog $log "NOT RUN: $why"
    Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') "NOT RUN: $why`n"
    exit 6
}
if (-not $o.candidate) { Write-Host 'perf-bench: -Candidate <dist> is required'; exit 2 }
$viewList = @(([string]$o.views) -split ',' | Where-Object { $_ })
$mode = $o.set   # what actually runs
# motion (P0-10) needs triRecord in every dist: checked after the verb scan below
if ($o.set -eq 'hitch') {
    if ($viewList -contains 'boot-cache') {
        if ($viewList.Count -gt 1) { Stop-Unsupported 'boot-cache cannot be mixed with other hitch views in one job' }
        $mode = 'bootcache'
    } elseif ($viewList | Where-Object { $_ -match '^H-' -and $_ -ne 'H-steady' }) {
        Stop-Unsupported "hitch scenario $($viewList -join ',') is not in the bench yet (P0-13 scenario generators for H-start, H-stream, H-pan, H-dusk, H-rain, H-combat, H-cold, H-life have not landed); only H-steady (no views or view H-steady) and boot-cache run"
    } elseif ($viewList -contains 'H-steady') { $viewList = @(); $o.views = '' }
}
$needBaseline = @('quick', 'abba', 'abba-scoped') -contains $o.set
if ($needBaseline -and -not $o.baseline) { Write-Host "perf-bench: -Set $($o.set) needs -Baseline <dist>"; exit 2 }
$twoSided = [bool]$o.baseline -and @('quick', 'abba', 'abba-scoped', 'legacy', 'stills', 'counts', 'full', 'hitch', 'motion') -contains $o.set
if ($o.set -eq 'abba-scoped' -and -not $o.views) { Write-Host 'perf-bench: abba-scoped needs -Views (the claimed views)'; exit 2 }
$timed = @('quick', 'abba', 'abba-scoped', 'full', 'obs', 'hitch', 'legacy', 'motion') -contains $o.set
$tools = Resolve-GateTools -Tri $o.tri -Python $o.python
Write-GateLog $log "perf-bench set=$($o.set) run=$mode timed=$timed two-sided=$twoSided out=$($o.out) candidate=$($o.candidate) baseline=$($o.baseline) dry=$($o.dryrun)"
foreach ($k in 'tri', 'hidden', 'stop', 'sampler') {
    if (-not (Test-Path -LiteralPath $tools[$k])) { Write-GateLog $log "missing tool $k at $($tools[$k])"; if (-not $o.dryrun) { exit 2 } }
}

# ------------------------------------------------------------------ dists and reading
$dists = [ordered]@{ B = $o.candidate }
if ($twoSided) { $dists['A'] = $o.baseline }
$info = [ordered]@{}
foreach ($s in $dists.Keys) {
    try { $info[$s] = Get-ExeInfo -Dist $dists[$s] } catch { Write-GateLog $log "$_"; exit 2 }
}
# Both sides run the SAME generated tests, so they may use only verbs every dist has (INFRA-C1's
# candidate capture rig calls triBuildId, which the tip lacks: the bench's own tests never do).
$has = @{}
foreach ($v in 'triPerfRing', 'triGpuRing', 'triSetFpsCap', 'triResetTemporal', 'triSeedRandom', 'triDeterministic', 'triRecord') {
    $has[$v] = $true; foreach ($s in $info.Keys) { if (-not $info[$s].verbs[$v]) { $has[$v] = $false } }
}
$read = $o.read
if ($read -eq 'auto') { $read = if ($has.triPerfRing -and $has.triGpuRing) { 'ring' } else { 'meas' } }
if ($read -eq 'ring' -and -not ($has.triPerfRing -and $has.triGpuRing)) { Write-GateLog $log 'ring reading asked but a dist lacks triPerfRing/triGpuRing'; exit 2 }
$config = if ($o.set -eq 'legacy') { 'legacy' } else { 'fixed' }
if ($twoSided -and $null -ne $o.expectdiffer -and [bool]$o.expectdiffer -and $info['A'].sha256 -eq $info['B'].sha256) {
    Write-GateLog $log 'refused: the job says the builds differ but both exes have the same sha256'; exit 2
}
Write-GateLog $log "read=$read config=$config verbs(all dists): $(($has.Keys | Sort-Object | ForEach-Object { "$_=$($has[$_])" }) -join ' ')"
if ($mode -eq 'motion' -and -not $has.triRecord) { Stop-Unsupported 'motion clips need the P0-10 recorder (triRecord) in every dist of the job; build them with items\P0-10\patch.diff' }

# ------------------------------------------------------------------ data dir
try { $dataDir = Resolve-DataDir -Junction $o.junction -DataDir $o.datadir -Tools $tools -NoCreate:([bool]$o.dryrun) }
catch { Write-GateLog $log "$_"; exit 2 }

# ------------------------------------------------------------------ tests
function Invoke-Gen([string[]]$ga) {
    $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $tools.python @ga 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath (Join-Path $o.out 'gen.log') -Append -Encoding utf8
    $rc = $LASTEXITCODE; $ErrorActionPreference = $eap
    if ($rc -ne 0) { Write-GateLog $log "test generator failed ($rc), see gen.log"; exit 2 }
}
function New-SetTests([string]$Dir, [string[]]$Extra) {
    $ga = @('-I', (Join-Path $PSScriptRoot 'gen_perfgate.py'), '--set', $mode, '--out', $Dir, '--read', $read) + $Extra
    if ($o.rounds) { $ga += @('--rounds', "$($o.rounds)") }
    if ($o.modes) { $ga += @('--modes', $o.modes) }
    if ($o.scale2) { $ga += '--scale2' }
    if ($has.triSetFpsCap) { $ga += '--has-fpscap' }
    Invoke-Gen $ga
    $pl = Get-ChildItem -LiteralPath $Dir -Filter '*.plan.json' | Select-Object -First 1
    return @((Get-Content -Raw -LiteralPath $pl.FullName | ConvertFrom-Json).files)
}
$testsDir = Join-Path $o.out 'tests'
$restTests = @(); $gp = $null
switch ($mode) {
    'stills' {
        if ($script:jobCommand) {
            # A job-provided capture command (P0-6's own determinism tests): only a run-captures.ps1
            # call is accepted; the bench forces -OutDir to the boot folder and -Tri to the frozen tri.
            $cmdText = ($script:jobCommand -split '\s{3,}')[0].Trim()
            $tok = @($cmdText -split '\s+' | Where-Object { $_ })
            $fi = [Array]::IndexOf($tok, '-File')
            if ($tok[0] -notmatch '^powershell(\.exe)?$' -or $fi -lt 0 -or $tok[$fi + 1] -notmatch 'run-captures\.ps1$') {
                Stop-Unsupported "job command is not a run-captures.ps1 call: $cmdText"
            }
            $script:capScript = $tok[$fi + 1]
            $script:capArgs = @($tok | Select-Object -Skip ($fi + 2))
            Write-GateLog $log "stills via the job's capture command: $cmdText"
            $mainTests = @()
            break
        }
        $ga = @('-I', (Join-Path $PSScriptRoot 'gen_golden.py'), '--tier', $o.tier, '--out', $testsDir)
        if ($o.views) { $ga += @('--views', $o.views) }
        if ($o.modes) { $ga += @('--modes', $o.modes) }
        if ($has.triResetTemporal) { $ga += '--has-reset' }
        if ($has.triSeedRandom) { $ga += '--has-seed' }
        # with triDeterministic the sim clock must step at 1/60 too, or the sea, clouds and rain phase
        # follow real frame time (P0-6)
        if ($has.triDeterministic) { $ga += @('--has-deterministic', '--fixed-step') }
        Invoke-Gen $ga
        $gp = Get-Content -Raw -LiteralPath (Get-ChildItem -LiteralPath $testsDir -Filter '*.plan.json' | Select-Object -First 1).FullName | ConvertFrom-Json
        $mainTests = @($gp.files)
    }
    'motion' {
        # P0-10: one camera path per clip (views = clip names, gen_motion.CLIPS), recorded by triRecord in every
        # mode; two builds default to classic + remastered (the owner's three-way uses the baseline's Classic row)
        $ga = @('-I', (Join-Path $PSScriptRoot 'gen_motion.py'), '--out', $testsDir)
        if ($o.views) { $ga += @('--clips', $o.views) }
        $ga += @('--modes', $(if ($o.modes) { $o.modes } elseif ($twoSided) { 'classic,remastered' } else { 'remastered' }))
        if ($has.triResetTemporal) { $ga += '--has-reset' }
        if ($has.triSeedRandom) { $ga += '--has-seed' }
        if ($has.triDeterministic) { $ga += '--has-deterministic' }
        Invoke-Gen $ga
        $gp = Get-Content -Raw -LiteralPath (Join-Path $testsDir 'motion.plan.json') | ConvertFrom-Json
        $mainTests = @($gp.files)
    }
    'smoke' {
        $mainTests = @('smoke_abel', 'smoke_eden', 'smoke_sinai_lobo') | ForEach-Object { Join-Path $script:CapturesDir "$_.test.sqf" }
    }
    default {
        $extra = @(); if ($o.views -and $mode -ne 'bootcache') { $extra += @('--views', $o.views) }
        $mainTests = New-SetTests $testsDir $extra
        if ($mode -eq 'abba-scoped') { $restTests = New-SetTests (Join-Path $o.out 'tests-rest') @('--views', $o.views, '--rest') }
        $gp = Get-Content -Raw -LiteralPath (Get-ChildItem -LiteralPath $testsDir -Filter '*.plan.json' | Select-Object -First 1).FullName | ConvertFrom-Json
    }
}

# ------------------------------------------------------------------ env per side
$common = ConvertTo-EnvMap $o.env
$candEnv = ConvertTo-EnvMap $o.candidateenv
$baseEnv = ConvertTo-EnvMap $o.baselineenv
foreach ($k in @($candEnv.Keys)) { if ($k -in 'POSEIDON_OBSERVER_SAMPLE', 'TRI_GLOBAL_SLOTS') { $common[$k] = $candEnv[$k]; $candEnv.Remove($k) } }
foreach ($k in @($baseEnv.Keys)) { if ($k -in 'POSEIDON_OBSERVER_SAMPLE', 'TRI_GLOBAL_SLOTS') { $baseEnv.Remove($k) } }
if ($o.shaderoverride) { $candEnv['CWR_SHADER_OVERRIDE_DIR'] = $o.shaderoverride }
if ($o.shaderoverridebaseline) { $baseEnv['CWR_SHADER_OVERRIDE_DIR'] = $o.shaderoverridebaseline }
$obsOn = -not $o.noobserversample
if ($common.ContainsKey('POSEIDON_OBSERVER_SAMPLE')) {
    $ov = $common['POSEIDON_OBSERVER_SAMPLE']; $common.Remove('POSEIDON_OBSERVER_SAMPLE')
    if ($null -eq $ov -or "$ov" -eq '') { $obsOn = $false } elseif ("$ov" -eq '0') { Write-GateLog $log 'refused: POSEIDON_OBSERVER_SAMPLE=0 still turns sampling on (presence is tested); use null to unset'; exit 2 }
}
function Side-Env([string]$side, [bool]$obs) {
    $e = @{}
    foreach ($k in $common.Keys) { $e[$k] = $common[$k] }
    $mine = if ($side -eq 'A') { $baseEnv } else { $candEnv }
    $other = if ($side -eq 'A') { $candEnv } else { $baseEnv }
    # the runner applied the candidate env to this process: unset what the other side set and this side does not
    foreach ($k in $other.Keys) { if (-not $mine.ContainsKey($k)) { $e[$k] = $null } }
    foreach ($k in $mine.Keys) { $e[$k] = $mine[$k] }
    $e['POSEIDON_OBSERVER_SAMPLE'] = if ($obs) { '1' } else { $null }   # unset, never 0
    return $e
}

# ------------------------------------------------------------------ boot plan
$boots = @()
function Add-Boot([string]$side, [string[]]$tests, [hashtable]$envMap, [string]$part = 'main', [string]$role = '') {
    $script:boots += [ordered]@{ idx = $script:boots.Count; side = $side; dist = $dists[$side]; tests = $tests; env = $envMap; part = $part; role = $role }
}
$sidesAB = if ($twoSided) { @('A', 'B') } else { @('B') }
switch ($mode) {
    'quick' { foreach ($s in 'A', 'B') { Add-Boot $s $mainTests (Side-Env $s $obsOn) } }
    { $_ -in 'abba', 'abba-scoped' } {
        for ($c = 0; $c -lt [int]$o.cycles; $c++) { foreach ($s in 'A', 'B', 'B', 'A') { Add-Boot $s $mainTests (Side-Env $s $obsOn) } }
        if ($mode -eq 'abba-scoped' -and $restTests.Count) { foreach ($s in 'A', 'B') { Add-Boot $s $restTests (Side-Env $s $obsOn) 'rest' } }
    }
    'obs' { foreach ($on in $true, $false, $false, $true) { $s = if ($on) { 'A' } else { 'B' }; $script:boots += [ordered]@{ idx = $script:boots.Count; side = $s; dist = $o.candidate; tests = $mainTests; env = (Side-Env 'B' $on); part = 'main'; role = '' } } }
    'bootcache' { foreach ($s in $sidesAB) { Add-Boot $s $mainTests (Side-Env $s $obsOn) 'main' 'cold'; Add-Boot $s $mainTests (Side-Env $s $obsOn) 'main' 'warm' } }
    'legacy' {
        $n = if ($o.boots) { [int]$o.boots } else { 2 }
        if ($twoSided) { $pat = @('A', 'B', 'B', 'A'); for ($k = 0; $k -lt 2 * $n; $k++) { $s = $pat[$k % 4]; Add-Boot $s $mainTests (Side-Env $s $obsOn) } }
        else { for ($k = 0; $k -lt $n; $k++) { Add-Boot 'B' $mainTests (Side-Env 'B' $obsOn) } }
    }
    default {
        $n = if ($o.boots) { [int]$o.boots } else { 1 }
        for ($k = 0; $k -lt $n; $k++) { foreach ($s in $sidesAB) { Add-Boot $s $mainTests (Side-Env $s $obsOn) } }
    }
}
$sideInfo = [ordered]@{}
if ($mode -eq 'obs') {
    $sideInfo['A'] = [ordered]@{ label = 'observer sampled (POSEIDON_OBSERVER_SAMPLE=1)'; dist = $o.candidate; build_id = $info['B'].build_id }
    $sideInfo['B'] = [ordered]@{ label = 'observer every frame (unset)'; dist = $o.candidate; build_id = $info['B'].build_id }
} else {
    if ($twoSided) { $sideInfo['A'] = [ordered]@{ label = $o.baselinelabel; dist = $o.baseline; build_id = $info['A'].build_id } }
    $sideInfo['B'] = [ordered]@{ label = $o.candidatelabel; dist = $o.candidate; build_id = $info['B'].build_id }
}
$expected = @{}
if ($gp -and $gp.PSObject.Properties['windows']) {
    $perBoot = [int]$gp.rounds * [int]$gp.windows
    foreach ($b in $boots) { if ($b.part -eq 'main') { $expected[$b.side] = [int]$expected[$b.side] + $perBoot } }
}

function Invoke-CaptureCommandBoot([string]$BootDir, [hashtable]$EnvMap) {
    New-Item -ItemType Directory -Force -Path $BootDir | Out-Null
    $user = New-SeededUserDir -Dir (Join-Path $BootDir 'user') -Config $config
    $a = @()
    for ($i = 0; $i -lt $script:capArgs.Count; $i++) {
        $x = $script:capArgs[$i] -replace '<T>', $script:SquadRoot -replace '<out>', $o.out
        if ($x -in '-OutDir', '-Tri' -and $i + 1 -lt $script:capArgs.Count) { $i++; continue }
        $a += $x
    }
    $a += @('-OutDir', $BootDir, '-Tri', $tools.tri)
    $e = @{}; foreach ($k in $EnvMap.Keys) { $e[$k] = $EnvMap[$k] }
    $e['TRI_GLOBAL_SLOTS'] = '1'; $e['POSEIDON_USER_DIR'] = $user.dir
    $rec = [ordered]@{ boot_dir = $BootDir; dist = $o.candidate; data_dir = $dataDir; config = $config; user_dir = $user.dir;
        cmd = "$($script:capScript) $($a -join ' ')"; env = $e; start = (Get-Date).ToString('o'); exit = $null; dry_run = [bool]$o.dryrun }
    if ($o.dryrun) { Write-GateLog $log "DRY RUN capture command: $($rec.cmd)"; $rec.exit = 0; return $rec }
    $prev = Set-BootEnv $e
    try {
        $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
        & powershell -NoProfile -ExecutionPolicy Bypass -File $script:capScript @a 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath (Join-Path $BootDir 'capture.log') -Encoding utf8
        $rec.exit = $LASTEXITCODE; $ErrorActionPreference = $eap
    } finally { Restore-BootEnv $prev; $rec.end = (Get-Date).ToString('o') }
    Write-GateLog $log "capture command boot exit $($rec.exit): $BootDir"
    return $rec
}

# ------------------------------------------------------------------ run
$plan = [ordered]@{ set = $(if ($mode -eq 'bootcache') { 'bootcache' } else { $o.set }); job_set = $o.set; read = $read; config = $config;
    timed = $timed; out = $o.out; views = $o.views; modes = $o.modes; data_dir = $dataDir; sides = $sideInfo; exe = $info; tools = $tools;
    foreign_runner = $o.foreignlog; expected_windows = $expected; generator = $gp; dry_run = [bool]$o.dryrun;
    started = (Get-Date).ToString('o'); boots = @() }
$contended = $false
$k = 0
$userDirs = @{}
foreach ($b in $boots) {
    $k++
    $bootDir = Join-Path $o.out ("boots\{0:D2}-{1}{2}{3}" -f $k, $b.side, $(if ($b.part -eq 'rest') { '-rest' } else { '' }), $(if ($b.role) { "-$($b.role)" } else { '' }))
    if ($timed -and -not $o.dryrun) {
        $q = Wait-Quiet -Log $log -QuietSec ([int]$o.quietsec) -MaxPauseMin ([int]$o.maxpausemin) -OwnDataDir $dataDir -Tri $tools.tri
        if (-not $q) { $contended = $true; break }
    } else {
        $r = Get-QuietReasons -OwnDataDir $dataDir -Tri $tools.tri
        Write-GateLog $log "probe before boot ${k} ($(if ($timed) { 'dry run' } else { 'untimed: no quiet wait' })): $(if ($r.Count) { $r -join ' ' } else { 'quiet' })"
    }
    $shared = ''
    if ($b.role -eq 'warm') { $shared = $userDirs[$b.side] }
    if ($script:capScript) {
        $rec = Invoke-CaptureCommandBoot -BootDir $bootDir -EnvMap $b.env
    } else { try {
        $rec = Invoke-GateBoot -BootDir $bootDir -Dist $b.dist -Tests $b.tests -DataDir $dataDir -Tools $tools -Env $b.env `
            -Config $config -Log $log -TimeoutSec ([int]$o.timeoutsec) -DryRun:([bool]$o.dryrun) -UserDir $shared `
            -Retries $(if ($timed) { 0 } else { 1 })
    } catch { Write-GateLog $log "boot $k failed to start: $_"; $rec = [ordered]@{ boot_dir = $bootDir; exit = 99; error = "$_" } } }
    if ($b.role -eq 'cold') { $userDirs[$b.side] = $rec.user_dir }
    $rec['idx'] = $b.idx; $rec['side'] = $b.side; $rec['part'] = $b.part; $rec['role'] = $b.role
    $rec['exe'] = $info[$(if ($mode -eq 'obs') { 'B' } else { $b.side })]
    $rec['build_id'] = $rec['exe'].build_id
    Write-Utf8NoBom (Join-Path $bootDir 'manifest.json') ($rec | ConvertTo-Json -Depth 6)
    $plan.boots += $rec
}
if (-not $o.dryrun) {
    $final = Get-QuietReasons -OwnDataDir $dataDir -Tri $tools.tri
    $plan['final_probe'] = @($final)
    Write-GateLog $log "after the run: $(if ($final.Count) { $final -join ' ' } else { 'quiet' })"
}
$plan['contended_pause'] = $contended
$plan['finished'] = (Get-Date).ToString('o')
$planPath = Join-Path $o.out 'bench-plan.json'
Write-Utf8NoBom $planPath ($plan | ConvertTo-Json -Depth 8)
$motionPost = $null
if ($mode -eq 'motion') {
    # The image half (exact A/B diff, FFV1 encode, motion.py screen, the owner's three-way clip) is heavy work and
    # this job holds the machine lock, so it goes to squad-heavy -Kind encode, detached: the wrapper starts it once
    # the runner has let go of the lock (motion-post.ps1 writes motion.summary.txt).
    $heavy = Join-Path $script:SquadRoot 'bin\squad-heavy.ps1'
    $jid = if ($env:SQUAD_JOB_FILE) { (Split-Path -Leaf $env:SQUAD_JOB_FILE) -replace '\.json$', '' } else { 'perf-bench' }
    $motionPost = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $heavy, '-Kind', 'encode', '-Id', $jid, '-Agent', 'perf-bench', '--',
        'powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $PSScriptRoot 'motion-post.ps1'), '-Out', $o.out, '-Python', $tools.python)
    if ($o.dryrun) { Write-GateLog $log "DRY RUN motion post (detached): powershell $($motionPost -join ' ')" }
}
if ($o.dryrun) { Write-GateLog $log "DRY RUN done: $planPath"; exit 0 }

# ------------------------------------------------------------------ results
$ErrorActionPreference = 'Continue'
if ($mode -eq 'motion') {
    $failedBoots = @($plan.boots | Where-Object { $_.exit -ne 0 })
    if ($o.job) { $env:SQUAD_ITEM_ID = [string]$j.id; if ($j.PSObject.Properties['owner_says'] -and $j.owner_says) { $env:SQUAD_MOTION_SAYS = [string]$j.owner_says } }
    $pp = Start-Process powershell -ArgumentList $motionPost -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $o.out 'motion-post.out.log') -RedirectStandardError (Join-Path $o.out 'motion-post.err.log')
    Write-GateLog $log "motion post started detached (squad-heavy encode, PID $($pp.Id)); results in motion.summary.txt once the machine lock is free"
    Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') "set=motion clips=$($o.views) boots exit $(($plan.boots | ForEach-Object { $_.exit }) -join ','); post-processing pending (squad-heavy encode PID $($pp.Id), see motion.summary.txt)`n"
    $rc = if ($failedBoots.Count) { 4 } elseif ($contended) { 3 } else { 0 }
    Write-GateLog $log "perf-bench exit $rc"
    exit $rc
}
& $tools.python -I (Join-Path $PSScriptRoot 'bench_report.py') --plan $planPath --out (Join-Path $o.out 'bench') 2>&1 | ForEach-Object { "$_" } | Tee-Object -FilePath $log -Append
$rc = $LASTEXITCODE
$failedBoots = @($plan.boots | Where-Object { $_.exit -ne 0 })
$recDumps = @()
if ($mode -eq 'stills' -and -not $failedBoots.Count) { $recDumps = @($plan.boots | ForEach-Object { Get-ChildItem -LiteralPath $_.boot_dir -Recurse -Filter '*.raw.txt' -File -ErrorAction SilentlyContinue }) }
if ($recDumps.Count) {
    # P0-10: triRecord dumps beside triScreenshot shots: the bit-identity check (untimed, so inline through squad-heavy)
    $heavy = Join-Path $script:SquadRoot 'bin\squad-heavy.ps1'
    $lines = @()
    foreach ($bb in $plan.boots) {
        $tag = Split-Path -Leaf $bb.boot_dir
        $ca = @('-I', (Join-Path $PSScriptRoot 'recorder.py'), 'check', $bb.boot_dir, '--json', (Join-Path $o.out "recorder-check-$tag.json"))
        & powershell -NoProfile -ExecutionPolicy Bypass -File $heavy -Kind analysis -Id perf-bench -Agent perf-bench -- $tools.python @ca 2>&1 |
            ForEach-Object { "$_" } | Tee-Object -FilePath (Join-Path $o.out "recorder-check-$tag.log")
        $lines += "${tag}: " + (Get-Content (Join-Path $o.out "recorder-check-$tag.log") | Where-Object { $_ -match '^recorder-check: ' } | Select-Object -Last 1)
    }
    Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') ("set=stills recorder check (P0-10): " + ($lines -join ' | ') + "`n")
    $rc = 0
} elseif ($mode -eq 'stills') {
    # Image work beside an untimed job is allowed (2.5 item 3) and goes through the heavy-work door.
    if ($twoSided -and -not $failedBoots.Count) {
        $aDir = ($plan.boots | Where-Object { $_.side -eq 'A' } | Select-Object -First 1).boot_dir
        $bDir = ($plan.boots | Where-Object { $_.side -eq 'B' } | Select-Object -First 1).boot_dir
        $noise = if ($o.noise) { $o.noise } else { Join-Path $PSScriptRoot 'noise\m2-scratch-afdb4d6-p5-reset.json' }
        $va = @('-I', (Join-Path $PSScriptRoot 'vgate.py'), 'compare', $aDir, $bDir, '--claim', $o.claim, '--noise', $noise,
            '--out', (Join-Path $o.out 'vgate'), '--seed', '1', '--expect-differ')
        if ($o.strictcounts) { $va += '--strict-counts' }
        $heavy = Join-Path $script:SquadRoot 'bin\squad-heavy.ps1'
        & powershell -NoProfile -ExecutionPolicy Bypass -File $heavy -Kind analysis -Id ($(if ($env:SQUAD_JOB_FILE) { Split-Path -Leaf $env:SQUAD_JOB_FILE } else { 'perf-bench' }) -replace '\.json$', '') -Agent perf-bench -- $tools.python @va 2>&1 |
            ForEach-Object { "$_" } | Tee-Object -FilePath (Join-Path $o.out 'vgate.log')
        $vline = (Get-Content (Join-Path $o.out 'vgate.log') | Where-Object { $_ -match '^vgate: ' } | Select-Object -Last 1)
        Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') "set=stills claim=$($o.claim) noise=$(Split-Path -Leaf $noise): $vline (report: vgate\report.md)`n"
        $rc = 0
    } elseif (-not $twoSided -and $plan.boots.Count -ge 2 -and -not $failedBoots.Count) {
        # one build, several boots: the determinism check (boot 1 against boot 2, exact), and inside each
        # boot any <view>__det1 against <view>__det2 pair (the session floor; no such pair, no shots)
        $heavy = Join-Path $script:SquadRoot 'bin\squad-heavy.ps1'
        $runs = @(, @('boot1-vs-boot2', $plan.boots[0].boot_dir, $plan.boots[1].boot_dir, @()))
        foreach ($bb in $plan.boots) { $runs += , @("det1-vs-det2-$(Split-Path -Leaf $bb.boot_dir)", $bb.boot_dir, $bb.boot_dir, @('--label-a', 'det1', '--label-b', 'det2')) }
        $lines = @()
        foreach ($r in $runs) {
            $vl = Join-Path $o.out "vgate-$($r[0]).log"
            $va = @('-I', (Join-Path $PSScriptRoot 'vgate.py'), 'compare', $r[1], $r[2], '--claim', 'exact', '--out', (Join-Path $o.out "vgate-$($r[0])"), '--seed', '1', '--no-sheets') + $r[3]
            & powershell -NoProfile -ExecutionPolicy Bypass -File $heavy -Kind analysis -Id perf-bench -Agent perf-bench -- $tools.python @va 2>&1 |
                ForEach-Object { "$_" } | Tee-Object -FilePath $vl
            $lines += "$($r[0]): " + ((Get-Content $vl | Where-Object { $_ -match '^vgate: ' } | Select-Object -Last 1) -replace '; [A-Za-z]:\\.*$', '')
        }
        Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') ("set=stills one build x $($plan.boots.Count) boots, exact: " + ($lines -join ' | ') + "`n")
        $rc = 0
    } elseif (-not $twoSided) {
        Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') "set=stills one build, $($plan.boots.Count) boot(s) shot; compare with vgate.py (noise or compare)`n"
        $rc = 0
    }
}
if ($mode -eq 'smoke') {
    Write-Utf8NoBom (Join-Path $o.out 'bench.summary.txt') "set=smoke tri exit $(($plan.boots | ForEach-Object { $_.exit }) -join ',')`n"
    $rc = 0
}
if ($failedBoots.Count) { Write-GateLog $log "failed boots: $(($failedBoots | ForEach-Object { "$($_.idx)=$($_.exit)" }) -join ' ')"; $rc = 4 }
if ($contended -and $rc -eq 0) { $rc = 3 }
Write-GateLog $log "perf-bench exit $rc"
exit $rc
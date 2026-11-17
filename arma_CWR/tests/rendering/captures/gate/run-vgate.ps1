<#
.SYNOPSIS
    Shoots a vgate still set (PERF-PLAN P0-8): the T1 or T2 golden views of one dist, on -Boots
    fresh boots (3 for a noise floor), each boot in its own folder with manifest.json. An
    untimed `stills` job: only the squad queue runner calls it (only the runner launches games),
    between timed jobs, without the 30 s quiet wait (2.5 item 3).

.DESCRIPTION
    run-vgate.ps1 -Tier T1|T2 -Out <caller folder> -Dist <dist> [-Boots 1] [-Views v1,v2]
                  [-Env K=V,...] [-Junction <name> | -DataDir <path>] [-NoExtraTests] [-DryRun]
    run-vgate.ps1 -Job <job.json>     (fields: tier, out, candidate.dist or dist, boots, views,
                                       env, junction, data_dir, dry_run)
    GNU-style spellings (--tier T1) are accepted too.

    Per boot: a fresh seeded user dir (the fixed configuration of 3.3), POSEIDON_OBSERVER_SAMPLE=1,
    TRI_GLOBAL_SLOTS=1, tri -j 1 on the hidden desktop, gen_golden.py tests with the determinism
    verbs this dist has (byte scan: triResetTemporal, triSeedRandom, triDeterministic), and for
    T2 the existing cockpit, optics and combat tests (golden_views.EXTRA_TESTS) unless
    -NoExtraTests. Then:
      vgate.py noise boots\01 boots\02 boots\03 --build-id <id> --out noise.json   (a floor)
      vgate.py compare <golden set> <candidate set> --claim ...                   (a verdict)
    both through squad-heavy.ps1 -Kind analysis (image work), never inside this job.
    Exit: 0 shot, 2 bad arguments, 4 a boot failed.
#>
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'gate-lib.ps1')
$o = @{ tier = 'T1'; out = ''; dist = ''; boots = 1; views = ''; env = @(); junction = ''; datadir = ''; noextratests = $false;
    dryrun = $false; job = ''; timeoutsec = 5400; tri = ''; python = '' }
$switches = @('noextratests', 'dryrun')
$argv = @($args); $i = 0
while ($i -lt $argv.Count) {
    $a = [string]$argv[$i]
    if ($a -notmatch '^--?([A-Za-z0-9-]+)$') { Write-Host "run-vgate: unexpected argument '$a'"; exit 2 }
    $n = $Matches[1].ToLowerInvariant().Replace('-', '')
    if (-not $o.ContainsKey($n)) { Write-Host "run-vgate: unknown option '$a'"; exit 2 }
    if ($switches -contains $n) { $o[$n] = $true; $i++; continue }
    if ($i + 1 -ge $argv.Count) { Write-Host "run-vgate: $a needs a value"; exit 2 }
    $v = $argv[$i + 1]
    if ($o[$n] -is [array]) { $o[$n] = @($o[$n]) + @(([string]$v) -split ',' | Where-Object { $_ }) } else { $o[$n] = $v }
    $i += 2
}
if ($o.job) {
    $j = Get-Content -Raw -LiteralPath $o.job | ConvertFrom-Json
    foreach ($k in 'tier', 'out', 'boots', 'views', 'junction') { if ($j.PSObject.Properties[$k] -and $j.$k) { $o[$k] = $j.$k } }
    if ($j.PSObject.Properties['data_dir'] -and $j.data_dir) { $o.datadir = $j.data_dir }
    if ($j.PSObject.Properties['dry_run'] -and $j.dry_run) { $o.dryrun = $true }
    if ($j.PSObject.Properties['env'] -and $j.env) { $o.env = $j.env }
    if ($j.PSObject.Properties['dist'] -and $j.dist) { $o.dist = $j.dist }
    if ($j.PSObject.Properties['candidate'] -and $j.candidate) {
        if ($j.candidate -is [string]) { $o.dist = $j.candidate } else { $o.dist = $j.candidate.dist; if ($j.candidate.env) { $o.env = $j.candidate.env } }
    }
}
if ($o.views -is [array]) { $o.views = ($o.views -join ',') }
if (@('T1', 'T2') -notcontains $o.tier) { Write-Host 'run-vgate: -Tier T1|T2'; exit 2 }
if (-not $o.out -or -not $o.dist) { Write-Host 'run-vgate: -Out and -Dist are required'; exit 2 }
New-Item -ItemType Directory -Force -Path $o.out | Out-Null
$log = Join-Path $o.out 'run-vgate.log'
$tools = Resolve-GateTools -Tri $o.tri -Python $o.python
try { $info = Get-ExeInfo -Dist $o.dist } catch { Write-GateLog $log "$_"; exit 2 }
try { $dataDir = if ($o.dryrun -and -not $o.datadir) { Join-Path $script:EngineRoot "packages\$($o.junction)" } else { Resolve-DataDir -Junction $o.junction -DataDir $o.datadir -Tools $tools } }
catch { Write-GateLog $log "$_"; exit 2 }
Write-GateLog $log "run-vgate tier=$($o.tier) dist=$($o.dist) boots=$($o.boots) build=$($info.build_id) verbs reset=$($info.verbs.triResetTemporal) seed=$($info.verbs.triSeedRandom) det=$($info.verbs.triDeterministic)"

$testsDir = Join-Path $o.out 'tests'
$ga = @('-I', (Join-Path $PSScriptRoot 'gen_golden.py'), '--tier', $o.tier, '--out', $testsDir)
if ($o.views) { $ga += @('--views', $o.views) }
if ($info.verbs.triResetTemporal) { $ga += '--has-reset' }
if ($info.verbs.triSeedRandom) { $ga += '--has-seed' }
if ($info.verbs.triDeterministic) { $ga += @('--has-deterministic', '--fixed-step') }  # P0-6: step the sim clock at 1/60 too
$ErrorActionPreference = 'Continue'
& $tools.python @ga 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath (Join-Path $o.out 'gen.log') -Encoding utf8
$rc = $LASTEXITCODE
$ErrorActionPreference = 'Stop'
if ($rc -ne 0) { Write-GateLog $log "gen_golden failed ($rc)"; exit 2 }
$gp = Get-Content -Raw -LiteralPath (Join-Path $testsDir "vg_$($o.tier.ToLower()).plan.json") | ConvertFrom-Json
$tests = @($gp.files)
if ($o.tier -eq 'T2' -and -not $o.noextratests) {
    foreach ($p in $gp.extra_tests.PSObject.Properties) { $tests += (Join-Path $script:EngineRoot $p.Value) }
}
$envMap = ConvertTo-EnvMap $o.env
$envMap['POSEIDON_OBSERVER_SAMPLE'] = '1'
$failed = 0
$boots = @()
for ($k = 1; $k -le [int]$o.boots; $k++) {
    $bootDir = Join-Path $o.out ("boots\{0:D2}" -f $k)
    try {
        $rec = Invoke-GateBoot -BootDir $bootDir -Dist $o.dist -Tests $tests -DataDir $dataDir -Tools $tools -Env $envMap `
            -Config fixed -Log $log -TimeoutSec ([int]$o.timeoutsec) -DryRun:([bool]$o.dryrun) -Retries 1
    } catch { Write-GateLog $log "boot $k failed to start: $_"; $rec = [ordered]@{ boot_dir = $bootDir; exit = 99; error = "$_" } }
    $rec['exe'] = $info; $rec['build_id'] = $info.build_id; $rec['tier'] = $o.tier
    Write-Utf8NoBom (Join-Path $bootDir 'manifest.json') ($rec | ConvertTo-Json -Depth 6)
    if ($rec.exit -ne 0) { $failed++ }
    $boots += $rec
}
Write-Utf8NoBom (Join-Path $o.out 'stills-plan.json') (([ordered]@{ tier = $o.tier; dist = $o.dist; exe = $info; data_dir = $dataDir; boots = $boots; generator = $gp }) | ConvertTo-Json -Depth 8)
Write-GateLog $log "run-vgate done: $($boots.Count) boots, $failed failed. Next (image work, squad-heavy -Kind analysis): vgate.py noise / compare on $($o.out)\boots\NN"
if ($failed) { exit 4 }
exit 0

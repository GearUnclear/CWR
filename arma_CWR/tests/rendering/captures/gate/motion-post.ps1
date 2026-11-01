<#
.SYNOPSIS
    The image half of a `motion` bench job (PERF-PLAN P0-10, P0-12, 3.6): the exact per-frame
    compare of the two builds' recordings, the lossless FFV1 encode, the motion screener
    (motion.py screen) and the owner's three-way clip (owner_sheet.py).

.DESCRIPTION
    perf-bench.ps1 starts this through squad-heavy.ps1 -Kind encode, detached, after the boots of
    a motion job: the job holds the machine lock, so heavy work cannot run inside it, and the
    wrapper waits until the runner has released the lock. Never run it by hand while a job runs.

        motion-post.ps1 -Out <job out dir> [-Python python] [-KeepRaw]

    Reads <out>\bench-plan.json (boots, sides, the generator plan with the clip labels). Per clip
    label (<clip>__today = remastered, <clip>__classic, ...) and side:
      1. two builds: recorder.py diff A B on the dumps (exact, every frame) -> motion\diff-<label>.json
      2. recorder.py encode -> clips\<side>\<label>.mkv (FFV1, verified frame by frame, the dump
         deleted after a good verify unless -KeepRaw); no ffmpeg: dumps kept, nothing encoded
      3. motion.py screen --cand B today --ref A today [--vanilla A classic] -> motion\<clip>\
         (one build: motion.py metrics on its clips). Tripwires only, never approvals; motion.py
         labels the classic row "vanilla" in its own still products, which are for agents only.
      4. a three-way owner clip when A classic, A today and B today exist: owner\ (index.html,
         clips\<id>.mp4), original_label "Classic lighting on today's engine" (never "vanilla").
    Writes motion.summary.txt and appends its line to bench.summary.txt. Exit 0 done (verdicts
    inside), 1 something failed (see motion-post.log).
#>
param([Parameter(Mandatory)][string]$Out, [string]$Python = 'python', [switch]$KeepRaw)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'gate-lib.ps1')
$log = Join-Path $Out 'motion-post.log'
$planPath = Join-Path $Out 'bench-plan.json'
if (-not (Test-Path -LiteralPath $planPath)) { Write-GateLog $log "no bench-plan.json in $Out"; exit 1 }
$plan = Get-Content -Raw -LiteralPath $planPath | ConvertFrom-Json
$gen = $plan.generator
$rec = Join-Path $PSScriptRoot 'recorder.py'
$failed = 0
function Invoke-Py([string]$Tag, [string[]]$PyArgs) {
    $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    $lines = @(& $Python -I @PyArgs 2>&1 | ForEach-Object { "$_" })
    $rc = $LASTEXITCODE; $ErrorActionPreference = $eap
    foreach ($l in $lines) { Write-GateLog $log "  [$Tag] $l" }
    if ($rc -ne 0) { Write-GateLog $log "  [$Tag] exit $rc"; $script:failed++ }
    return @{ rc = $rc; lines = $lines }
}
$ffmpeg = $env:FFMPEG
if (-not $ffmpeg) { $c = Get-Command ffmpeg -ErrorAction SilentlyContinue; if ($c) { $ffmpeg = $c.Source } }
if (-not $ffmpeg -and (Test-Path -LiteralPath 'C:\ffmpeg\bin\ffmpeg.exe')) { $ffmpeg = 'C:\ffmpeg\bin\ffmpeg.exe' }
Write-GateLog $log "motion-post out=$Out ffmpeg=$(if ($ffmpeg) { $ffmpeg } else { 'none' })"

# ---- the dumps per side and label
$dumps = @{}
foreach ($b in @($plan.boots)) {
    if ($b.exit -ne 0) { Write-GateLog $log "boot $($b.idx) ($($b.side)) exit $($b.exit): its dumps are not used"; continue }
    foreach ($f in @(Get-ChildItem -LiteralPath $b.boot_dir -Recurse -Filter '*.raw' -File -ErrorAction SilentlyContinue)) {
        if (-not (Test-Path -LiteralPath "$($f.FullName).txt")) { continue }
        $dumps["$($b.side)|$($f.BaseName)"] = $f.FullName
    }
}
$labels = @()
foreach ($c in $gen.labels.PSObject.Properties) { foreach ($m in $c.Value.PSObject.Properties) { $labels += [ordered]@{ clip = $c.Name; mode = $m.Name; label = $m.Value } } }
$sides = @($plan.sides.PSObject.Properties.Name)
$summary = @()
foreach ($l in $labels) {
    $have = @($sides | Where-Object { $dumps.ContainsKey("$_|$($l.label)") })
    Write-GateLog $log "$($l.label): dumps on side(s) $($have -join ',')"
    if ($have.Count -lt $sides.Count) { $summary += "$($l.label): missing on $((@($sides | Where-Object { $have -notcontains $_ })) -join ',')"; $failed++ }
}

# ---- 1. exact per-frame compare A vs B (on the dumps, before they are encoded and deleted)
New-Item -ItemType Directory -Force -Path (Join-Path $Out 'motion') | Out-Null
if ($sides -contains 'A') {
    foreach ($l in $labels) {
        $a = $dumps["A|$($l.label)"]; $b = $dumps["B|$($l.label)"]
        if (-not $a -or -not $b) { continue }
        $r = Invoke-Py "diff $($l.label)" @($rec, 'diff', $a, $b, '--json', (Join-Path $Out "motion\diff-$($l.label).json"))
        $line = ($r.lines | Where-Object { $_ -match '^recorder-diff: ' } | Select-Object -Last 1) -replace '^recorder-diff: ', ''
        $summary += "$($l.label) A vs B: $line"
    }
}

# ---- 2. lossless encode
$mkv = @{}
if ($ffmpeg) {
    $env:FFMPEG = $ffmpeg
    foreach ($k in @($dumps.Keys)) {
        $side, $label = $k -split '\|', 2
        $dst = Join-Path $Out "clips\$side\$label.mkv"
        $ea = @($rec, 'encode', $dumps[$k], '--out', $dst)
        if ($KeepRaw) { $ea += '--keep-raw' }
        $r = Invoke-Py "encode $side $label" $ea
        if ($r.rc -eq 0) { $mkv[$k] = $dst }
    }
} else {
    $summary += 'no ffmpeg on this machine: dumps kept, not encoded, no screen and no owner clip'
}

# ---- 3. the motion screener, 4. the owner clip
$mp = Join-Path $PSScriptRoot 'motion.py'
$clips = @($labels | ForEach-Object { $_.clip } | Select-Object -Unique)
$sheetItems = @()
foreach ($c in $clips) {
    $today = ($labels | Where-Object { $_.clip -eq $c -and $_.mode -eq 'remastered' } | Select-Object -First 1).label
    $classic = ($labels | Where-Object { $_.clip -eq $c -and $_.mode -eq 'classic' } | Select-Object -First 1).label
    $mo = Join-Path $Out "motion\$c"
    if ($today -and $mkv["A|$today"] -and $mkv["B|$today"]) {
        $sa = @($mp, 'screen', '--cand', $mkv["B|$today"], '--ref', $mkv["A|$today"], '--out', $mo)
        if ($classic -and $mkv["A|$classic"]) { $sa += @('--vanilla', $mkv["A|$classic"]) }
        $r = Invoke-Py "screen $c" $sa
        $v = ($r.lines | Where-Object { $_ -match '^verdict: ' } | Select-Object -Last 1)
        $summary += "$c screen: $v"
        if ($classic -and $mkv["A|$classic"]) {
            $n = [int]$gen.clips.$c.frames
            $sheetItems += [ordered]@{ id = "$($env:SQUAD_ITEM_ID)$(if ($env:SQUAD_ITEM_ID) { '_' } else { '' })$c"
                says = $(if ($env:SQUAD_MOTION_SAYS) { $env:SQUAD_MOTION_SAYS } else { 'The same camera move in each version, at real speed.' })
                question = 'Does the right-hand picture move the same way as the middle one? If not, what looks wrong?'
                clip = [ordered]@{ original = $mkv["A|$classic"]; today = $mkv["A|$today"]; new = $mkv["B|$today"]; from = 0; frames = $n
                    original_label = "Classic lighting on today's engine" } }
        }
    } else {
        $mine = @($mkv.Keys | Where-Object { $_ -like "B|$c*" } | ForEach-Object { $mkv[$_] })
        if ($mine.Count) {
            $r = Invoke-Py "metrics $c" (@($mp, 'metrics') + $mine + @('--json', (Join-Path $Out "motion\$c-metrics.json")))
            $summary += "$c metrics: one build, $($mine.Count) clip(s), see motion\$c-metrics.json"
        }
    }
}
if ($sheetItems.Count) {
    $spec = Join-Path $Out 'owner\sheet-spec.json'
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $spec) | Out-Null
    Write-Utf8NoBom $spec (([ordered]@{ title = 'Changes to look at'; items = $sheetItems }) | ConvertTo-Json -Depth 6)
    $r = Invoke-Py 'owner sheet' @((Join-Path $PSScriptRoot 'owner_sheet.py'), $spec, '--out', (Join-Path $Out 'owner'))
    $summary += "owner three-way: $(if ($r.rc -eq 0) { 'owner\index.html' } else { 'FAILED' })"
}
$line = "motion-post: $(if ($failed) { "$failed step(s) failed" } else { 'done' }); " + ($summary -join ' | ')
Write-Utf8NoBom (Join-Path $Out 'motion.summary.txt') "$line`n"
try { [IO.File]::AppendAllText((Join-Path $Out 'bench.summary.txt'), "$line`n") } catch { }
Write-GateLog $log $line
exit $(if ($failed) { 1 } else { 0 })

<#
.SYNOPSIS
    Samples the GPU clock, load and power, and every running PoseidonGame, while
    a timing run goes (plan section 8 item 7; run-timing.ps1 and
    gate\perf-bench.ps1 start it).

.DESCRIPTION
    Writes one CSV row every -Every seconds until -Seconds have passed or the
    file <Out>.stop appears: nvidia-smi's graphics utilisation, memory
    utilisation, graphics and memory clocks, power and P-state, then
    "<all PoseidonGame>/foreign<n>" where foreign counts the games whose command
    line does not carry -Junction (games of other sessions, which make a run's
    timings not quiet), and any Arma 3 process. summarize_timing.py joins the
    rows to the shots by time. The rows are stamped in whole seconds.

    Foreign load (PERF-PLAN 2.5 quiet check; the probe of M2's foreign.ps1),
    appended as three more columns:
      foreign_gpu  every process in `nvidia-smi pmon -c 1` with SM use, except
                   PoseidonGame and the Claude app, as name=SM% (all of them,
                   whatever the load, so a run can be audited later);
      owner_game   the owner's game: `deadlock` and `project8` (the memory note's
                   old guess at its name), as name:cpu=<s of CPU since the last
                   sample>:sm=<SM%>; empty when neither runs;
      reasons      why this sample is not quiet (empty when quiet): the owner's
                   game at 3% SM or more or 0.3 s of CPU per sample; any other
                   foreign process at -SmThreshold % SM or more; vmmem (WSL)
                   above 0.6 s of CPU per sample; a build tool (ninja, cmake,
                   clang-cl, cl, lld-link, link, cargo, rustc, ffmpeg,
                   glslangValidator); a foreign PoseidonGame.
      tolerated    GPU users at -SmThreshold % SM or more but below their
                   per-process override (-SmOverrides, default Discord=40:
                   Discord-only windows measured within the clean spread,
                   STEWARD-1 2026-10-31, perf-squad RESUME.md 6). Logged, never
                   a reason.
    gate\bench_report.py drops every timing window with a non-empty `reasons`
    sample inside it (3 s before, 1 s after), as M2's analyze.py did.
#>
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$Seconds = 5400,
    [int]$Every = 2,
    [string]$Junction = 'RemasterUD-gr-',
    [int]$SmThreshold = 15,
    [string]$SmOverrides = 'Discord=40'
)
$smOv = @{}
foreach ($kv in @($SmOverrides -split '[;,]' | Where-Object { $_ -match '=' })) { $k, $v = $kv -split '=', 2; $smOv[$k.Trim()] = [int]$v }
function Get-SmLimit([string]$Name) {
    $n = $Name -replace '\.exe$', ''
    foreach ($k in $smOv.Keys) { if ($n -like "$k*") { return $smOv[$k] } }
    return $SmThreshold
}
"time,util_gpu,util_mem,clock_gr,clock_mem,power_w,pstate,poseidon_games,other_gpu_apps,foreign_gpu,owner_game,reasons,tolerated" | Out-File -FilePath $Out -Encoding ascii
$prevCpu = @{}
function Get-CpuDelta([string]$Pattern) {
    $n = 0; $d = 0.0
    foreach ($p in @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match $Pattern })) {
        $n++
        try { $t = $p.TotalProcessorTime.TotalSeconds } catch { continue }
        $k = "$Pattern|$($p.Id)"
        if ($script:prevCpu.ContainsKey($k)) { $d += $t - $script:prevCpu[$k] }
        $script:prevCpu[$k] = $t
    }
    return @{ n = $n; delta = $d }
}
$builders = @('ninja', 'cmake', 'clang-cl', 'cl', 'lld-link', 'link', 'cargo', 'rustc', 'ffmpeg', 'glslangValidator')
$end = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $end) {
    $q = (& nvidia-smi --query-gpu=utilization.gpu,utilization.memory,clocks.gr,clocks.mem,power.draw,pstate --format=csv,noheader,nounits) -replace '\s', ''
    $all = @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue)
    $foreign = @($all | Where-Object { $_.CommandLine -notmatch [regex]::Escape($Junction) }).Count
    $games = "$($all.Count)/foreign$foreign"
    $other = (@(Get-Process arma3_x64, arma3 -ErrorAction SilentlyContinue | ForEach-Object { $_.ProcessName }) -join '+')
    $reasons = @(); $tolerated = @()
    $fg = @(); $ownerSm = @{}
    try {
        foreach ($l in @(& nvidia-smi pmon -c 1 2>$null)) {
            if ($l -match '^\s*#' -or -not $l.Trim()) { continue }
            $f = $l.Trim() -split '\s+'
            if ($f.Count -lt 5) { continue }
            $sm = $f[3]; $name = $f[-1]
            if ($sm -notmatch '^\d+$' -or $name -match '^(PoseidonGam|claude)') { continue }
            $fg += "$name=$sm"
            if ($name -match '^(deadlock|project8)') { $ownerSm[$name] = [int]$sm; if ([int]$sm -ge 3) { $reasons += "owner-gpu:$name=$sm" } }
            elseif ([int]$sm -ge (Get-SmLimit $name)) { $reasons += "gpu:$name=$sm" }
            elseif ([int]$sm -ge $SmThreshold) { $tolerated += "gpu:$name=$sm" }
        }
    } catch { }
    $owner = @()
    foreach ($n in 'deadlock', 'project8') {
        $c = Get-CpuDelta "^$n"
        if ($c.n -gt 0) {
            $owner += "${n}:cpu=$([math]::Round($c.delta, 2)):sm=$(if ($ownerSm.ContainsKey($n)) { $ownerSm[$n] } else { '-' })"
            if ($c.delta -gt 0.3) { $reasons += "owner-cpu:${n}=$([math]::Round($c.delta, 2))" }
        }
    }
    $w = Get-CpuDelta '^vmmem'
    if ($w.delta -gt 0.6) { $reasons += "vmmem-cpu:$([math]::Round($w.delta, 2))" }
    $b = @(Get-Process $builders -ErrorAction SilentlyContinue | ForEach-Object { $_.ProcessName } | Select-Object -Unique)
    if ($b.Count) { $reasons += "build:$($b -join '+')" }
    if ($foreign -gt 0) { $reasons += "poseidon-foreign:$foreign" }
    "$((Get-Date).ToString('HH:mm:ss')),$q,$games,$other,$($fg -join ' '),$($owner -join ' '),$($reasons -join ' '),$($tolerated -join ' ')" | Out-File -FilePath $Out -Append -Encoding ascii
    if (Test-Path "$Out.stop") { break }
    Start-Sleep -Seconds $Every
}

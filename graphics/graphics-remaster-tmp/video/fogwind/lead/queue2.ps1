# Lead's probe queue: one game at a time on RemasterUD-fog-lead, after the full-clip lane has finished.
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\lead\runs\queue.txt'
while (-not (Select-String -Path $lane -Pattern 'queue finished' -Quiet)) { Start-Sleep -Seconds 5 }
$kit = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\lead\run_probe_lead.ps1'
$out = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\lead\runs'
$fix = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi'
$d91 = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi'
$jobs = @(
    
    @('fog3_static', 'cost2-fix', $fix, ''),
    @('fog3_static', 'cost2-d91', $d91, '')
)
foreach ($j in $jobs) {
    "$(Get-Date -Format HH:mm:ss) start $($j[1])" | Out-File -FilePath (Join-Path $out 'queue2.txt') -Append -Encoding utf8
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $kit, '-Probe', $j[0], '-Tag', $j[1], '-OutRoot', $out,
        '-Junction', 'RemasterUD-fog-lead', '-GameDir', $j[2])
    if ($j[3]) { $a += @('-Prefix', $j[3]) }
    & powershell @a *> (Join-Path $out "$($j[1]).launch.log")
    "$(Get-Date -Format HH:mm:ss) done $($j[1])" | Out-File -FilePath (Join-Path $out 'queue2.txt') -Append -Encoding utf8
}
"$(Get-Date -Format HH:mm:ss) queue2 finished" | Out-File -FilePath (Join-Path $out 'queue2.txt') -Append -Encoding utf8

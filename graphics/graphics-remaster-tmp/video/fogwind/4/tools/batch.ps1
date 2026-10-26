# Agent 4: run a list of probes one after another. Each arg is "Test:Tag:OverrideVariant" (variant '' = none).
param([string]$Jobs)
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\4\tools\run_probe4.ps1'
$sh = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\4\shaders'
foreach ($j in ($Jobs -split "[;,]")) {
    $p = $j.Split(':')
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $t, '-Test', $p[0], '-Tag', $p[1])
    if ($p.Count -gt 2 -and $p[2]) { $a += @('-ShaderOverrideDir', (Join-Path $sh $p[2])) }
    & powershell @a
    "batch: $j finished $(Get-Date -Format HH:mm:ss)"
}
& powershell -NoProfile -File 'D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1' -DataDir 'D:\Arma_CWA_worktrees\video-work\arma_CWR\packages\RemasterUD-fog-4'
"batch: all done $(Get-Date -Format HH:mm:ss)"

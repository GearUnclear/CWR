# Agent 5 (copied from agent 6): run probe variants one after another (one game at a time).
param([string[]]$Tags)
$Tags = @($Tags | ForEach-Object { $_ -split '[,\s]+' } | Where-Object { $_ })
$here = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\5'
foreach ($t in $Tags) {
    "$(Get-Date -Format HH:mm:ss) start $t" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'tools\run_probe5.ps1') -Tag $t
    $band = Get-Content -LiteralPath (Join-Path $here "runs\$t\band.txt") -ErrorAction SilentlyContinue | Select-Object -Last 1
    "$(Get-Date -Format HH:mm:ss) done $t $band" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8
}
"$(Get-Date -Format HH:mm:ss) batch finished" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8

# wait until the previous run (static-percellbuild) is done, then run the list given as args
$p = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\runs\static-percellbuild\status.txt'
while (-not ((Test-Path $p) -and ((Get-Content $p -Raw) -match 'done '))) { Start-Sleep -Seconds 5 }
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\tools\batch2.ps1'
& powershell -NoProfile -ExecutionPolicy Bypass -File $t @args

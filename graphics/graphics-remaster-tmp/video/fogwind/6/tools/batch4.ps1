# agent 6 batch 4: waits for batch 3 (moving-S2z done), then S2h static + moving.
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\tools\run_probe.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\shaders'
$st = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\runs\moving-S2z\status.txt'
while (-not ((Test-Path -LiteralPath $st) -and (Select-String -LiteralPath $st -Pattern '^done' -Quiet))) { Start-Sleep -Seconds 5 }
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test static -Tag static-S2h -ShaderOverrideDir "$s\S2h"
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test moving -Tag moving-S2h -ShaderOverrideDir "$s\S2h"

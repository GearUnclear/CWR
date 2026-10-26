# agent 6 batch 3: waits for batch 2 (moving-C0 done), then S2z static + moving.
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\tools\run_probe.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\shaders'
$st = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\runs\moving-C0\status.txt'
while (-not ((Test-Path -LiteralPath $st) -and (Select-String -LiteralPath $st -Pattern '^done' -Quiet))) { Start-Sleep -Seconds 5 }
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test static -Tag static-S2z -ShaderOverrideDir "$s\S2z"
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test moving -Tag moving-S2z -ShaderOverrideDir "$s\S2z"

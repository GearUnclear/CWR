$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\tools\run_probe_fog3.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\shaders'
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test static -Tag static-t4w -ShaderOverrideDir "$s\t4w"
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test static -Tag static-t4alt -ShaderOverrideDir "$s\t4alt"

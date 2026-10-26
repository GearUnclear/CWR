# agent 6 batch 2: cloud_shadows probe with S2, then C0 on the moving probe.
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\tools\run_probe.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\shaders'
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test x -Tag cloud-S2 -Probe mistlead_cloud_shadows -Label cloud_shadows -From 30 -ShaderOverrideDir "$s\S2"
& powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test moving -Tag moving-C0 -ShaderOverrideDir "$s\C0"

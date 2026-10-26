# agent 6: run probes one after another. Args: test:variant ...  (variant 'none' = the build as is)
$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\tools\run_probe.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\6\shaders'
foreach ($v in $args) {
  $parts = $v.Split(':'); $test = $parts[0]; $var = $parts[1]
  if ($var -eq 'none') { & powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test $test -Tag "$test-build" }
  else { & powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test $test -Tag "$test-$var" -ShaderOverrideDir "$s\$var" }
}

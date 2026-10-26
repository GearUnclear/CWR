$t = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\tools\run_probe_fog3.ps1'
$s = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\3\shaders'
foreach ($v in $args) {
  $parts = $v.Split(':'); $test = $parts[0]; $var = $parts[1]
  if ($var -eq 'none') { & powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test $test -Tag "$test-percellbuild" }
  else { & powershell -NoProfile -ExecutionPolicy Bypass -File $t -Test $test -Tag "$test-$var" -ShaderOverrideDir "$s\$var" }
}

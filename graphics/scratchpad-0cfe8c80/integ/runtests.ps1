param([string]$Label, [string]$Extra = "")
$exe = 'D:\Arma_CWA_worktrees\lighting\arma_CWR\build\lighting-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe'
$out = "D:\Arma_CWA_worktrees\lighting\tmp\bugfix\integ_tests_$Label.txt"
Set-Location D:\
$tags = @('[remaster]','[gpu]','[GL33]','[DebugGroup]','[ShutdownOrder]','[UBORestoreGuard]','[StateOwnership]','[StateCacheAudit]','[BufferUsageAudit]','[IboBindAudit]','[Conventions]','[Shaders]')
if ($Extra) { $tags += $Extra.Split(';') }
"" | Set-Content $out
foreach ($t in $tags) {
  $r = & $exe $t 2>&1 | Select-Object -Last 3
  $line = ($r | Where-Object { $_ -match 'test cases|All tests passed|No test cases' }) -join ' '
  "$t : $line" | Tee-Object -FilePath $out -Append
}
$g = & $exe '[graphics]' 2>&1
$gl = ($g | Select-Object -Last 3 | Where-Object { $_ -match 'test cases|All tests passed' }) -join ' '
"[graphics] : $gl" | Tee-Object -FilePath $out -Append
($g | Select-String -Pattern 'FAILED|failed' -Context 0,0 | Select-Object -First 10) | ForEach-Object { "   " + $_.Line } | Tee-Object -FilePath $out -Append

param([string]$Tests, [string]$OutDir, [string]$Override = '', [string]$GameDir = 'dist/look-fix-plate-rwdi')
$engine = 'D:\Arma_CWA_worktrees\look-fix-plate\arma_CWR'
$done = Join-Path $OutDir 'DONE.txt'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Remove-Item $done -ErrorAction SilentlyContinue
$args2 = @('-NoProfile','-File', "$engine\tests\rendering\captures\run-captures.ps1", '-GameDir', $GameDir, '-DataJunction', 'RemasterUD-fix-plate', '-Tests', $Tests, '-OutDir', $OutDir)
if ($Override) { $args2 += @('-ShaderOverrideDir', $Override) }
& powershell @args2 *> (Join-Path $OutDir 'run.log')
"exit=$LASTEXITCODE $(Get-Date -Format s)" | Out-File -FilePath $done -Encoding ascii

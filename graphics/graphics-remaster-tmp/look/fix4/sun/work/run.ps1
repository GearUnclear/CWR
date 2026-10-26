param([string]$Tests, [string]$Out, [string]$Game = 'dist/look-fix-veil-rwdi', [string]$Ovr = '')
Set-Location D:/Arma_CWA_worktrees/look-fix-veil/arma_CWR
$done = "$Out/DONE.txt"
Remove-Item $done -ErrorAction SilentlyContinue
$a = @('-NoProfile','-File','tests/rendering/captures/run-captures.ps1','-GameDir',$Game,'-DataJunction','RemasterUD-r4-sun','-Tests',$Tests,'-OutDir',$Out)
if ($Ovr) { $a += @('-ShaderOverrideDir',$Ovr) }
& powershell @a *> "$Out/run.log"
"exit=$LASTEXITCODE" | Out-File -Encoding ascii $done

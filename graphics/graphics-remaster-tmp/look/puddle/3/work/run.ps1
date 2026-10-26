param([string]$Name, [string]$Override = '')
$out = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/puddle/3/$Name"
New-Item -ItemType Directory -Force $out | Out-Null
Remove-Item -Force -ErrorAction SilentlyContinue "$out.done"
Set-Location D:/Arma_CWA_worktrees/look-puddle-3/arma_CWR
$args2 = @('-NoProfile','-File','tests/rendering/captures/run-captures.ps1','-GameDir','dist/look-puddle-3-rwdi','-DataJunction','RemasterUD-puddle-3','-Tests','tests/rendering/captures/look_shine_abel.test.sqf,tests/rendering/captures/look_shine_eden.test.sqf','-OutDir',$out)
if ($Override) { $args2 += @('-ShaderOverrideDir', $Override) }
& powershell @args2 *> "$out.log"
"exit $LASTEXITCODE" | Out-File -Encoding ascii "$out.done"

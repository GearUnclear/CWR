param([string]$Tag, [string]$Name, [string]$Worlds = 'abel,eden,sinai', [string]$Override = '')
$root = 'D:\Arma_CWA_worktrees\look-light\arma_CWR'
$out = "C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\ll\captures\$Name"
$tests = ($Worlds -split ',' | ForEach-Object { "tests/rendering/captures/look_${Tag}_$_.test.sqf" }) -join ','
$args = @('-NoProfile', '-File', "$root\tests\rendering\captures\run-captures.ps1", '-Tests', $tests, '-GameDir', 'dist/look-light-rwdi',
          '-DataJunction', 'RemasterUD-look-light', '-OutDir', $out)
if ($Override) { $args += @('-ShaderOverrideDir', $Override) }
& powershell @args *>&1 | Out-File -Encoding utf8 "$out.log"
& python "D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\light\collect.py" $out *>&1 | Out-File -Append -Encoding utf8 "$out.log"
"done" | Out-File -Append -Encoding utf8 "$out.log"

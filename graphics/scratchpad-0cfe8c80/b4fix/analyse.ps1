param([string]$Run = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B4\review-fix\r2-combat')
$s = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad'
$e = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B4\review-fix'
$v7 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B4\runs\v7-combat'
$before = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B4\before-combat'
$bd = 'D:\Arma_CWA_worktrees\gr-B4\arma_CWR\tests\rendering\captures\banddiff.py'

# The knob shots are not copied by make_pairs: copy them next to the others.
Get-ChildItem -Recurse "$Run\tri" -Filter '*__smoke0.png' | ForEach-Object { Copy-Item $_.FullName (Join-Path $Run ($_.Name -replace '^\d+_', '')) -Force }
Get-ChildItem -Recurse "$Run\tri" -Filter '*__scatter0.png' | ForEach-Object { Copy-Item $_.FullName (Join-Path $Run ($_.Name -replace '^\d+_', '')) -Force }

'== plume, sky, base, camp-fire pool (plume.py)' | Set-Content "$e\plume-r2.txt"
$views = 'combat_fires', 'combat_eye_k00', 'combat_eye_k08', 'combat_eye_k11', 'combat_shell_k11', 'combat_shell_k16'
$imgs = foreach ($v in $views) { foreach ($m in 'classic', 'off', 'on', 'smoke0', 'scatter0') { $p = Join-Path $Run "${v}__$m.png"; if (Test-Path $p) { $p } } }
python "$s\b4fix\plume.py" @imgs | Add-Content "$e\plume-r2.txt"
'== v7 (the first build) for comparison' | Add-Content "$e\plume-r2.txt"
$imgs7 = foreach ($v in $views) { foreach ($m in 'classic', 'off', 'on') { $p = Join-Path $v7 "${v}__$m.png"; if (Test-Path $p) { $p } } }
python "$s\b4fix\plume.py" @imgs7 | Add-Content "$e\plume-r2.txt"

python "$s\b4stats.py" $Run | Set-Content "$e\lit-r2.txt"
python "$s\b4r\lv.py" $Run | Set-Content "$e\lv-r2.txt"
python "$s\b4review\rb.py" $Run | Set-Content "$e\rb-r2.txt"
python "$s\b4r\tim.py" $Run | Set-Content "$e\timing-r2.txt"

python $bd $Run $v7 --label-a off --label-b off --md "$e\gate5-r2off-vs-v7off.md" | Out-Null
python $bd $Run $before --label-a off --label-b on --md "$e\gate5-r2off-vs-before-on.md" | Out-Null
python $bd --pairs $Run --label-a off --label-b on --md "$e\gate6-r2-off-vs-on.md" | Out-Null
python "$s\b4r\trip.py" $Run "$Run\strips" | Out-Null
'done'

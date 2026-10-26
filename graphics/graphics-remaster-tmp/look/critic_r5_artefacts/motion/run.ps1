$ErrorActionPreference = 'Continue'
$eng = 'D:\Arma_CWA_worktrees\look-director\arma_CWR'
$out = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\critic_r5_artefacts\motion'
Set-Location $eng
& powershell -NoProfile -File (Join-Path $eng 'tests\rendering\captures\run-captures.ps1') -GameDir dist/look-director-rwdi -DataJunction RemasterUD-look-director-timing -Tests '../../graphics-remaster/tmp/look/critic_r5_artefacts/tests/crit5m_motion.test.sqf' -OutDir $out 2>&1 | Out-File -Encoding utf8 (Join-Path $out 'console.txt')
New-Item -ItemType File -Force (Join-Path $out 'DONE') | Out-Null

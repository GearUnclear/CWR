$ErrorActionPreference = 'Continue'
$eng = 'D:\Arma_CWA_worktrees\look-director\arma_CWR'
$out = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\critic_r4_artefacts\timing'
Set-Location $eng
$parts = @(
  @{ n='abel_a'; t='../../graphics-remaster/tmp/look/critic_r4_artefacts/tests/crit4t_abel_a.test.sqf' },
  @{ n='abel_b'; t='../../graphics-remaster/tmp/look/critic_r4_artefacts/tests/crit4t_abel_b.test.sqf' },
  @{ n='abel_c'; t='../../graphics-remaster/tmp/look/critic_r4_artefacts/tests/crit4t_abel_c.test.sqf' },
  @{ n='eden_sinai'; t='../../graphics-remaster/tmp/look/critic_r4_artefacts/tests/crit4t_eden.test.sqf,../../graphics-remaster/tmp/look/critic_r4_artefacts/tests/crit4t_sinai.test.sqf' }
)
foreach ($p in $parts) {
  $log = Join-Path $out ($p.n + '.console.txt')
  $w = 0
  while ($w -lt 240) {
    $g = @(Get-Process PoseidonGame -ErrorAction SilentlyContinue)
    if ($g.Count -eq 0) { break }
    Start-Sleep 5; $w += 5
  }
  $g = @(Get-Process PoseidonGame -ErrorAction SilentlyContinue)
  ("waited {0}s for quiet; foreign PoseidonGame at start: {1} ({2})" -f $w, $g.Count, (($g | ForEach-Object { $_.Path }) -join '; ')) | Out-File -Encoding utf8 $log
  & powershell -NoProfile -File (Join-Path $eng 'tests\rendering\captures\run-timing.ps1') -GameDir dist/look-director-rwdi -DataJunction RemasterUD-look-director-timing -OutRoot $out -Stamp $p.n -Tests $p.t 2>&1 | Out-File -Encoding utf8 -Append $log
  New-Item -ItemType File -Force (Join-Path $out ($p.n + '.done')) | Out-Null
}
New-Item -ItemType File -Force (Join-Path $out 'ALL.done') | Out-Null

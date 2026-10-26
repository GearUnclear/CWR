$ErrorActionPreference = 'Continue'
$eng = 'D:\Arma_CWA_worktrees\look-director\arma_CWR'
$out = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\critic_r5_artefacts\timing'
$tri = 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe'
$tdir = '../../graphics-remaster/tmp/look/critic_r4_artefacts/tests'
Set-Location $eng
$parts = @(
  @{ n='abel_a'; t="$tdir/crit4t_abel_a.test.sqf" },
  @{ n='abel_b'; t="$tdir/crit4t_abel_b.test.sqf" },
  @{ n='abel_c'; t="$tdir/crit4t_abel_c.test.sqf" },
  @{ n='eden_sinai'; t="$tdir/crit4t_eden.test.sqf,$tdir/crit4t_sinai.test.sqf" }
)
foreach ($p in $parts) {
  $log = Join-Path $out ($p.n + '.console.txt')
  $w = 0
  while ($w -lt 900) {
    $g = @(Get-Process PoseidonGame -ErrorAction SilentlyContinue)
    $s = (& $tri slots) -join ' '
    if ($g.Count -eq 0 -and $s -match 'slots held: 0/4') { break }
    Start-Sleep 10; $w += 10
  }
  $g = @(Get-Process PoseidonGame -ErrorAction SilentlyContinue)
  ("{0} waited {1}s for quiet; foreign PoseidonGame at start: {2} ({3})" -f (Get-Date -Format o), $w, $g.Count, (($g | ForEach-Object { $_.Path }) -join '; ')) | Out-File -Encoding utf8 $log
  & powershell -NoProfile -File (Join-Path $eng 'tests\rendering\captures\run-timing.ps1') -GameDir dist/look-director-rwdi -DataJunction RemasterUD-look-director-timing -OutRoot $out -Stamp $p.n -Tests $p.t 2>&1 | Out-File -Encoding utf8 -Append $log
  New-Item -ItemType File -Force (Join-Path $out ($p.n + '.done')) | Out-Null
}
New-Item -ItemType File -Force (Join-Path $out 'ALL.done') | Out-Null

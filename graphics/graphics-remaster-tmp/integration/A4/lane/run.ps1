# A4 lane game runs, each with its own data junction (RemasterUD-gr-A4*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-A4.
param([string]$Which, [string]$Tag = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A4\lane'
$rc = 'D:\Arma_CWA_worktrees\gr-A4\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-A4\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/A4/lane/tests/a4lane.test.sqf'
$before = 'D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/dist/graphics-remaster-rwdi-A3'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir $before -DataJunction RemasterUD-gr-A4before -OutDir "$lane\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-A4-rwdi -DataJunction RemasterUD-gr-A4 -OutDir "$lane\$stamp-new" -Tests $laneTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-A4-rwdi -DataJunction RemasterUD-gr-A4smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"

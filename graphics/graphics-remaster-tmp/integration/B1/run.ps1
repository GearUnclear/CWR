# B1 lane game runs, each with its own data junction (RemasterUD-gr-B1*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B1.
#   before  the lane stills on the base build (dist/gr-B1-rwdi-before, 193c735)
#   new     the lane stills on the B1 build (dist/gr-B1-rwdi)
#   sinai   the Sinai slab check on the B1 build
#   smoke   smoke_abel on the B1 build
#   test    any test path (relative to arma_CWR) on the B1 build: -Test <path>
param([string]$Which, [string]$Tag = '', [string]$Test = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B1'
$rc = 'D:\Arma_CWA_worktrees\gr-B1\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B1\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/B1/tests/b1lane.test.sqf'
$sinaiTest = '../../graphics-remaster/tmp/integration/B1/tests/b1sinai.test.sqf'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-B1-rwdi-before -DataJunction RemasterUD-gr-B1before -OutDir "$lane\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B1-rwdi -DataJunction RemasterUD-gr-B1 -OutDir "$lane\$stamp-new" -Tests $laneTest }
    'sinai'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B1-rwdi -DataJunction RemasterUD-gr-B1 -OutDir "$lane\$stamp-sinai" -Tests $sinaiTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B1-rwdi -DataJunction RemasterUD-gr-B1smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B1-rwdi -DataJunction RemasterUD-gr-B1 -OutDir "$lane\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
